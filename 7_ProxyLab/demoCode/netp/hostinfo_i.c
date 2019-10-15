#include "csapp.h"

int main(int argc, char **argv) {
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    /* Get a list of addrinfo records */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* Connections only */

    printf("Thanks for using hostinfo_interactive!\n");
    printf("Enter 'q' or 'quit' to exit!\n");

    while (1) {
        printf("Enter the host name: ");
        scanf("%s", buf);

        if (!strcmp("q", buf) || !strcmp("quit", buf)) {
            exit(0);
        }

        if ((rc = getaddrinfo(buf, NULL, &hints, &listp)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
            continue;
        }

        /* Walk the list and display each IP address */
        flags = NI_NUMERICHOST;  /* Display address string instead of domain name */
        for (p = listp; p; p = p->ai_next) {
            Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
            printf("%s\n", buf);
        }

        /* Clean up */
        Freeaddrinfo(listp);
    }

    exit(0);
}
