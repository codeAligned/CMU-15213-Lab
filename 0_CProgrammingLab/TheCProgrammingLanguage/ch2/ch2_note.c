#include <limits.h>
#include <stdio.h>
#include <math.h>

void printRanges() {
    printf("\nBits of type char: %d\n\n", CHAR_BIT); /* IV */

    printf("Maximum numeric value of type char: %d\n", CHAR_MAX);   /* IV */
    printf("Minimum numeric value of type char: %d\n\n", CHAR_MIN); /* IV */

    printf("Maximum value of type signed char: %d\n", SCHAR_MAX);   /* IV */
    printf("Minimum value of type signed char: %d\n\n", SCHAR_MIN); /* IV */

    printf("Maximum value of type unsigned char: %u\n\n", (unsigned)UCHAR_MAX); /* SF */ /* IV */

    printf("Maximum value of type short: %d\n", SHRT_MAX);   /* IV */
    printf("Minimum value of type short: %d\n\n", SHRT_MIN); /* IV */

    printf("Maximum value of type unsigned short: %u\n\n", (unsigned)USHRT_MAX); /* SF */ /* IV */

    printf("Maximum value of type int: %d\n", INT_MAX);   /* IV */
    printf("Minimum value of type int: %d\n\n", INT_MIN); /* IV */

    printf("Maximum value of type unsigned int: %u\n\n", UINT_MAX); /* RB */ /* IV */

    printf("Maximum value of type long: %ld\n", LONG_MAX); /* RB */   /* IV */
    printf("Minimum value of type long: %ld\n\n", LONG_MIN); /* RB */ /* IV */

    printf("Maximum value of type unsigned long: %lu\n\n", ULONG_MAX); /* RB */ /* IV */
}

void constants() {
    // Constants: int constants, long constants, double constants ...
    123L;   // long
    123l;   // long
    123u;   // unsigned int
    123U;   // unsigned int
    123UL;  // unsigned long
    123ul;  // unsigned long
    1.2f;   // floating point
    1.2F;   // floating point
    // A character constant is an integer. Character constants participate
    // in numeric operations just like an int.
    // A constant expression is an expression involving only constants and
    // can be evaluated during compilation. It can be used where a constant
    // can occur.
}

int strLength(char arr[]) {
    int l = 0;
    while (arr[l] != '\0') {
        ++l;
    }
    return l;
}

void enumeration() {
    // Enumeration constant.
    // The first name in an enum has value 0, the second 1, and so on, unless
    // explicitly specified. if not all values are specified, unspecified values
    // continue the progression from the last specified value.
    // Names in different enumerations must be distinct. Values need not be
    // distince in the same enumeration.
    enum months { JAN = 1,
                  FEB,
                  MAR,
                  APR,
                  MAY,
                  JUN,
                  JUL,
                  AUG,
                  SEP,
                  OCT,
                  NOV,
                  DEC };

    // "enum months" becomes a type
    enum months mon = JAN;
    printf("%d\n", mon);

    // enum can be considered as an alternative to #define
}

void declarations() {
    // A declaration specifies the type and identifier.
    // A declaration can initialize the variable explicitly.
    int i2 = 1;

    // For non-automatic variables(variables defined outside of functions),
    // the initialization is done once only, before the program starts
    // execution and the initializer must be a constant expression.
    // External and static variables are initialized to zero by default.
    // Automatic variables without explicit initialization has undefined value.
}


void expectDoubleWithDeclaration(double);
void expectDoubleWithDeclaration(double d) {
    return;
}

void expectDoubleWithoutDeclaration(double d) {
    return;
}

void typeConversion() {
    // When an operator has operands of different type, they are converted
    // to a common one. Only conversion from a narrower operand to wider
    // operand can happen implicitly.
    // A char is just a small int.

    // Conversion becomes complicated when involving unsigned type.
    // The signed type is promoted to unsigned type.
    printf("%d\n", -1L > 1U);
    // 1U => 1L by adding 0's to the left of original bits
    // 1L => 1UL and interpreted to be a very large positive number.

    // Conversion also happens at assignment: the value of the right side
    // is converted to the type of the left, which is the type of the result.

    // Longer integers are converted to shorter ones by dropping higher-order
    // bits.
    // From printRanges(), we know char is signed on this machine.
    // value of c unchanged, value of j could be changed
    int i = 512, j = 512, k = 511;  
    // 512 => 0x 0...0100000000, 8 trailing zeros
    // 511 => 0x 0...0011111111, 8 trailing ones
    char c = 100;
    printf("\n%d\n", c);
    i = c; 
    c = i;
    printf("%d\n", c);

    printf("%d\n", j);
    c = j;
    j = c;
    printf("%d\n", j);

    printf("%d\n", k);
    c = k;
    k = c;
    printf("%d\n", k);

    // Type conversion also happens when arguments are passed to functions.
    
    // Explicit type conversion can be forced with a unary operator, like:
    //      (type-name) expression
    // The meaning of the casting operation is as if the expression were
    // assigned to a variable of the specified type, which is then used in
    // place of the whole construction. Example:
    // int n = 10;
    // sqrt((double) n);
    // double temp = n;
    // sqrt(temp);

    // This is stated in the book, but does not seem to be the case. It might
    // be because of the compiler version. You may not even need this.
    // Recall that function prototype is the declartion of the function,
    // specifying the name, return type, and parameter types of the function.
    // In the absense of a function type, char and short argument becomes int,
    // float argument becomes double. 
    // However, if function prototype is present, the declaration causes
    // automatic conversion if needed. For example:
    // double sqrt(double i);
    // Then we can directly code: 
    // double root2 = sqrt(2); 
    // without casting.

    expectDoubleWithDeclaration(2);
    expectDoubleWithoutDeclaration(2);
    sqrt(2);
}

void bitwiseOperator() {
    // Shift operators << and >> must take positive offset.
    // Right shring an unsigned type always fill the vacancies with zero.
    // Whether shifting a signed type is logical shift or arithemetic shift 
    // depends on the machine.
}

void assignmentOperator() {
    // += is called assignment operator
    // x *= y + 1  is equivalent x = x * (y+1) 

    // In all assignment statement, the type of the assignment expression
    // is the type of its left operand, and the value is its value
    // after assignment.
}

int main(void) {
    typeConversion();
    return 0;
}