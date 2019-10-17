/** 
 * proxy.c 
 * 
 * Iterative -> Concurrent -> Cache proxy
 * 
 * 1. Iterative
 *  [X] Echo server
 *  [X] Parse request line:
 *      Assumptions made:
 *      a. The request is made by a browser and the URL starts with "http://".
 *      b. Only HTTP/1.0 and HTTP/1.1 are allowed.
 *  [X] Handle request headers:
 *  [X] Send request to server and read response;
 *      List of webhost using HTTP instead of HTTPS:
 *      http://www.example.com/
 *      http://www.ox.ac.uk/
 *      http://www.ucla.edu/
 *      http://www.mit.edu/
 *      http://www.apache.org/
 *      http://www.nyu.edu/
 *      http://go.com/
 *      http://www.washington.edu/
 *  [X] Send response back ts client;
 * 
 * 2. Concurrent
 * Very similar to the code in lecture slide "23-concprog.pdf" page 29-30. 
 * Need to modify "./nop-server.py" to be "python3 ./nop-server.py" to make
 * driver.sh run correctly.
 * 
 * 3. Cache proxy
 * The writeup specifies that the size of the entire cache is at most 1049000
 * bytes and the max cachable web object is 102400 bytes. This would a fully 
 * assiciative cache: only one set and any web object can be matched to any
 * cache line. 1049000 / 102400 = 10, so there would be 10 cache lines.
 * 
 * If the web object is smaller than the maximum size, put into cache, with
 * the hash value of the parsed_request as key. Use timestamp to implement the
 * LRU eviction policy.
 * 
 * Because Pthread_create only allows passing one argument of type (void *), but
 * we need to pass cache and proxy_clientfd_p, we define a struct to combine
 * them together.
 */

#include <stdio.h>
#include "csapp.h"
#include "url_parser.h"

#define USE_CACHE

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CACHE_LINE_NUM (MAX_CACHE_SIZE / MAX_OBJECT_SIZE)
#define DEFAULT_PORT_STR "8888"

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

unsigned long curr_time = 0; /* Used to update timestamp */
int read_cnt = 0;            /* read_cnt: number of readers reading */
sem_t mutex, w;              /* mutex: lock read_cnt */
                             /* w:     lock cache while writing */

/* Defined structs */
typedef struct cache_line {
    char content[MAX_OBJECT_SIZE]; /* Stored web object */
    int length;                   /* Actual length of web object */
    unsigned long long timestamp;  /* To perform LRU */
    unsigned long hash;            /* Hash value of the request header */
    int valid_bit;                 /* Valid bit */
} cache_line_t;

typedef struct combined {
    cache_line_t *cache;
    int *proxy_clientfd_p;
} combined_t;

/* Function prototypes. */
void *serve(void *proxy_clientfd_p);
void parse_client_request(rio_t *client_rp, char *parsed_request,
                          char *host, char *port, char *uri);
void parse_hdr(rio_t *rp, char *parsed_request, char *host);
void parse_request_line(rio_t *client_rp, char *parsed_request,
                        char *host, char *port, char *uri);
int resend_request(rio_t *client_rp, rio_t *server_rp, char *parsed_request,
                   char *host, char *port);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);
void cache_init(cache_line_t **cache_p);
unsigned long hash_func(char *str);
int check_cache(cache_line_t *cache, unsigned long target);
void semaphore_init();
int get_write_idx(cache_line_t *cache);

void cache_init(cache_line_t **cache_p) {
    int i;
    cache_line_t *p;

    *cache_p = Calloc(CACHE_LINE_NUM, sizeof(cache_line_t));
    for (p = *cache_p, i = 0; i < CACHE_LINE_NUM; ++i, ++p) {
        p->length = 0;
        p->timestamp = 0;
        p->hash = 0;
        p->valid_bit = 0;
    }
}

void semaphore_init() {
    sem_init(&mutex, 0, 1);
    sem_init(&w, 0, 1);
}

int main(int argc, char **argv) {
    int listenfd, *proxy_clientfd_p;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    char proxy_port[MAXLINE];

    /* Variables related to threading */
    pthread_t tid;
    cache_line_t *cache = NULL;
    combined_t *args;

    /* Determine proxy port */
    strcpy(proxy_port, (argc == 2) ? argv[1] : DEFAULT_PORT_STR);
    listenfd = Open_listenfd(proxy_port);
    printf("Proxy listening on port: %s\n", proxy_port);

    /* Init proxy cache */
    cache_init(&cache);

    /* Init semaphores */
    semaphore_init();

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        /* proxy_clientfd: used by proxy to serve client */
        proxy_clientfd_p = malloc(sizeof(int));
        *proxy_clientfd_p = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen,
                    client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        /* Update time */
        ++curr_time;

        args = Malloc(sizeof(combined_t));
        args->cache = cache;
        args->proxy_clientfd_p = proxy_clientfd_p;
        Pthread_create(&tid, NULL, serve, (void *)args);
    }

    /* Free proxy cache */
    free(cache);
    exit(0);
}

