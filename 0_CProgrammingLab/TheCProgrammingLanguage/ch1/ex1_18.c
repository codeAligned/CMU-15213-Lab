// Remove trailing blanks, tabls from each line.
// Remove entirely blank lines

#include <stdio.h>

#define MAX_LEN 20

int main() {
    int c, lastNonblankIdx, i;
    char line[MAX_LEN];

    i = 0;
    lastNonblankIdx = -1;
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            line[lastNonblankIdx + 1] = '\0';
            printf("[%s]\n", line);
            i = 0;
            lastNonblankIdx = -1;
        } else {
            line[i] = c;
            if (c != ' ' && c != '\n' && c != '\t') {
                lastNonblankIdx = i;
            }
            ++i;
        }
    }
}