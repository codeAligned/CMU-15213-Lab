#include <stdio.h>

// If the return type is omitted in the function declaration, int is assumed.

// You can include the declaration of one funtion in another function. The
// calling function needs to know the return type of the called function, so
// you can declare the called function explicitly.

/*
In Linux.
Recompile all:
gcc -c dummy.c;
gcc -c ch4_note_function_prototype.c;
gcc -o ch4_note_function_prototype ch4_note_function_prototype.o dummy.o;
./ch4_note_function_prototype;
*/

int main() {
    // Try comment this line and recompile
    double dummy(void);  // Defined in dummy.c
    // When the above line is commented out, no declaration of function dummy
    // is available. And the function is assumed to return int. Thus, dummy()
    // would return a double which main() takes as an int, and generate
    // meaningless ouput.

    int resultInt = dummy();
    double resultDouble = dummy();
    printf("%d\n", resultInt);
    printf("%f\n", resultDouble);
    // Output: 
    // Uncomment: 
    // 1000
    // 1000.990000
    // Commented out:
    // 0
    // 0.000000
    // 
    // Message from compiler: 
    // ch4_note.c:26:21: warning: implicit declaration of function ‘dummy’ [-Wimplicit-function-declaration]
    //  int resultInt = dummy();
    //                  ^
    // Meaing that compiler detects an implit declaration of function dummy()
    // here, when the undeclared function is first used in an expression. 
    // A mismatch between declaration and definition happens. 
    // Generally, it is good practice to write prototype(function declaration)
    // for all functions in C.
}

// If there is no function prototype, a function is implicitly declared by
// its first appearance in an expression. If a name that has not been
// previously declared occurs in an expression and is followed by a left
// parenthesis, it is declared by context to be a function name, the funtion
// is assumed to return an int, and nothing is assumed about its arguments.
// Furthermore, if a function declaration does not include arguments, it too
// it taken to mean that nothing is to be assumed about the arguments of the
// function. This is to be backward compatible with older C programs, but it
// is bad practice. If the function takes no arguments, use void to indicate.