/**
 * serve - parse request from client, send the parsed request to server, and 
 *         send the result back to client.
 */
void *serve(void *vargs) {
    Pthread_detach(Pthread_self());
    combined_t args = *((combined_t *)vargs);
    free((combined_t *)vargs);
    int proxy_clientfd = *((int *)args.proxy_clientfd_p);
    cache_line_t *cache = (cache_line_t *)args.cache;
    unsigned long request_hash;
    int matched_cache_line_idx;

    char parsed_request[MAXLINE];
    char host[MAXLINE], port[MAXLINE], uri[MAXLINE];
    char buf[MAXLINE];
    rio_t client_rio; /* rio used by proxy to communicate with client */
    rio_t server_rio; /* rio used by proxy to communicate with client */
    int proxy_serverfd;

    Rio_readinitb(&client_rio, proxy_clientfd);
    parse_client_request(&client_rio, parsed_request, host, port, uri);
    request_hash = hash_func(parsed_request);
    printf("\n## Parsed request [hash = %lu] ##\n%s",
           request_hash, parsed_request);

#ifdef USE_CACHE
    if ((matched_cache_line_idx = check_cache(cache, request_hash)) >= 0) {
        printf("Cache hit!\n");
        /* Cache hit: send cached object to client */
        P(&mutex);
        ++read_cnt;
        if (read_cnt == 1) {
            P(&w);
        }
        V(&mutex);

        printf("%d\n", (cache + matched_cache_line_idx)->length);
        printf("%s\n", (cache + matched_cache_line_idx)->content);
        Rio_writen(client_rio.rio_fd,
                   (cache + matched_cache_line_idx)->content,
                   (cache + matched_cache_line_idx)->length);

        P(&mutex);
        --read_cnt;
        if (read_cnt == 0) {
            V(&w);
        }
        V(&mutex);
    } else {
        printf("Cache miss!\n");
        /* Cache miss: send request to server, get response, send to client */
        proxy_serverfd = resend_request(&client_rio, &server_rio,
                                        parsed_request, host, port);

        /* Send response from server to client and put to cache */
        char cache_buf[MAX_OBJECT_SIZE];
        int total = 0;
        if (proxy_serverfd > 0) {
            ssize_t read_num;
            while ((read_num = Rio_readnb(&server_rio, buf, MAXLINE)) > 0) {
                /* Send response back to client */
                Rio_writen(client_rio.rio_fd, buf, read_num);
                printf("--- Content of buf: Begin ----\n%s\n", buf);
                printf("--- Content of buf: End ----\n");

                /* Write to cache_buf, while not full */
                if ((total + read_num) < MAX_OBJECT_SIZE) {
                    memcpy(cache_buf + total, buf, read_num);
                    total += read_num;
                }
            }
            cache_buf[MAX_OBJECT_SIZE] = '\0';
        }

        if (total < MAX_OBJECT_SIZE) {
            /* Write to cache */
            printf("Writing to cache!\n");
            int write_idx = get_write_idx(cache);
            P(&w);
            memcpy((cache + write_idx)->content, cache_buf, total);
            (cache + write_idx)->length = total;
            (cache + write_idx)->timestamp = curr_time;
            (cache + write_idx)->hash = request_hash;
            (cache + write_idx)->valid_bit = 1;
            V(&w);
        }
    }
#else
    proxy_serverfd = resend_request(&client_rio, &server_rio,
                                    parsed_request, host, port);

    /* Send response from server to client */
    if (proxy_serverfd > 0) {
        while (Rio_readnb(&server_rio, buf, MAXLINE) > 0) {
            Rio_writen(client_rio.rio_fd, buf, MAXLINE);
        }
    }
#endif

    // Closing file descriptors
    Close(proxy_serverfd);
    Close(proxy_clientfd);

    return NULL;
}

int get_write_idx(cache_line_t *cache) {
    int i;
    cache_line_t *p;
    unsigned long lru_time = (unsigned long)-1;
    int lru_idx;

    for (p = cache, i = 0; i < CACHE_LINE_NUM; ++i, ++p) {
        if (p->valid_bit == 0) {
            return i;
        } else {
            if (p->timestamp < lru_time) {
                lru_time = p->timestamp;
                lru_idx = i;
            }
        }
    } 

    return lru_idx;
}

/**
 * check_cache - Search cache by checking valid bit and hash value. Return the
 *               index of cache line if there is a match. Otherwise return -1.
 *               If there is a cache hit, the timestamp in the corresponding 
 *               cache line would be updated.
 */
