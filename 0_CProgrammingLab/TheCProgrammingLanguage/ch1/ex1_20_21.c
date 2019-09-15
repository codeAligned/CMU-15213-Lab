#include <stdio.h>

#define TAB_WIDTH 8
#define IN_BLANK 1
#define OUT_OF_BLANK 0

void detab() {
    // Detab each line of input. The output should look the same.
    // Detab: replace tab with space
    int c, currPos;

    currPos = 0;
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            printf("\n");
            currPos = 0;
        } else if (c != '\t') {
            printf("%c", c);
            ++currPos;
        } else {
            // c == '\t'
            for (int i = currPos % TAB_WIDTH; i < TAB_WIDTH; ++i) {
                printf("*");
            }
            currPos += (TAB_WIDTH - currPos % TAB_WIDTH);
        }
    }
}

void entab() {
    // Entab each line of input. The output should look the same.
    // Entab: replace blanks with minimum number of tabs and spaces.
    // => You cannot really help with tabs. So try to replace tab
    // with space whenver possible.
    int c, currPos, state, blankBegin;

    currPos = 0;
    state = OUT_OF_BLANK;
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            printf("\n");
            currPos = 0;
        } else if (c == '\t') {
            if (state == OUT_OF_BLANK) {
                state = IN_BLANK;
                blankBegin = currPos;
            }
            currPos = currPos + (TAB_WIDTH - currPos % TAB_WIDTH);
        } else if (c == ' ') {
            if (state == OUT_OF_BLANK) {
                state = IN_BLANK;
                blankBegin = currPos;
            }
            ++currPos;
        } else if (c != ' ') {
            if (state == IN_BLANK) {
                // blank to non-blank transition
                state = OUT_OF_BLANK;

                int i;
                // Off by 1 bug is possible here. 
                // Use a concrete example to help you get it right.
                for (i = blankBegin;
                     i + (TAB_WIDTH - i % TAB_WIDTH) <= currPos;
                     i = i + (TAB_WIDTH - i % TAB_WIDTH)) {
                    printf("\t");
                }

                if (i < currPos) {
                    for (int j = i; j < currPos; ++j) {
                        printf("*");
                    }
                }
            }
            printf("%c", c);
            ++currPos;
        }
    }
}

int main() {
    entab();
}