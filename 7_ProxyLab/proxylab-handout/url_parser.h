/* https://stackoverflow.com/a/51906794/9057530 */

typedef struct url_info {
    char *protocol;
    char *host;
    char *port;
    char *path;
} URL_INFO;

URL_INFO *split_url(URL_INFO *info, const char *url);