# Lab6 Malloc Lab

The original tar file is `malloclab-handout.tar` and the writeup is `malloclab.pdf`. The implicit list solution is in `./malloclab-implicit`, the explicit list solution is in `./malloclab-explicit`.

### Before starting 
Read `9.9` of `CSAPP: 3e` very carefully, make sure you understand each line of code.   

As indicated in the handout and the lecture, this could be the hardest among all labs. Be prepared.


### Implicit List Solution
Read through the code in `CSAPP: 3e` `9.9` (which could also be found at `./vm/malloc/`), make sure you totally understand the code. Then you easily get the solution for the implicit list.

### Explicit List Solution
Actually implementing the explict list turns out to be harder than I could imagine. The difficulty comes from several aspects:
- The explicit list is not heavily explained in the textbook or lecture, and lacks the details/corner cases required to implement it in real C code.  
- Be careful about the difference between `prev`/`next`, and `pred`/`succ`.  
- Be careful about setting the ending signal block for the free list.  
- `mm_free()` only updates the allocation bit, `coalesce()` puts the coalesced block in the free list.
- Casting is heavily used in this lab, especially in the macros. It seems very hard to do it to eliminate all warnings. In the end, I just ignore the warnings. But be very careful about pointer casting and pointer arithmetics. You may want to review the lecture slides and recitation slides.  
- Heap checker is very useful for debugging. 
- Deal with cornor cases very carefully. In terms of free list, the corner case occurs when the block is the first block in the free list, then it would have no previous block.


I use VirtualBox Ubuntu virtual machine for this lab, because of the `-m32` flag in the `Makefile`. The file sharing feature is very handy. `less` is helpful for debugging and tracing the execution. I did not use `gdb` or `valgrind` for this program.



