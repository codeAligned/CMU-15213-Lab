#include <stdio.h>
#include "csapp.h"

/**
 * Iterative -> Concurrent -> Cache proxy.
 * 
 * 1. Iterative
 *  [X] Receive and print out request from client(like an echo server);
 *  [] Handle malformed client request;
 *  [] Send request to server;
 *  [] Accept response from server;
 *  [] Send response back to client.
 * /

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define DEFAULT_PORT 8080

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Function prototypes. */
void echo(int connfd, char *client_hostname, char *client_port);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */
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
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen,
                    client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd, client_hostname, client_port);
        Close(connfd);
    }
    exit(0);
}

void echo(int connfd, char *client_hostname, char *client_port) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {  //line:netp:echo:eof
        printf("server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }

    printf("Disconnected from (%s, %s)\n", client_hostname, client_port);
}