int check_cache(cache_line_t *cache, unsigned long target) {
    int i;
    cache_line_t *p;

    for (p = cache, i = 0; i < CACHE_LINE_NUM; ++i, ++p) {
        if (p->valid_bit == 1 && p->hash == target) {
            P(&w);
            p->timestamp = curr_time;
            V(&w);
            return i;
        }
    }

    return -1;
}

/**
 * resend_request - Try to connect to the server at host:port and send the 
 *                  parsed request in char *parsed_request to it. 
 *                  This function would also init the server_rio, passed in
 *                  through a pointer, if succeed.
 * 
 * Return value: -1, if fail to connect to server.
 *               proxy_serverfd, if succeed.
 */
int resend_request(rio_t *client_rp, rio_t *server_rp, char *parsed_request,
                   char *host, char *port) {
    int proxy_serverfd; /* Used by proxy to connect to server */

    if ((proxy_serverfd = open_clientfd(host, port)) < 0) {
        clienterror(client_rp->rio_fd, "GET", "400", "Bad request",
                    "Fail to connect");
        return -1;
    }

    Rio_readinitb(server_rp, proxy_serverfd);
    Rio_writen(proxy_serverfd, parsed_request, strlen(parsed_request));
    return proxy_serverfd;
}

/**
 * parse_client_request - Parse the request from client, store in string 
 *                        pointed to by char *parsed_request.
 */
void parse_client_request(rio_t *client_rp, char *parsed_request,
                          char *host, char *port, char *uri) {
    parse_request_line(client_rp, parsed_request, host, port, uri);
    parse_hdr(client_rp, parsed_request, host);
}

/**
 * parse_request_line - Parse the request line and store it in 
 *                      char *parsed_request. 
 */
void parse_request_line(rio_t *client_rp, char *parsed_request,
                        char *host, char *port, char *uri) {
    char request_line[MAXLINE];
    char method[MAXLINE], url[MAXLINE], version[MAXLINE];

    if (!Rio_readlineb(client_rp, request_line, MAXLINE))
        return;

    sscanf(request_line, "%s %s %s", method, url, version);

    // Check request method
    if (strcasecmp(method, "GET")) {
        clienterror(client_rp->rio_fd, method, "501", "Not Implemented",
                    "The proxy does not implement this method");
        return;
    }

    URL_INFO info;
    split_url(&info, url);
    strcpy(host, info.host);
    strcpy(port, info.port);
    strcpy(uri, info.path);

    // Check HTTP version
    if (strcmp("HTTP/1.0", version) && strcmp("HTTP/1.1", version)) {
        clienterror(client_rp->rio_fd, method, "400", "Bad request",
                    "Invalid HTTP version");
        return;
    } else {
        strcpy(version, "HTTP/1.0");
    }

    sprintf(parsed_request, "%s %s %s\r\n", method, uri, version);
}

/**
 * parse_hdr - Parse the request header and store it in char *parsed_request. 
 */
void parse_hdr(rio_t *rp, char *parsed_request, char *host) {
    char hdr[MAXLINE];
    int host_hdr_exist = 0, conn_hdr_exist = 0, proxy_conn_hdr_exist = 0;

    while (1) {
        Rio_readlineb(rp, hdr, MAXLINE);
        if (!strcmp(hdr, "\r\n")) {
            break;
        }
        if (strstr(hdr, "Host:")) {
            host_hdr_exist = 1;
            strcat(parsed_request, hdr);
        } else if (strstr(hdr, "Connection:")) {
            conn_hdr_exist = 1;
            strcat(parsed_request, conn_hdr);
        } else if (strstr(hdr, "Proxy connection:")) {
            proxy_conn_hdr_exist = 1;
            strcat(parsed_request, proxy_conn_hdr);
        } else {
            strcat(parsed_request, hdr);
        }
    }

    strcat(parsed_request, user_agent_hdr);
    if (!host_hdr_exist) {
        char host_hdr[MAXLINE];
        sprintf(host_hdr, "Host: %s\r\n", host);
        strcat(parsed_request, host_hdr);
    }
    if (!conn_hdr_exist) {
        strcat(parsed_request, conn_hdr);
    }
    if (!proxy_conn_hdr_exist) {
        strcat(parsed_request, proxy_conn_hdr);
    }
    strcat(parsed_request, "\r\n");
}

/**
 * hash_func - Hash function to be applied to the request. Borrowed from: 
 *             https://stackoverflow.com/a/7666577/9057530
 */
unsigned long hash_func(char *str) {
    unsigned long res = 5381;
    int c;

    while ((c = *(str++)))
        res = ((res << 5) + res) + c; /* hash * 33 + c */

    return res;
}

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body,
            "%s<body bgcolor="
            "ffffff"
            ">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}