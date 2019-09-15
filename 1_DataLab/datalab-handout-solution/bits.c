/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

/*
Bit manipulation techniques:
Example 1: evaluate the difference between two integers.
Just need to check the sign bit and the OR of remaining bits.
https://stackoverflow.com/questions/46136104/bitwise-operation-in-c-to-compare-two-integers
1. You can use shifting only, to tell a particular bit(the n-th bit) in a 
signed integer is 0 or 1: If this bit is the MSB, right shift. If it is not the
most significant bit, left shift and then right shift.
Example: int a = 0x1000, n = 0. Use a >> 3.
Example: int b = 1010, n = 1. Use (b << 2) >> 3.
2. You can use !! to check if a integer has all bits being 0 or not.

Example 2: conditional using bit operations.
https://stackoverflow.com/questions/10034470/conditional-statement-using-bitwise-operators
3. Use !! to convert into either 0 or 1; This is actually the way to 
check if a value is zero or non-zero, and get a one-bit result.
4. Then negate the result from 3 to make all bits 0 or 1.
*/

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    int oneIfBothOne = x & y;
    int zeroIfBothOne = ~oneIfBothOne;
    int oneIfContainsOne = ~((~x) & (~y));
    int result = zeroIfBothOne & oneIfContainsOne;
    return result;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    int result = (~1 + 1) << 31;
    return result;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
int isTmax(int x) {
    // TMAX = 0x011..11
    // 1. Not all bits are 1
    int reversed = !!(~x);  // 1 for 0x011.11; 0 for 0x11..11

    // 2. 2x + 2 == 0, for both 0x011.11 and 0x11..11
    return !(x + x + 1 + reversed);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
    int slice = 170, oddOneEvenZero = 170;
    oddOneEvenZero = (oddOneEvenZero << 8) + slice;
    oddOneEvenZero = (oddOneEvenZero << 8) + slice;
    oddOneEvenZero = (oddOneEvenZero << 8) + slice;
    return !(~(oddOneEvenZero & x) + oddOneEvenZero + 1);
    // If you want to check if integer a and b are equal:
    // ~a + b + 1 evaluates to 0 if a == b, using 3 operators.
}

