/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
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

Note that initially, each entry in B contains some random value!!!
*/

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void print_matrix(int N, int M, int Mat[N][M]) {
    int i, j;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < M; ++j) {
            printf("%d ", Mat[i][j]);
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

#define B_SIZE 8
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int temp;
    int i, j, ii, jj;  // ii, jj stands for index at element level
                       // i, j stands for index of block start

    if ((N == 32 && M == 32) || (N == 16 && M == 16) || (N == 8 && M == 8)) {
        // ./test-trans -M 8 -N 8 gives 26 misses.
        for (i = 0; i < N; i += B_SIZE) {
            for (j = 0; j < M; j += B_SIZE) {
                if (i == j) {
                    /*
                    Diagonal block. This code block should induce 23 misses
                    for the 8 x 8 blocking: 23 = 7 + 2 * 8.
                    Then we find that the actual miss time is 26 for M = N = 8.
                    I believe the extra 3 misses is the function overhead.
                    This guessing is consistent with cases when M = N = 16 and
                    M = N = 32.
                    When M = N = 16, 81   = 3 + 16 * 2  + 23 * 2.
                    When M = N = 32, 287  = 3 + 16 * 12 + 23 * 4.

                    This does not extend to M = N = 64. The reason is that the 
                    total number of cache lines is 32. Extra eviction happens.
                    */
                    for (ii = i; ii < i + B_SIZE; ++ii) {
                        for (jj = j; jj < j + B_SIZE; ++jj) {
                            if (ii == jj) {
                                continue;
                            } else {
                                B[jj][ii] = A[ii][jj];
                            }
                        }

                        // Avoid evication
                        temp = A[ii][ii];
                        B[ii][ii] = temp;
                    }
                } else {  // Non-diagonal block, no eviction problem
                    for (ii = i; ii < i + B_SIZE; ++ii) {
                        for (jj = j; jj < j + B_SIZE; ++jj) {
                            B[ii][jj] = A[jj][ii];
                        }
                    }
                }
            }
        }
    } else if (N == 64 && M == 64) {
    } else if (N == 61 && M == 67) {
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
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
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
