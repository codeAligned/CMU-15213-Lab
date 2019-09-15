#include <stdio.h>
#include <stdlib.h>  // for atof()
#include <calc.h>

#define MAXOP 100  // max size of operand or operator
#define NUMBER '0'

main() {
    int type;
    double op2;
    char s[MAXOP];

    while ((type = getop(s)) != EOF) {
        switch (type) {
            case NUMBER:
                push(atof(s));
                break;
            case '+':
                push(pop() + pop());
                break;
            case '*':
                push(pop() * pop());
                break;
            case '-':
                op2 = pop();
                push(pop() - op2);
                break;
            case '/':
                op2 = pop();
                if (op2 != 0.0)
                    push(pop() / op2);
                else
                    printf("error: zero divisor\n");
                break;
            case '%':
                op2 = pop();
                if (op2 == 0.0)
                    printf("error: can not mod 0\n");
                else
                    push(fmod(pop(), op2));
                break;
            case '=':
                op2 = pop();
                setVar(pop(), op2);
                break;
            case '\n':
                printf("\t%.8g\n", pop());
                break;
            default:
                printf("error: unknown command %s\n", s);
                break;
        }
    }
    return 0;
}