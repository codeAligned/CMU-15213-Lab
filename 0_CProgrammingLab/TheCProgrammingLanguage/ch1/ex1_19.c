// reversing each line of input

#include <stdio.h>
#define MAX_LEN 100

int main() {
    int c, i;
    char line[MAX_LEN];

    i = 0;
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            for (int j = 0; j < i / 2; ++j) {
                char temp = line[j];
                line[j] = line[i - 1 - j];
                line[i - 1 - j] = temp;
            }
            line[i] = '\0';
            i = 0;
            printf("%s\n", line);
        } else {
            line[i++] = c;
        }
    }
}