/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    return (~x) + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    // Operator count exceed.
    // int diff1 = x + (~'0' + 1);
    // int diff2 = x + (~'9' + 1);
    // int diffSum = diff1 + diff2;
    // return !(x >> 6 | (9 + ~diffSum + 1) >> 31 | (diffSum + 9) >> 31);

    // Operator count: 12.
    // '0' = 48, '9' = 57, diffSum = x + x - 48 - 57 = 2x - 105
    int diffSum = (x << 1) + (~105 + 1);
    return !(x >> 6 | (~diffSum + 10) >> 31 | (diffSum + 9) >> 31);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
    // Cannot do.
    // Reference: https://stackoverflow.com/questions/10034470/conditional-statement-using-bitwise-operators/10034623#10034623
    // Most important technique:
    // 1. Use !! to convert into either 0 or 1; This is actually the way to
    // check if a value is zero or non-zero, and get a one-bit result.
    // 2. Then negate it to make all bits 0 or 1.
    // x == 0 -> twoCases == 0 -> allSameBits == 0
    // x != 0 -> twoCases == 1 -> allSameBits == -1

    // int twoCases = !!x;
    // int allSameBits = ~twoCases + 1;
    // return (allSameBits & y) | ((~allSameBits) & z);
    return ((~(!!x) + 1) & y) | ((~(~(!!x) + 1)) & z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    // The main idea is to compute the difference between x and y: x + ~y + 1
    // But this overflows when x, y have different sign.
    // Thus you should use subtraction only when x, y have same sign bit.
    // When x, y have differnt sign, you can get the result trivally.

    // Check sign
    // 6 ops
    int xSign = (x >> 31) & 1;
    int ySign = (y >> 31) & 1;
    int xNegativeYNonNegativeFlag = xSign & !ySign;

    // Subtract when same sign
    // 12 ops
    int diff = x + ~y + 1;
    int diffSign = (diff >> 31) & 1;
    int sameSignFlag = !(xSign ^ ySign);
    int diffNonPositveFlag = sameSignFlag & (~(!!diff) | diffSign);

    // 2 ops
    return (xNegativeYNonNegativeFlag | diffNonPositveFlag);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
    // Method: Use bit smearing
    // You do not need to understand how it works, just use the effect.
    // https://stackoverflow.com/questions/53161/find-the-highest-order-bit-in-c
    // https://stackoverflow.com/questions/10096599/bitwise-operations-equivalent-of-greater-than-operator/10097167#10097167
    int smeard = x;
    smeard = smeard | smeard >> 16;
    smeard = smeard | smeard >> 8;
    smeard = smeard | smeard >> 4;
    smeard = smeard | smeard >> 2;
    smeard = smeard | smeard >> 1;
    return (~smeard & 1);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5       [0]1100
 *            howManyBits(298) = 10     [0]100101010
 *            howManyBits(-5) = 4       [1]011
 *            howManyBits(0)  = 1       [0]
 *            howManyBits(-1) = 1       [1]
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    // Cannot do.
    // https://github.com/nikitos3000/cmu-15213-m14/blob/master/L1-datalab/bits.c
    int y, result, mask16, mask8, mask4, mask2, mask1, bitnum;

    mask1 = 0x2;                    // 0x1 << 1
    mask2 = 0xC;                    // 0x3 << 2
    mask4 = 0xF0;                   // 0x000000F0
    mask8 = 0xFF << 8;              // 0x0000FF00
    mask16 = (mask8 | 0xFF) << 16;  // 0xFFFF0000

    result = 1;
    y = x ^ (x >> 31);  //cast the number to positive with the same result

    // Check first 16 bits, if they have at least one bit - result > 16
    bitnum = (!!(y & mask16)) << 4;  // 16 OR zero
    result += bitnum;
    y = y >> bitnum;

    bitnum = (!!(y & mask8)) << 3;  // 8 OR zero
    result += bitnum;
    y = y >> bitnum;

    bitnum = (!!(y & mask4)) << 2;  // 4 OR zero
    result += bitnum;
    y = y >> bitnum;

    bitnum = (!!(y & mask2)) << 1;  // 2 OR zero
    result += bitnum;
    y = y >> bitnum;

    bitnum = !!(y & mask1);  // 1 OR zero
    result += bitnum;
    y = y >> bitnum;

    return result + (y & 1);
}
//float
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
    // Denormalized case is the most tricky one
    // http://siofive.com/2017/10/16/cmu15213-datalab/#float-twice
    // https://zhuanlan.zhihu.com/p/28335741

    // IEEE format: [1: sign bit][8: exp][23: Mantissa]
    // Rightshifting unsigned is logical sift: fill with 0.
    // Return NaN as itself

    // Special case: 0, NaN, infinity, denormalized
    // 1. For NaN, 0, infinity: return directly
    // For denormalized: shift Mtail. But when Mtail exceeds, rounding.
    unsigned expMask = 0xFF;
    unsigned exp = expMask & (uf >> 23);
    unsigned Mtail = (uf << 9) >> 9;

    unsigned expAllZeroFlag = !exp;
    unsigned expAllOneFlag = !(exp ^ expMask);
    unsigned MtailAllZeroFlag = !(Mtail);

    unsigned signBitExpMask = 0xff800000;
    unsigned signBitMask = 0x80000000;

    unsigned oneExp = 0x000800000;

    // exp all 1's: this is infinity or NaN.
    // Return uf directly since:
    // return itself if uf is NaN;
    // any operation on infinity is infinity.
    if (expAllOneFlag) {
        // if exp is 0x1111..11
        // Return directly.
    } else if (expAllZeroFlag) {
        if (MtailAllZeroFlag) {  // The number is 0

        } else {
            // The number is non zero, denormalized
            unsigned onlyKeepSignBit = signBitMask & uf;
            uf = onlyKeepSignBit | (uf << 1);
        }
    } else {
        if (!((exp + 1) ^ 0xff)) {
            // if exp + 1 == 0xff, i.e., adding one to exp makes it all 1's
            // Return infinity.
            uf = uf + oneExp;

            // Makes Mtail all zero
            uf = uf & signBitExpMask;
        } else {
            uf = uf + oneExp;
        }
    }
    return uf;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */

