#include <stdio.h>

// 6.1 Basics of Structure
struct myStruct1 {
    // "point" is called a structure tag, which is optional;
    // x, y are members of the structure.
    int x;
    int y;
};

int i1, i2, i3;
struct point {
    int x;
    int y;
} pt1, pt2, pt3;

// A structure not followed by a list of variables reserves no memory storage.
// It only describes a template of the struture.

struct point pt4;
struct point pt5 = {1, 2};

// Structures can be nested
struct rect {
    struct point leftLower;
    struct point rightUpper;
};

// 6.2 Structures and Functions
// Only legal operations on a structure:
// 1. Copying/Assigning a structure;
// 2. Taking the address with &;
// 3. Access members.
// Structures cannot be compared.

// When using structures in functions, there are 3 approaches:
// 1. Passing components separately;
// 2. Passing the entire structure;
// 3. Passing a pointer to the structure.

struct point makepoint(int x, int y) {
    struct point temp = {x, y};
    return temp;
}

struct point addpoint(struct point p1, struct point p2) {
    // We can use p1 directly, as the argument is passed by value.
    p1.x += p2.x;
    p1.y += p2.y;
    return p1;
}

void addpointTest() {
    struct point p1 = {1, 2}, p2 = {3, 4}, p3;
    printf("p1: (%d, %d)\n", p1.x, p1.y);
    p3 = addpoint(p1, p2);
    printf("p1: (%d, %d)\n", p1.x, p1.y);
    printf("p3: (%d, %d)\n", p3.x, p3.y);
}

// You can use "p->mem", as a shorthand for "(*p).mem"
// The structure operator . and ->, together with () for function call and []
// for array subscripts are at the top of the predecence hierarchy.

// 6.3 Arrays of Structures
struct pair {
    char *key;
    int count;
} pairTable[] = {
    "auto", 0,
    "break", 0
    // Inner curly braces are optional
};

// Compile-time operator to determine size of an object or type. The returned
// value is or type size_t, which is unsigned.
// sizeof object
// sizeof (type)
// Do not assume that the size of a structure is the sum of the sizes of all
// its members. For alignment requirements, the actual size could be larger.
// Alignment requirement example: many machine requires that integers be
// located at even addresses.
#define NUM_KEYS (sizeof pairTable / sizeof(struct pair))
#define NUM_KEYS2 (sizeof pairTable / sizeof pairTable[0])


// 6.4 Pointers to Structures
// Special care may be need to convert a program that uses structure directly
// to one that uses pointers to structures. Make sure that you do not write
// any illegal dereferencing or arithemitic out of bound.


// 6.5 Self-referencing structure
// It is illegal for a structure to contain an instance of itself, but it
// can have pointers to instance of itself.

// We generally want one program to have only one allocator, that allocates
// different kinds of objects. But it has to cope with 2 problems: 1. alignment
// requirements; 2. It should return different types of pointers accordingly.
// Answer:
// Alignment requirement can be satisfied at the cost of some wasted space. 
// You can declare the allocator to return void* and then explicitly cast the
// pointer into the desired type with a cast.

// 6.7 typedef
typedef char *String;

typedef struct tnode *Treeptr;
typedef struct tnode {
    char *word;
    int count;
    Treeptr left;
    Treeptr right;
} Treenode;

typedef int (*functionReturnInt) (void *, void*);  
// functionReturnInt is a type of pointer to function that returns int and 
// accepts 2 void* arguments.

int main() {
    addpointTest();
}