#include <stdio.h>

void f1();
void f2();
/*
gcc separateCompile1.c  separateCompile2.c separateCompile3.c
./a.out
*/

int main() {
    f1();
    f2();
}