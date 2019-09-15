#include <stdio.h>

// Symbolic constants
#define LOWER 0
#define UPPER 100
#define STEP 20

void printFahrenheitTableInaccurate() {
    int fahr;
    int celsius;
    int low, high, step;

    low = 0, high = 100, step = 20;
    fahr = low;
    while (fahr <= high) {
        celsius = 5 * (fahr - 32) / 9;       // Does not work: 5/9*(fahr-32)
        printf("%3d %3d\n", fahr, celsius);  // right-justified width
        fahr = fahr + step;
    }
}

void printFahrenheitTableAccurate() {
    // If an arithmetic operator has two integer operands, integer operation is
    // performed. If it has one floating-point operand, any integer operand
    // would be converted to be floating point.
    float fahr, celsius;
    int low, high, step;

    low = 0, high = 100, step = 20;
    fahr = low;
    while (fahr <= high) {
        celsius = (5.0 / 9.0) * (fahr - 32.0);
        printf("%3.0f %6.2f\n", fahr, celsius);  // right-justified width
        fahr = fahr + step;
    }
}

void printFahrenheitTableUsingFor() {
    int fahr;
    for (fahr = UPPER; fahr >= LOWER; fahr = fahr - STEP) {
        printf("%3d %6.2f\n", fahr, (5.0 / 9.0) * (fahr - 32.0));
    }
}

void charIO() {
    int c;
    // Using int instead of char. The reason is that, type char is not big
    // enough to hold EOF, so we use int.
    while (c != EOF) {
        // EOF: end of file indicator.
        // It is an integer defined in <stdio.h>. The specific value does not
        // matter as long as it is not the same as any char.
        putchar(c);
        c = getchar();
    }
}

void charIOShort() {
    // In C, an assignment is an expression of the value of the lefthand-side
    // after the assignment.
    int c;
    while ((c = getchar()) != EOF) {
        putchar(c);
    }
}

void charCountWhile() {
    long count;
    count = 0;
    while (getchar() != EOF) {
        ++count;
    }
    printf("%d char(s) entered!\n", count);
}

void charCountFor() {
    long count;
    for (count = 0; getchar() != EOF; ++count) {
        ;
    }
    printf("%d char(s) entered!\n", count);
}

void lineCountWhile() {
    long count = 0;
    int c;

    count = 0;
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            ++count;
        }
    }
    printf("%d line(s) entered!\n", count);
}

void lineCountFor() {
    long count;
    int c;
    for (count = 0; (c = getchar()) != EOF;) {
        if (c == '\n') {
            ++count;
        }
    }
    printf("%d line(s) entered!\n", count);
}

void blankTabNewlineCount() {
    long blankCount, tabCount, newlineCount;
    int c;

    blankCount = 0, tabCount = 0, newlineCount = 0;
    while ((c = getchar()) != EOF) {
        switch (c) {
            case ' ':
                ++blankCount;
                break;
            case '\t':
                ++tabCount;
                break;
            case '\n':
                ++newlineCount;
                break;
            default:
                break;
        }
    }

    printf("Blank: %d, Tab: %d, Newline: %d", blankCount, tabCount, newlineCount);
}

#define OUT 0
#define IN 1

void wordCount() {
    long lineCount, wordCount, charCount;
    int c;
    lineCount = wordCount = charCount = 0;
    // lineCount = (wordCount = (charCount = 0));

    int state = OUT;
    while ((c = getchar()) != EOF) {
        ++charCount;
        if (c == '\n') {
            ++lineCount;
        }

        if (c == '\n' || c == ' ' || c == '\t') {
            state = OUT;
        } else if (state == OUT) {
            state = IN;
            ++wordCount;
        }
    }

    printf("Line: %d, word: %d, char: %d", lineCount, wordCount, charCount);
}

void printWordPerLine() {
    int c, state = OUT;
    while ((c = getchar()) != EOF) {
        if (c == ' ' || c == '\t' || c == '\n') {
            if (state == IN) {
                printf("\n");
            }
            state = OUT;
        } else {
            state = IN;
            printf("%c", c);
        }
    }
}

void countCertainChars() {
    int table[13];  // 0-9, blank, tab, newline
    int i, c;

    for (i = 0; i < 13; ++i) {
        table[i] = 0;
    }

    while ((c = getchar()) != EOF) {
        if (c == ' ') {
            ++table[10];
        } else if (c == '\t') {
            ++table[11];
        } else if (c == '\n') {
            ++table[12];
        } else if ('0' <= c && c <= '9') {
            ++table[c - '0'];
        }
    }

    for (i = 0; i < 13; ++i) {
        printf("%d ", table[i]);
    }
}

#define MAX_LEN 10
void wordLengthHistogramHorizontal() {
    // counting words of at most 50 chars
    int i, j, c, currLen, state, table[MAX_LEN];
    for (i = 0; i < MAX_LEN; ++i) {
        table[i] = 0;
    }

    currLen = 0;
    state = OUT;
    while ((c = getchar()) != EOF) {
        if (c != ' ' && c != '\n' && c != '\t') {
            if (state == OUT) {
                state = IN;
                currLen = 1;
            } else {
                ++currLen;
            }
        } else {
            if (state == IN && currLen <= MAX_LEN) {
                ++table[currLen - 1];
            }
            state = OUT;
        }
    }

    for (i = 0; i < MAX_LEN; ++i) {
        printf("%2d  ", i + 1);
        for (j = 0; j < table[i]; ++j) {
            printf("|");
        }
        printf("\n");
    }
}

//---------------------------------------------
int getLine(char arr[], int size) {
    // If a line only contains one newline character, this function should
    // return length 1.
    int c, i;
    for (i = 0; i < size - 1 && (c = getchar()) != EOF; ++i) {
        arr[i] = c;
        if (c == '\n') {
            ++i;
            break;
        }
    }

    arr[i] = '\0';
    return i;
}

void copyCharArr(char from[], char to[]) {
    int i = 0;
    while ((to[i] = from[i]) != '\0') {
        ++i;
    }
}

void printLongestLine() {
    int size = 100;
    int currLength = 0, maxLength = 0;
    char line[size], longestLine[size];
    while ((currLength = getLine(line, size)) != 0) {
        if (currLength > maxLength) {
            maxLength = currLength;
            copyCharArr(line, longestLine);
        }
    }

    printf("Longest line has %d char(s): %s", maxLength, longestLine);
}
// -----------------------------------------------------




int main() {
    printLongestLine();
    return 0;
}
