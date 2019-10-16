#include <stdio.h>
#include "csapp.h"

#define DEBUG() printf("%d\n", __LINE__)
/* Begin: header and declaration for url parser */
/* https://stackoverflow.com/a/51906794/9057530 */
typedef struct url_info {
    char *protocol;
    char *host;
    char *port;
    char *path;
} URL_INFO;

URL_INFO *split_url(URL_INFO *info, const char *url);
/* End: header and declaration for url parser */

/*
 * Iterative -> Concurrent -> Cache proxy.
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
 * Before implementing caching, do not need to worry about thread-safety. The
 * reading, parsing and storing of request headers does not involve shared
 * variables.
 * 
 * 2. Concurrent
 * Very similar to the code in lecture slide "23-concprog.pdf" page 29-30. 
 * Need to modify "./nop-server.py" to be "python3 ./nop-server.py" to make
 * driver.sh run correctly.
 * 
 */

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define DEFAULT_PORT 8080
#define MAX_HEADER_NUM 20

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

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

int main(int argc, char **argv) {
    int listenfd, *proxy_clientfd_p;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    char proxy_port[MAXLINE];
    pthread_t tid;

    if (argc == 2) {
        strcpy(proxy_port, argv[1]);
    } else {
        strcpy(proxy_port, "8888");
    }

    listenfd = Open_listenfd(proxy_port);
    printf("Proxy listening on port: %s\n", proxy_port);

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        /* proxy_clientfd: used by proxy to serve client */
        proxy_clientfd_p = malloc(sizeof(int));
        *proxy_clientfd_p = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen,
                    client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        Pthread_create(&tid, NULL, serve, proxy_clientfd_p);
    }
    exit(0);
}

/**
 * serve - parse request from client, send the parsed request to server, and 
 *         send the result back to client.
 */
void *serve(void *proxy_clientfd_p) {
    Pthread_detach(Pthread_self());

    int proxy_clientfd = *((int *)proxy_clientfd_p);
    char parsed_request[MAXLINE];
    char host[MAXLINE], port[MAXLINE], uri[MAXLINE];
    char buf[MAXLINE];
    rio_t client_rio; /* rio used by proxy to communicate with client */
    rio_t server_rio; /* rio used by proxy to communicate with client */
    int proxy_serverfd;

    Rio_readinitb(&client_rio, proxy_clientfd);
    parse_client_request(&client_rio, parsed_request, host, port, uri);
    proxy_serverfd = resend_request(&client_rio, &server_rio,
                                    parsed_request, host, port);

    /* Send response from server back to client */
    if (proxy_serverfd > 0) {
        while (Rio_readnb(&server_rio, buf, MAXLINE) > 0) {
            Rio_writen(client_rio.rio_fd, buf, MAXLINE);
        }
    }
    
    if (proxy_clientfd_p) {
        free(proxy_clientfd_p);
    }

    // Closing file descriptors
    Close(proxy_serverfd);
    Close(proxy_clientfd);
    return NULL;
}

/**
 * resend_request - Try to connect to the server at host:port and send the 
 *                  parsed request in char *parsed_request to it. 
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

/* Begin of code for url_parser */
URL_INFO *split_url(URL_INFO *info, const char *url) {
    if (!info || !url)
        return NULL;
    info->protocol = strtok(strcpy((char *)malloc(strlen(url) + 1), url),
                            "://");
    info->host = strstr(url, "://");
    if (info->host) {
        info->host += 3;
        char *host_port_path = strcpy((char *)calloc(1, strlen(info->host) + 1),
                                      info->host);
        info->host = strtok(host_port_path, ":");
        info->host = strtok(host_port_path, "/");
    } else {
        char *host_port_path = strcpy((char *)calloc(1, strlen(url) + 1), url);
        info->host = strtok(host_port_path, ":");
        info->host = strtok(host_port_path, "/");
    }
    char *URL = strcpy((char *)malloc(strlen(url) + 1), url);
    info->port = strstr(URL + 6, ":");
    char *port_path = 0;
    char *port_path_copy = 0;
    if (info->port && isdigit(*(port_path = (char *)info->port + 1))) {
        port_path_copy = strcpy((char *)malloc(strlen(port_path) + 1),
                                port_path);
        char *r = strtok(port_path, "/");
        if (r)
            info->port = r;
        else
            info->port = port_path;
    } else
        info->port = "80";
    if (port_path_copy)
        info->path = port_path_copy + strlen(info->port ? info->port : "");
    else {
        char *path = strstr(URL + 8, "/");
        info->path = path ? path : "/";
    }
    int r = strcmp(info->protocol, info->host) == 0;
    if (r && strcmp(info->port, "80") == 0)
        info->protocol = "http";
    else if (r)
        info->protocol = "tcp";
    return info;
}
/* End of code for url_parser */