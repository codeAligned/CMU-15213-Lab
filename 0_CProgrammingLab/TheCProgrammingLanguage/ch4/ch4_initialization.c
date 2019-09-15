// Without explicit initialization, external and static variables are
// initialized to zero. Automatic and register variables have undefined
// initial values.
// For external and static variables, the initializer must be a constant
// expression and the initialization is done once, before the program starts
// execution. For automatic and register varialbes, it is done each time the
// function or block is entered.

// An array may be initialized by following its declaration with a list of 
// initializers in curly braces. 
// At array explicit initialization, the array size can be omitted and it will
// be computed by the compiler.
// If there are fewer initializers than the number specified, zero would be
// filled for external, static and automatic variables. It's an error to have
// too many initializers. 
// Character arrays are special. A string can be used as an initializer and a
// null character is automatically appended.