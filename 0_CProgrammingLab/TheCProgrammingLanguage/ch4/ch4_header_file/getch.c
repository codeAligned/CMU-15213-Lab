#include <stdio.h>

#define BUFSIZE 100

// If the two functions and two static variables are compiled in one file, then
// no other function can access buf and bufp. That is, buf and bufp is hidden
// within this file, this use of "static" is like an access control feature,
// like private members in Java.


static char buf[BUFSIZE];
static int bufp = 0;

int getch(void) {
    return (bufp > 0) ? buf[--bufp] : getchar();
}

void ungetch(int c) {
    if (bufp >= BUFSIZE) {
        printf("ungetch: too many characters\n");
    } else {
        buf[bufp++] = c;
    }
}
