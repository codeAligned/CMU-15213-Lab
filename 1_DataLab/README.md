# Lab1 DataLab

The original tar file is `datalab-handout.tar` and the writeup is
`datalab.pdf`. My solution is in `datalab-handout-solution`.

When working on WSL on Windows, I need to remove the `-m32` flag
in the `Makefile` in order to compile. In addition, on WSL, running
`./btest` gives me full mark and running `./dlc bits.c` gives no
warning or violation, but `./driver.pl` gives me zero mark. This might
be because of the removal of the `-m32` flag in `Makefile`.

`Possible solution.txt` contains sites that are helpful when working on the lab.  

### Difficult problems
1. `conditional`  
    Reference: https://stackoverflow.com/questions/10034470/conditional-statement-using-bitwise-operators/10034623#10034623  
    Most important technique:
    1. Use !! to convert into either 0 or 1; This is actually the way to
    check if a value is zero or non-zero, and get a one-bit result.
    2. Then negate the above to make all bits 0 or 1.  
    `x == 0` -> `!!(x) == 0` -> `~!!(x) == 0 == 0x00000000`;  
    `x != 0` -> `!!(x) == 1` -> `~!!(x) == -1 == 0xffffffff`.  
  

2. `isLessOrEqual`  
    When subtracting two numbers, `x` and `y`. The result `x - y` could overflow. 
    The only case when `x - y` are guaranteed to not overflow is when `x` and `y`
    are of the same sign. Thus, sign checking is very important in this problem.  

3. `logicalNegate`   
    Use the technique called `bit smearing`. You do not need to fully 
    understand how it works, just use the effect.  
    Check the links:   
    1. https://stackoverflow.com/questions/53161/find-the-highest-order-bit-in-c  
    2. https://stackoverflow.com/questions/10096599/bitwise-operations-equivalent-of-greater-than-operator/10097167#10097167
4. `howManyBits`  
    Probably the hardest problem in this lab. I use the solution from:  
    https://github.com/nikitos3000/cmu-15213-m14/blob/master/L1-datalab/bits.c  
    
5. `float_twice`  
    When dealing with floating point numbers, keep 2 points in mind:  
    1. Be aware of the special cases: `+0`, `-0`, `NaN`, `Infinity`, `denormalized`.  
    2. Be aware of possible overflow/underflow.  

    In this problem:  
    - When input is `+0`, `-0`, `NaN`, `Infinity`, return directly;  
    - When input is denormalized value, you just shift the bits 1 position to the left. Note that this actually handles the case for **the transition from denormalized representation to normalized representation**.   
    - When input is normalized, increment the `exp` field by `1`. Note that if the `exp` fields becomes all `1`'s after the increment, overflow happens. You need to make the `frac` field all `0`'s to represent `Infinity`.  

    This problem is the first problem of floating point number in this lab and takes me very long to solve. However, after this, the following two problems seems to be handable.
6. `float_i2f`
7. `float_i2f`  
    Be careful about whether rounding or truncation should happen.
