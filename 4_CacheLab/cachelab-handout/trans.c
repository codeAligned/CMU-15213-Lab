/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int N, int M, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

/*
s = 5, E = 1, b = 5. 
Block/Line size = 32 bytes = 8 ints.
1 set = 1 line.
Total sets = 32.
Cache size = 8 * 32 = 2 ^ 8 = 16 x 16 = 256 ints.

Note that initially, each entry in B contains some random value!!!
*/

int is_transpose(int N, int M, int A[N][M], int B[M][N]);

void print_matrix(int N, int M, int Mat[N][M]) {
    int i1, j1;
    for (i1 = 0; i1 < N; ++i1) {
        for (j1 = 0; j1 < M; ++j1) {
            printf("%d ", Mat[i1][j1]);
        }
        printf("\n");
    }
    printf("\n");
}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

/*
Use 8 x 8 block size. Reasoning:
For N = M = 32, each row contains 32 ints. So the cache can store at most 8
rows of input. Thus, the block size  <= 8. In general, we want to have larger 
block size.

The code block for diagonal block should cause 23 misses: 23 = 7 + 2 * 8.
When M = N = 32, 287  = 3 + 16 * 12 + 23 * 4, very close to estimation.
*/
void blocksize_8_32_32(int N, int M, int A[N][M], int B[M][N]) {
    int i1, j1, i0, j0;  // i0, j0 stands for index at element level
                         // i1, j1 stands for index of level1 block start
    for (i1 = 0; i1 < N; i1 += 8) {
        for (j1 = 0; j1 < M; j1 += 8) {
            if (i1 == j1) {  // Diagonal block
                // The diagonal block requires special treatment because the
                // first row of B evicts the first row of A
                for (i0 = i1; i0 < i1 + 8; ++i0) {
                    for (j0 = j1; j0 < j1 + 8; ++j0) {
                        if (i0 == j0) {
                            continue;
                        } else {
                            B[j0][i0] = A[i0][j0];
                        }
                    }

                    // Avoid evication
                    B[i0][i0] = A[i0][i0];
                }
            } else {  // Non-diagonal block, no eviction problem
                for (i0 = i1; i0 < i1 + 8; ++i0) {
                    for (j0 = j1; j0 < j1 + 8; ++j0) {
                        B[i0][j0] = A[j0][i0];
                    }
                }
            }
        }
    }
}

/*
For N = M = 64, each row contains 64 ints. So the cache can store at most 4 rows
of input. Thus, the block size <= 4. When we run with blocksize of 4, output:

$ ./test-trans -N 64 -M 64
Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:6498, misses:1699, evictions:1667

Summary for official submission (func 0): correctness=1 misses=1699

Result interpretation: 1699 = 3 + 32 * 11 + (16 * 16 - 32) / 2 * 12.
- 32 corresponds to the 32 4x4 blocks on the diagonal of the input, with the 
upper-left element's index (i1, j1) satisfying:
i1 == j1 || (i1 == j1 + 4 && i1 % 8 == 4) || (i1 == j1 - 4 && i1 % 8 == 0)).
Thus blocks needs 3 + 2 * 4 = 11 misses.

- (16 * 16 - 32) corresponds to remaining 4x4 blocks. Each 2 consecutive 4x4 
blocks in the same row would cause 12 cache misses. 

The reason for the above result is that: in terms of reading from cache, we are
doing well as we are processing one row at a time, using all 8 ints in the 
cache line. However, in terms of writing, we are not using cache efficiently. 
Each time we are only writing to the first 4 or last 4 ints in one cache line.

We try to reduce cache misses during the writing process into B.
*/

void blocksize_4_64_64(int N, int M, int A[N][M], int B[M][N]) {
    int i1, j1, i0, j0;  // i0, j0 stands for index at element level
                         // i1, j1 stands for index of level1 block start
    int v0, v1, v2, v3;
    for (i1 = 0; i1 < N; i1 += 4) {
        for (j1 = 0; j1 < M; j1 += 4) {
            if (i1 == j1 ||
                (i1 == j1 + 4 && i1 % 8 == 4) ||
                (i1 == j1 - 4 && i1 % 8 == 0)) {
                for (i0 = i1, j0 = j1; i0 < i1 + 4; ++i0) {
                    v0 = A[i0][j0];
                    v1 = A[i0][j0 + 1];
                    v2 = A[i0][j0 + 2];
                    v3 = A[i0][j0 + 3];
                    B[j0][i0] = v0;
                    B[j0 + 1][i0] = v1;
                    B[j0 + 2][i0] = v2;
                    B[j0 + 3][i0] = v3;
                }
            } else {  // Non-diagonal block, no eviction problem
                for (i0 = i1; i0 < i1 + 4; ++i0) {
                    for (j0 = j1; j0 < j1 + 4; ++j0) {
                        B[i0][j0] = A[j0][i0];
                    }
                }
            }
        }
    }
}

