#include <ctype.h>
#include <stdio.h>

#define NUMBER '0'

int getch(void);
void ungetch(int);

int getop(char s[]) {
    int i, c;

    while ((s[0] = c = getch()) == ' ' || c == '\t')
        ;

    s[1] = '\0';
    if (!isdigit(c) && c != '.' && c != '-')
        return c;

    i = 0;
    if (c == '-') {
        if (isdigit(c = getch()) || c == '.')
            s[++i] = c;
        else {
            if (c != (char)EOF)
                ungetch(c);
            return '-';
        }
    }

    if (isdigit(c))
        while (isdigit(s[++i] = c = getch()))
            ;  // Empty

    if (c == '.')
        while (isdigit(s[++i] = c = getch()))
            ;  // Empty

    s[i] = '\0';

    if (c != (char)EOF)
        ungetch(c);
    return NUMBER;
}