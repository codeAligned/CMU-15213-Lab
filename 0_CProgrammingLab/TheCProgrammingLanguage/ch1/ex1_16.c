#include <stdio.h>

int getLine(char arr[], int size) {
    // If a line only contains one newline character, this function should
    // return length 1.
    int c, i;
    for (i = 0; (c = getchar()) != EOF; ++i) {
        if (i < size - 1) {
            arr[i] = c;
        }
        // If the input exceed size, do not update the arr

        if (c == '\n') {
            ++i;
            break;
        }
    }

    // The array would contain at most the first (size-1) character of input
    if (i <= size - 1) {
        arr[i] = '\0';
    } else {
        arr[size - 1] = '\0';
    }
    return i;
}

void copyCharArr(char from[], char to[]) {
    int i = 0;
    while ((to[i] = from[i]) != '\0') {
        ++i;
    }
}

void printLongestLine() {
    int size = 10;
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

int main() {
    printLongestLine();
}