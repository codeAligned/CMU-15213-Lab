// 5.1 Pointers and Addresses
// A pointer is a variable containing the address of a variable.

// The & operator can be applied to objects in memory, but not expressions,
// constants or register variables.

// Syntax for declaring a int pointer: int *ip; is to denote that (*ip) would
// be of integer type.

// The name of an array is a synonym for the location of the first element.
// Any array subscription can be rewritten using a pointer and offset.


// 5.2 Pointers and Function Arguments
// 5.3 Pointers and Arrays
// In function parameter declaration, char [] and char * are equivalent. But
// the latter is preferred since it says explicity that the parameter is
// a pointer (the array name decays to a pointer).
// You can pass a subarray to a function by passing the pointer to its start.
// For example:
// char arr[10];
// f(&arr[2]);
// f(arr+2);

// 5.4 Address Arithmetic
// Simple memory allocation
#define ALLOCSIZE 10000

// Declared as static to be invisible outside the file
static char allocbuf[ALLOCSIZE];
static char *allocp = allocbuf;

char *alloc(int n) {
    if (allocp + n <= allocbuf + ALLOCSIZE) {
        allocp += n;
        return allocp - n;
    } else {
        printf("Not enough space!\n");
        return 0; // C guarantees that 0 is never a valid address for data
        // Normally pointer and integer are not compatible in C, with
        // 0 as an exception. The symbol constant NULL is usually used
        // in place of 0, defined in <stdio.h>.
    }
}

char *afree(char *p) {
    if (allocbuf <= p && p < allocbuf + ALLOCSIZE) {
        allocp = p;
    }
}

// Valid pointer operations:
// 1. Assignment of pointers of the same type(except void*);
// 2. Adding/Substracting a pointer and an integer;
// 3. Subtracting or comparing two pointers to members of the same array;
// 4. Assignment or comparing to zero/NULL.

// 5.5 Character Pointers and Functions
// C does not provide any operators for processing an entire string: all are
// pointer operations.

char message1[] = "This is a message";
char *message2 = "This is a message";
// Important difference: message1 is an array. Individual characters within the
// array can be changed but message1 would always refer to the same storage.
// However, message2 is a pointer, and can be modified to point elsewhere.


// 5.6 Pointer Arrays; Pointers to Pointers;
// Sorting text lines is a good illustration of pointer array usage. You may
// want to review program code p108 - p110.

// 5.7 Multi-dimensional array in C
// In C, a two-dimensional array is really a one-dimensional array, with 
// each element being an array(not a pointer). Elements are stored by rows.
// If a multiple-dimensional array is to be passed to a function, the parameter
// declaration must include the number of columns, so that the language knows
// how to adjust the step when you do pointer arithemetics. The number of rows
// is irrelevant, as it is passed as a pointer to array.
// When passing an array of 2 rows and 13 columns:
// f(int arr[2][13]); or
// f(int arr[][13]); or
// f(int (*arr)[13]); 
// Only the first dimensional is free, all the others have to be specified.

// 5.8 Initialization of Pointer Arrays
char *name[] = {"Jane", "Michael"};


// 5.9 Pointers vs Multi-dimensional arrays
int a[10][20];
int *b[10];
// a is a true 2-dimensional array: 200 int-sized locations have been allocated.
// For b, the definition only allocates 10 pointers and does not initialize
// them; initialization must be done explicitly.
// The important advantage of pointer array is that: the rows could be of
// different lengths.

// 5.10 Command-line Arguments
// Command-line arguments are passed to main(): main(int argc, char** argv).


// 5.11 Pointer to Functions
// In C, a function is not a variable, but it is possible to define pointers
// to functions. 
// Any pointer can be cast to void* and back again without loss of information.

int (*comp1) (void*, void*); 
int *comp2 (void*, void*);
// Declaration: comp1 is a pointer to a function that returns int and expecting 
// 2 void* arguments; comp2 is a function returning int and expecting 2 void*. 
// The parenthesis is necessary.


// 5.12 Complicated Declarations
int *f();  // f: Function return int pointer
int (*pf)();  // pf: pointer to function that returns int