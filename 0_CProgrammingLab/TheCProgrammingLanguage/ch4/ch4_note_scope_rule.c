// The scope of an external variable or a function lasts from the point of
// declaration to the end of the file being compiled. 
// A function cannot see functions/variables declared below it.
// If an external variable is to be used before definition, or it is defined in
// another file, then an extern declaration is necessary.

// Declaration announces the existence and property of a variable;
// Definition allocates memory for it.

// There must be only one definition of an external variable among all files
// that make up the source program, other files can contain extern declarations
// to access it. Array size is required at definition, but optional at
// declaration.