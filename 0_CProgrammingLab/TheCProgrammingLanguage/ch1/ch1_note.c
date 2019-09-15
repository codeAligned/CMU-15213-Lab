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

// Function declaration
// You can declare more than once
int power(int m, int n);
int power(int, int);

// Function definition
int power(int m, int n) {
    if (n == 0) {
        return 1;
    } else {
        return m * power(m, n - 1);
    }
}

// Pass-by-value
// Function arguments are passed by value. When a function needs to modify
// a variable in the calling routine, the caller must provide the address of
// the variable as argument, and the called function must declare the paramter
// to be a pointer in order to access the variable indirectly.
// Special case: when the name of an array is used as an argument, the value
// passed to the function is the location/address of the beginning of the array.

// Automatic (local) variables
// automatic variables: local variables in functions, which are created when
// the function is called and destroyed when the function terminates.

// Extern variables
// External variables can be defined outside of any function and it must be
// declared in each function that wants to access it. The declaration can be
// by "extern" keyword, or implicitly from the context. 
// If the definition of an external variable occurs in the source
// file before its use in a particular function, then the "extern" declaration
// is not required. It is actually a common practice to place
// the definition of all external variables at the beginning of the source
// file and omit all extern declarations.
// If the program has multiple source files, and a variable is defined in file1
// and used in file2 and file3, then extern declarations are needed in file2
// and file3 to connect the occurrences of the variable. The usual practice is
// to collect extern declarations of variables and functions in a separate file,
// called a header, that is included by #include at the front of each source
// file, and there will be no need for extern declarations in the source files.

// Be careful about using external variables, very bug-prune and reduces
// code-resuability.

int main() {
    printf("%d", power(2, 1));
    return 0;
}