unsigned float_i2f(int xi) {
    unsigned xu, signBit, temp, signBitMask, frac, bias, exp, firstOneMask,
        xuWithOriginalFirstOneSetToZero, roundBitIdx, roundBit,
        stickyBitSource, guardBit;
    int firstOneIdx;

    signBitMask = 0x80000000;
    signBit = exp = frac = 0;
    xu = xi;

    // Reverse negative int to positive int.
    signBit = xu & signBitMask;
    xu = signBit ? -xu : xu;

    // Determine the index first non zero bit.
    // We count from least significant bit in this function, starting from 0.
    temp = xu;
    firstOneIdx = 31;
    for (; (firstOneIdx >= 0) && !(temp & signBitMask);) {
        firstOneIdx = firstOneIdx - 1;
        temp = temp << 1;
    }

    // Compute fractional and exp
    bias = 0x7f;  // 0x0111 1111
    firstOneMask = 1 << firstOneIdx;
    xuWithOriginalFirstOneSetToZero = xu - firstOneMask;
    // Align bits after firstOneIdx in x to the fractional in the final result
    if (firstOneIdx <= 23) {
        // There are no more than 23 bits avaiable
        // The first bit starts are indix (firstOneIdx - 1)
        // Should be moved to index 22
        frac = xuWithOriginalFirstOneSetToZero << (23 - firstOneIdx);
    } else {
        // More than 23 bits, need to do rounding
        // Guard bit: Least significant bit of result
        // Round bit: 1st bit that would be removed
        // Sticky bit: OR of bits after round bit.
        roundBitIdx = firstOneIdx - 24;
        roundBit = (xu >> roundBitIdx) & 1;

        // Bits BEHIND(not including) the first bit of one would be
        // fit into significand
        frac = xuWithOriginalFirstOneSetToZero >> (firstOneIdx - 23);

        if (roundBit) {
            // Round bit is 1, need to check sticky bits
            stickyBitSource = (xu << (32 - roundBitIdx));
            if (stickyBitSource) {
                // More than half
                frac = frac + 1;
            } else {
                // Exactly half, check the bit before roundBit
                // Check the bit before the roundBit
                guardBit = (xu >> (roundBitIdx + 1)) & 1;
                if (guardBit) {
                    frac = frac + 1;
                }
            }
        }
    }
    exp = firstOneIdx + bias;
    return xu ? signBit + (exp << 23) + frac : 0;
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
    unsigned res, expMask, exp, fracMask, frac, outOfRange, M, bias, temp,
        fractionalFirstBitMask, signBitMask, signBit;
    int firstOneInFractionalIdx, E;

    // If uf is +0 or -0, return 0.
    if (!(uf << 1)) {
        return 0;
    }

    // Shift exp to the right end and do operation
    expMask = 0x7f800000;
    exp = (expMask & uf) >> 23;

    fracMask = 0x008ffff;
    frac = fracMask & uf;

    outOfRange = 0x80000000u;
    // E could be positive or negative, use int type
    bias = 0x7f;

    if (exp == expMask) {  // Exp bits are all 1's, NaN and infinity
        res = outOfRange;
    } else if (!exp) {  // Exp bits are all 0's, denormalized
        return 0;
    } else {  // Normalized
        E = exp - bias;
        // printf("E = %d\n", E);
        if (E < 0) {
            // printf("E < 0 !!!\n");
            return 0;
        } else if (E > 30) {
            // printf("E > 30 !!!\n");
            return outOfRange;
        }

        temp = uf;
        firstOneInFractionalIdx = 22;
        fractionalFirstBitMask = 0x00400000;
        for (; (firstOneInFractionalIdx >= 0) && !(temp & fractionalFirstBitMask);) {
            firstOneInFractionalIdx -= 1;
            temp = temp << 1;
        }
        // printf("firstOneInFractionalIdx: %d\n", firstOneInFractionalIdx);

        M = 0x00800000 + frac;
        if (E > 23) {
            res = M << (E - 23);
        } else {
            res = M >> (23 - E);
        }
    }

    // Handle signbit
    signBitMask = 0x80000000;
    signBit = signBitMask & uf;
    if (signBit) {
        res = -res;
    }

    // Test Cases
    /*
    // positive and negative zero - done
    printf("0x%08x, should be 0x00000000\n", float_f2i(0x80000000));
    printf("0x%08x, should be 0x00000000\n\n", float_f2i(0x00000000));

    // NaN and Infinity - done
    printf("0x%08x, should be 0x80000000\n", float_f2i(0x7f800000));
    printf("0x%08x, should be 0x80000000\n", float_f2i(0xff800000));
    printf("0x%08x, should be 0x80000000\n", float_f2i(0x7f800001));
    printf("0x%08x, should be 0x80000000\n\n", float_f2i(0xff800001));

    // Denormalized - done
    printf("0x%08x, should be 0x00000000\n", float_f2i(0x007fffff));
    printf("0x%08x, should be 0x00000000\n", float_f2i(0x00700000));
    printf("0x%08x, should be 0x00000000\n", float_f2i(0x007f8001));
    printf("0x%08x, should be 0x00000000\n\n", float_f2i(0x00700001));

    // Normalized
    // E < 0
    printf("0x%08x, should be 0x00000000\n", float_f2i(0x00800002));
    printf("0x%08x, should be 0x00000000\n", float_f2i(0x0e000000));
    // E > 30
    printf("0x%08x, should be 0x80000000\n", float_f2i(0x4f000000));
    printf("0x%08x, should be 0x80000000\n\n", float_f2i(0x7f000000));
    // 0 <= E && E <= 30
    printf("0x%08x, should be 0x00000001\n", float_f2i(0x3f800001));
    printf("0x%08x, should be 0x40000080\n", float_f2i(0x4e800001));

    // Negative
    printf("0x%08x, should be 0xffffffff\n", float_f2i(0xbf800000));
    */

    return res;
}
