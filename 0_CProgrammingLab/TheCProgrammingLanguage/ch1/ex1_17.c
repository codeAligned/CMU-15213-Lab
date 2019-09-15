// Print all input lines with more than 80 chars

#include <stdio.h>
#define MIN_LEN 5

int main() {
    int c, len;
    len = 0;
    char leadingChars[MIN_LEN];

    while ((c = getchar()) != EOF) {
        if (c != '\n') {
            if (len < MIN_LEN - 1) {
                // Fill array but do not print
                leadingChars[len++] = c;
            } else if (len == MIN_LEN - 1) {
                // Array becomes full, put null character
                leadingChars[MIN_LEN - 1] = '\0';

                // Print leading chars
                printf("%s", leadingChars);

                // Print current char
                ++len;
                printf("%c", c);
            } else {
                // Print current char
                ++len;
                printf("%c", c);
            }
        } else {
            printf("\n");
            len = 0;
        }
    }
}