/*
$ ./test-trans -M 64 -N 64

Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:12098, misses:1475, evictions:1443

Summary for official submission (func 0): correctness=1 misses=1475

1475 = 3 + 32 * 11 + (16 * 16 - 32) / 4 * 20
*/
void blocksize_8_4_64_64(int N, int M, int A[N][M], int B[M][N]) {
    int i2, j2, i1, j1, i0, j0;  // i0, j0 stands for index at element level
                                 // i1, j1 stands for index of upper-left corner of 4x4 block
                                 // i2, j2 stands for index of upper-left corner of 8x8 block
    int v0, v1, v2, v3;
    int temp;
    for (i2 = 0; i2 < N; i2 += 8) {
        for (j2 = 0; j2 < M; j2 += 8) {
            if (i2 == j2) {  // 8x8 block along the diagonal
                for (i1 = i2; i1 < i2 + 8; i1 += 4) {
                    for (j1 = j2; j1 < j2 + 8; j1 += 4) {
                        for (i0 = i1, j0 = j1; i0 < i1 + 4; ++i0) {
                            v0 = A[i0][j0];
                            v1 = A[i0][j0 + 1];
                            v2 = A[i0][j0 + 2];
                            v3 = A[i0][j0 + 3];
                            B[j0][i0] = v0;
                            B[j0 + 1][i0] = v1;
                            B[j0 + 2][i0] = v2;
                            B[j0 + 3][i0] = v3;
                        }
                    }
                }
            } else {  // non-diagonal 8x8 block
                /*
                -------------
                |  1  |  2  |
                -------------
                |  3  |  4  |
                ------------
                1, 2, 3, 4 refers to the position within 8x8 block, not the content.
                subblock 1, 2, 3, 4 are all 4x4 block.
                */
                // 1. Copy subblock (1, 2) from A to B, with subblock 1 transposed within itself, subblock 2 unchanged
                // The subblock 1 has index (i2, j2) in A, (j2, i2) in A
                for (i0 = i2; i0 < i2 + 4; ++i0) {
                    for (j0 = j2; j0 < j2 + 8; ++j0) {
                        if (j0 < j2 + 4) {  // subblock 1
                            B[j0][i0] = A[i0][j0];
                        } else {  // subblock 2
                            B[j2 + (i0 - i2)][i2 + (j0 - j2)] = A[i0][j0];
                        }
                    }
                }

                // 2. Swap the subblock 3 in A with subblock 2 in B specially,
                // such that after the swapping, the two subblocks are transposed with themselves respectively.
                // The upper-left corner of the two 4x4 blocks are:
                // (i2 + 4, j2) of 3 in A, (j2, i2 + 4) of 2 in B
                for (i0 = i2 + 4; i0 < i2 + 8; ++i0) {
                    for (j0 = j2; j0 < j2 + 4; ++j0) {
                        temp = A[i0][j0];
                        A[i0][j0] = B[j0][i0];
                        B[j0][i0] = temp;
                    }
                }

                // 3. Copy subblock 3 from A to B, unchanged
                // The upperleft has index (i2 + 4, j2) in A, (j2 + 4, i2) in B
                for (i0 = i2 + 4; i0 < i2 + 8; ++i0) {
                    for (j0 = j2; j0 < j2 + 4; ++j0) {
                        B[j2 + 4 + (i0 - i2 - 4)][i2 + j0 - j2] = A[i0][j0];
                    }
                }

                // 4. Copy sublock 4 from A to B, transposed within itself
                for (i0 = i2 + 4; i0 < i2 + 8; ++i0) {
                    for (j0 = j2 + 4; j0 < j2 + 8; ++j0) {
                        B[j0][i0] = A[i0][j0];
                    }
                }

                // 5. Copy back to A to make sure A's content is not changed
                for (i0 = i2 + 4; i0 < i2 + 8; ++i0) {
                    for (j0 = j2; j0 < j2 + 4; ++j0) {
                        A[i0][j0] = B[j0][i0];
                    }
                }
            }
        }
    }
}


/*
When B_SIZE = 8:
Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:6061, misses:2118, evictions:2086

Summary for official submission (func 0): correctness=1 misses=2118

TEST_TRANS_RESULTS=1:2118

When B_SIZE = 16:
Function 0 (1 total)
Step 1: Validating and generating memory traces
Step 2: Evaluating performance (s=5, E=1, b=5)
func 0 (Transpose submission): hits:6187, misses:1992, evictions:1960

Summary for official submission (func 0): correctness=1 misses=1992

TEST_TRANS_RESULTS=1:1992
*/
#define B_SIZE 16
void blocksize_4_67_61(int N, int M, int A[N][M], int B[M][N]) {
    int i, j, k, h;
    for (i = 0; i < N; i += B_SIZE) {
        for (j = 0; j < M; j += B_SIZE) {
            for (k = i; k < i + B_SIZE && k < N; k++) {
                for (h = j; h < j + B_SIZE && h < M; h++) {
                    B[h][k] = A[k][h];
                }
            }
        }
    }
}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    if (N == M) {
        if (N <= 32) {
            blocksize_8_32_32(N, M, A, B);
        } else {
            blocksize_8_4_64_64(N, M, A, B);
        }
    } else if (N == 67 && M == 61) {
        blocksize_4_67_61(N, M, A, B);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise transpose";
void trans(int N, int M, int A[N][M], int B[M][N]) {
    int i1, j1, tmp;

    for (i1 = 0; i1 < N; i1++) {
        for (j1 = 0; j1 < M; j1++) {
            tmp = A[i1][j1];
            B[j1][i1] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int N, int M, int A[N][M], int B[M][N]) {
    int i1, j1;

    for (i1 = 0; i1 < N; i1++) {
        for (j1 = 0; j1 < M; ++j1) {
            if (A[i1][j1] != B[j1][i1]) {
                return 0;
            }
        }
    }
    return 1;
}
