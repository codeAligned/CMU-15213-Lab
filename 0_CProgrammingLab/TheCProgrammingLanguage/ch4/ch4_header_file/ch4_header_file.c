// We now have main.c, stack.c, getop.c and getch.c.
// Another thing: now the declarations and definitions are spreaded across
// files, we want to centralize this as much as possible, so that we always 
// need to worry about one copy only. We place this common material in a
// header file, calc.h.

// The static declaration, when applied to external variables or functions,
// limits the scope of that object to the rest of the source file being 
// compiled.
// From StackOverflow: https://stackoverflow.com/a/572550
// 1. A static variable inside a function keeps its value between invocations.
// 2. A static global variable or a function is "seen" only in the file it's 
// declared in