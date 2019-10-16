#include <stdio.h>
#include "csapp.h"

#define DEBUG() printf("%d\n", __LINE__)
/* Begin: header and declaration for url_parser */
/* https://github.com/jaysonsantos/url-parser-c */
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct url_parser_url {
    char *protocol;
    char *host;
    int port;
    char *path;
    char *query_string;
    int host_exists;
    char *host_ip;
} url_parser_url_t;
void free_parsed_url(url_parser_url_t *url_parsed);
int parse_url(char *url, bool verify_host, url_parser_url_t *parsed_url);
/* Begin: header and declaration for url_parser */

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
 *      List of website using HTTP instead of HTTPS:
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
void serve(int proxy_connfd);
void parse_client_request(rio_t *client_rp, char *parsed_request,
                          char *host, char *port);
void parse_hdr(rio_t *rp, char *parsed_request, char *host);
void parse_request_line(rio_t *client_rp, char *parsed_request,
                        char *host, char *port);
void resend_request(rio_t *client_rp, char *parsed_request,
                    char *host, char *port);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
    int listenfd, proxy_connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    char proxy_port[MAXLINE];

    if (argc == 2) {
        strcpy(proxy_port, argv[1]);
    } else {
        strcpy(proxy_port, "8080");
    }

    listenfd = Open_listenfd(proxy_port);
    printf("Proxy listening on port: %s\n", proxy_port);

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        /* proxy_connfd: used by proxy as a connected fd to serve client */
        proxy_connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen,
                    client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        serve(proxy_connfd);
        printf("Close connection to (%s, %s)\n", client_hostname, client_port);
        Close(proxy_connfd);
    }
    exit(0);
}

void serve(int proxy_connfd) {
    char parsed_request[MAXLINE], host[MAXLINE], port[MAXLINE];
    rio_t client_rio; /* The rio used by proxy to communicate with client */
    Rio_readinitb(&client_rio, proxy_connfd);

    parse_client_request(&client_rio, parsed_request, host, port);
    printf("%s", parsed_request);
    resend_request(&client_rio, parsed_request, host, port);
}

void resend_request(rio_t *client_rp, char *parsed_request,
                    char *host, char *port) {
    int proxy_clientfd; /* Used by proxy as a client fd to connect to server */
    rio_t rio;
    char buf[MAXLINE];

    if ((proxy_clientfd = open_clientfd(host, port)) < 0) {
        clienterror(client_rp->rio_fd, "GET", "400", "Bad request",
                    "Fail to connect");
    }

    Rio_readinitb(&rio, proxy_clientfd);
    Rio_writen(proxy_clientfd, parsed_request, strlen(parsed_request));
    while (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
        Rio_writen(client_rp->rio_fd, buf, strlen(buf));
    }
    Close(proxy_clientfd);
}

void parse_client_request(rio_t *client_rp, char *parsed_request,
                          char *host, char *port) {
    parse_request_line(client_rp, parsed_request, host, port);
    parse_hdr(client_rp, parsed_request, host);
}

void parse_request_line(rio_t *client_rp, char *parsed_request,
                        char *host, char *port) {
    char request_line[MAXLINE];
    char method[MAXLINE], url[MAXLINE], version[MAXLINE], uri[MAXLINE];

    if (!Rio_readlineb(client_rp, request_line, MAXLINE))
        return;

    sscanf(request_line, "%s %s %s", method, url, version);

    // Check request method
    if (strcasecmp(method, "GET")) {
        clienterror(client_rp->rio_fd, method, "501", "Not Implemented",
                    "The proxy does not implement this method");
        return;
    }

    // Parse hostname and URI from URL
    url_parser_url_t *parsed_url = (url_parser_url_t *)malloc(sizeof(url_parser_url_t));
    if (parse_url(url, false, parsed_url)) {
        clienterror(client_rp->rio_fd, method, "400", "Bad request",
                    "Cannot parse URL");
        return;
    }

    strcpy(host, parsed_url->host);
    strcpy(uri, parsed_url->path);
    sprintf(port, "%d", parsed_url->port);
    free_parsed_url(parsed_url);

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
void free_parsed_url(url_parser_url_t *url_parsed) {
    if (url_parsed->protocol)
        free(url_parsed->protocol);
    if (url_parsed->host)
        free(url_parsed->host);
    if (url_parsed->path)
        free(url_parsed->path);
    if (url_parsed->query_string)
        free(url_parsed->query_string);

    free(url_parsed);
}

/**
 * Defaults values:
 * - port: 80
 * - uri: "/" 
 */
int parse_url(char *url, bool verify_host, url_parser_url_t *parsed_url) {
    char *local_url = (char *)malloc(sizeof(char) * (strlen(url) + 1));
    char *token;
    char *token_host;
    char *host_port;
    char *host_ip;

    char *token_ptr;
    char *host_token_ptr;

    char *path = NULL;

    // Copy our string
    strcpy(local_url, url);

    token = strtok_r(local_url, ":", &token_ptr);
    parsed_url->protocol = (char *)malloc(sizeof(char) * strlen(token) + 1);
    strcpy(parsed_url->protocol, token);

    // Host:Port
    token = strtok_r(NULL, "/", &token_ptr);
    if (token) {
        host_port = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strcpy(host_port, token);
    } else {
        host_port = (char *)malloc(sizeof(char) * 1);
        strcpy(host_port, "");
    }

    token_host = strtok_r(host_port, ":", &host_token_ptr);
    parsed_url->host_ip = NULL;
    if (token_host) {
        parsed_url->host = (char *)malloc(sizeof(char) * strlen(token_host) + 1);
        strcpy(parsed_url->host, token_host);

        if (verify_host) {
            struct hostent *host;
            host = gethostbyname(parsed_url->host);
            if (host != NULL) {
                parsed_url->host_ip = inet_ntoa(*(struct in_addr *)host->h_addr);
                parsed_url->host_exists = 1;
            } else {
                parsed_url->host_exists = 0;
            }
        } else {
            parsed_url->host_exists = -1;
        }
    } else {
        parsed_url->host_exists = -1;
        parsed_url->host = NULL;
    }

    // Port
    token_host = strtok_r(NULL, ":", &host_token_ptr);
    if (token_host)
        parsed_url->port = atoi(token_host);
    else
        parsed_url->port = 80;

    token_host = strtok_r(NULL, ":", &host_token_ptr);
    assert(token_host == NULL);

    token = strtok_r(NULL, "?", &token_ptr);
    parsed_url->path = NULL;
    if (token) {
        path = (char *)realloc(path, sizeof(char) * (strlen(token) + 2));
        strcpy(path, "/");
        strcat(path, token);

        parsed_url->path = (char *)malloc(sizeof(char) * strlen(path) + 1);
        strncpy(parsed_url->path, path, strlen(path));

        free(path);
    } else {
        parsed_url->path = (char *)malloc(sizeof(char) * 2);
        strcpy(parsed_url->path, "/");
    }

    token = strtok_r(NULL, "?", &token_ptr);
    if (token) {
        parsed_url->query_string = (char *)malloc(sizeof(char) * (strlen(token) + 1));
        strncpy(parsed_url->query_string, token, strlen(token));
    } else {
        parsed_url->query_string = NULL;
    }

    token = strtok_r(NULL, "?", &token_ptr);
    assert(token == NULL);

    free(local_url);
    free(host_port);
    return 0;
}
/* End of code for url_parser */