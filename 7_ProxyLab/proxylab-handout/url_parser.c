#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "url_parser.h"

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