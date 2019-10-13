/*
 * mm-explicit.c
 * 
 * Explicit free list, with FIFO. Support first-fit and best-fit.
 * 
 * Scheme: 
 * "predecessor" and "successor" are in terms of heap.
 * "previous" and "next" are in terms of free list.
 * 
 * From the lecture slide, "next" and "prev" points to the 
 * header of the next/previous free block.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "FastLearn",
    /* First member's full name */
    "Fred Yue YIN",
    /* First member's email address */
    "yy0125@connect.hku.hk",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};



// #define DEBUG_MODE
// #define BEST_FIT

#ifdef DEBUG_MODE
    #define mm_checkheap(verbose) checkheap(verbose)
#else
    #define mm_checkheap(verbose)
#endif

#define WSIZE 4
#define DSIZE 8
#define QSIZE 16
#define MIN_BLOCKSIZE 24
#define CHUNKSIZE (1 << 12)

#define MAX_INT ((unsigned)((-1) << 1)) >> 1

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word (4-bytes) at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size or allocated fields from (a header/footer) at address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of header, footer, prev pointer and 
 * next pointer.
 */
#define HDRP(bp) ((char *)(bp)-3 * WSIZE)
#define NEXTP(bp) ((char *)(bp)-2 * WSIZE)
#define PREVP(bp) ((char *)(bp)-1 * WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - QSIZE)

/* Given block ptr bp, return the prev and next pointer themselves.
 */
#define NEXTV(bp) (GET(NEXTP(bp)))
#define PREVV(bp) (GET(PREVP(bp)))

/* Given block ptr bp, compute the block pointer to the predecessor 
 * and successor in the heaplist.
 */
#define PRED_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp)-4 * WSIZE)))
#define SUCC_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-3 * WSIZE)))

/* Given block ptr bp of a free block, compute the block pointer to 
 * the previous and next free block.
 */
#define NEXT_HDRP(bp) (GET(NEXTP(bp)))
#define PREV_HDRP(bp) (GET(PREVP(bp)))

#define NEXT_BLKP(bp) ((char *)NEXT_HDRP(bp) + 3 * WSIZE)
#define PREV_BLKP(bp) ((char *)PREV_HDRP(bp) + 3 * WSIZE)

/**
 * Two list exists in our system. 
 * Heap list: a list consisting of all blocks. 
 * Free list: a list consisting purely free blocks. 
 * 
 * 
 */
static char *heap_listp = 0; /* A block pointer pointing to the first block on the heap  */
static char *free_listp = 0; /* A block pointer pointing to the first free block in the free list */

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkheap(int verbose);
static void checkblock(void *bp);

/* 
 * mm_init - initialize the malloc package.
 * 1. Prologue block would have 4 words:
 * [Prologue header | next | prev (= NULL) | prologue footer]
 * 2. Epilogue block would be the same as in implicit list.
 * 3. Normal block would have minimum size of 6 words = 24 bytes.
 * 6 = 2 (header + footer) + 2 (pred + succ) + 2 (alignment requirement)
 */
int mm_init(void) {
    // printf("\nmm_init() called!\n");
    if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void *)-1) return -1;

    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(QSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), 0);              /* Prologue next */
    PUT(heap_listp + (3 * WSIZE), 0);              /* Prologue prev */
    PUT(heap_listp + (4 * WSIZE), PACK(QSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (5 * WSIZE), PACK(0, 1));     /* Epilogue header */

    heap_listp += (4 * WSIZE); /* heaplist_p */
    free_listp = heap_listp;

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    mm_checkheap(1);
    return 0;
}

static void *extend_heap(size_t words) {
    // printf("extend_heap() called!\n");
    char *nextp; /* pointer to the second word (next pointer) of extended block */
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    if ((long)(nextp = mem_sbrk(size)) == -1) return NULL;

    /* Initialize free block header/footer and the epilogue header */
    bp = nextp + DSIZE;
    PUT(HDRP(bp), PACK(size, 0));         /* header */
    PUT(FTRP(bp), PACK(size, 0));         /* footer */
    PUT(HDRP(SUCC_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    // Keep in mind: 
    // 1. coalesce() cannot be called until header and footer are updated.
    // 2. You should call coalesce only on a block pointer that have not been 
    // put on the free list. This is the assumption made by coalesce().
    return coalesce(bp);
}

/**
 * mm_free() would only setting the allocation bit to 0 in header and footer.
 * The newly freed block is not put onto the free list by mm_free(): this is
 * handled by coalesce.
*/
static void *coalesce(void *bp) {
#ifdef DEBUG_MODE
    printf("coalesce() called!\n");
#endif

    size_t prev_alloc = GET_ALLOC(FTRP(PRED_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(SUCC_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

#ifdef DEBUG_MODE
    if (prev_alloc && next_alloc) {
        printf("case 1, neither free\n");
    } else if (prev_alloc) {
        printf("case 2, successor free\n");
    } else if (next_alloc) {
        printf("case 3, predecessor free\n");
    } else {
        printf("case 4, both free\n");
    }
#endif

    if (prev_alloc && next_alloc) { /* Case 1 */
        // Both predcessor and successor are allocated.
        PUT(NEXTP(bp), HDRP(free_listp));
        PUT(PREVP(bp), 0);
        PUT(PREVP(NEXT_BLKP(bp)), HDRP(bp));
        free_listp = bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        // Predecessor is allocated, successor is free
        char *successor_bp;
        successor_bp = SUCC_BLKP(bp); // This must be before adjusting size

        // 1. Adjust the size
        size += GET_SIZE(HDRP(SUCC_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        if (PREVV(successor_bp)) {
#ifdef DEBUG_MODE
                printf("case 2.1 \n");
#endif
            // 2. Change pointers of blocks of predecessor and successor
            PUT(NEXTP(PREV_BLKP(successor_bp)), NEXTV(successor_bp));
            PUT(PREVP(NEXT_BLKP(successor_bp)), PREVV(successor_bp));

            // 3. Change pointers of the aggregated block
            PUT(NEXTP(bp), HDRP(free_listp)); /* next points to original head of free list */
            PUT(PREVP(bp), 0);                /* prev points to NULL */
            PUT(PREVP(free_listp), HDRP(bp));

            // 4. Update free list
            free_listp = bp;
        } else {
#ifdef DEBUG_MODE            
            printf("case 2.2 \n");
#endif            
            // 2. Change pointers of blocks of predecessor and successor
            PUT(PREVP(NEXT_BLKP(successor_bp)), HDRP(bp));

            // 3. Change pointers of the aggregated block
            PUT(NEXTP(bp), HDRP(NEXT_BLKP(successor_bp))); /* next points to original head of free list */
            PUT(PREVP(bp), 0);

            // 4. Update free list
            free_listp = bp;
        }
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        // Successor is allocated, predecessor is free

        // 1. Adjust the size
        size += GET_SIZE(HDRP(PRED_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PRED_BLKP(bp)), PACK(size, 0));

        bp = PRED_BLKP(bp);

        if (PREVV(bp)) {
            // 2. Change pointers of blocks of predecessor and successor
            PUT(NEXTP(PREV_BLKP(bp)), NEXTV(bp));
            PUT(PREVP(NEXT_BLKP(bp)), PREVV(bp));

            // 3. Change pointers of the aggregated block
            PUT(NEXTP(bp), HDRP(free_listp)); /* next points to original head of free list */
            PUT(PREVP(bp), 0);                /* prev points to NULL */

            // Update prev pointer of the original first free block
            PUT(PREVP(free_listp), HDRP(bp));

            // 4. Update free list
            free_listp = bp;
        } else {
            // 2. Change pointers of blocks of predecessor and successor
            // Nothing done

            // 3. Change pointers of the aggregated block
            // Nothing done

            // 4. Update free list
            // Nothing done
        }
    }

    else { /* Case 4 */
        // Both predecessor and successor are free

        // 1. Adjust the size
        size += GET_SIZE(HDRP(PRED_BLKP(bp))) + GET_SIZE(FTRP(SUCC_BLKP(bp)));
        PUT(HDRP(PRED_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(SUCC_BLKP(bp)), PACK(size, 0));

        if (PREVV(PRED_BLKP(bp)) && PREVV(SUCC_BLKP(bp))) {
#ifdef DEBUG_MODE
            printf("coalesce(): case 4.1 \n");
#endif
            // Neither predecessor or successor is at the start of the free list
            
            // 2. Change pointers of blocks of predecessor and successor
            PUT(NEXTP(PREV_BLKP(PRED_BLKP(bp))), NEXTV(PRED_BLKP(bp)));
            PUT(PREVP(NEXT_BLKP(PRED_BLKP(bp))), PREVV(PRED_BLKP(bp)));
            
            PUT(NEXTP(PREV_BLKP(SUCC_BLKP(bp))), NEXTV(SUCC_BLKP(bp)));
            PUT(PREVP(NEXT_BLKP(SUCC_BLKP(bp))), PREVV(SUCC_BLKP(bp)));

            bp = PRED_BLKP(bp);

            // 3. Change pointers of the aggregated block
            PUT(NEXTP(bp), HDRP(free_listp)); 
            PUT(PREVP(bp), 0);     
            PUT(PREVP(free_listp), HDRP(bp));    

            // 4. Update free list
            free_listp = bp;
        } else if (!PREVV(PRED_BLKP(bp))){
#ifdef DEBUG_MODE
            printf("coalesce(): case 4.2 \n");
#endif            
            // Predecessor is at the start of the free list

            // 2. Change pointers of blocks of successor
            PUT(NEXTP(PREV_BLKP(SUCC_BLKP(bp))), NEXTV(SUCC_BLKP(bp)));
            PUT(PREVP(NEXT_BLKP(SUCC_BLKP(bp))), PREVV(SUCC_BLKP(bp)));

            bp = PRED_BLKP(bp);

            // 3. Change pointers of the aggregated block
            // Nothing done        

            // 4. Update free list
            // Nothing done
        } else if (!PREVV(SUCC_BLKP(bp))) {
#ifdef DEBUG_MODE
            printf("coalesce(): case 4.3 \n");
#endif
            // Successor is at the start of the free list
            char *successor_bp;

            // 2. Change pointers of blocks of predecessor
            PUT(NEXTP(PREV_BLKP(PRED_BLKP(bp))), NEXTV(PRED_BLKP(bp)));
            PUT(PREVP(NEXT_BLKP(PRED_BLKP(bp))), PREVV(PRED_BLKP(bp)));

            successor_bp = SUCC_BLKP(bp);
            bp = PRED_BLKP(bp);

            // 3. Change pointers of the aggregated block
            PUT(NEXTP(bp), NEXTV(successor_bp));
            PUT(PREVP(bp), 0);

            PUT(PREVP(NEXT_BLKP(successor_bp)), HDRP(bp));

            // 4. Update free list
            free_listp = bp;
        }
    }

    mm_checkheap(1);
    return bp;
}

/*
 * mm_free
 * 
 * mm_free only sets the allocation bits of header and footer. It does not put
 * the block in the free list. This is done by function coalesce().
 */
void mm_free(void *bp) {
#ifdef DEBUG_MODE
    printf("mm_free called! bp = %p\n", bp);
#endif
    if (bp == 0)
        return;

    if (heap_listp == 0) {
        mm_init();
    }

    size_t size = GET_SIZE(HDRP(bp));

    // Update allocation bit in header and footer
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 * 
 * Parameter:
 * size: number of bytes required to be allocated.
 */
void *mm_malloc(size_t size) {
#ifdef DEBUG_MODE
    printf("mm_malloc() called! size = %d \n", size);
#endif

    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;          /* Block pointer */

    if (heap_listp == 0) {
        mm_init();
    }

    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = MIN_BLOCKSIZE;
    else
        asize = DSIZE * ((size + (QSIZE) + (DSIZE - 1)) / DSIZE);

#ifdef DEBUG_MODE
    printf("adjusted size: %d\n", asize);
#endif

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
#ifdef DEBUG_MODE
        // printf("find_fit: %p\n", bp);
#endif
        place(bp, asize);
        return bp;
    }

#ifdef DEBUG_MODE
    printf("find_fit() returns NULL!\n");
#endif

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;
    place(bp, asize);

    mm_checkheap(1);
    return bp;
}

/**
 * Current policy: always split the block if the remaining is not smaller
 * than the minimum block size.
 */
static void place(void *bp, size_t asize) {
#ifdef DEBUG_MODE
    printf("place() called! bp = %p \n", bp);
#endif

    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= MIN_BLOCKSIZE) {
        char *allocated_bp, *remaining_bp;

        allocated_bp = bp;
        PUT(HDRP(allocated_bp), PACK(asize, 1));
        PUT(FTRP(allocated_bp), PACK(asize, 1));

        // Update header and footer
        remaining_bp = SUCC_BLKP(allocated_bp);
        PUT(HDRP(remaining_bp), PACK(csize - asize, 0));
        PUT(FTRP(remaining_bp), PACK(csize - asize, 0));

        // Update free list pointers
        if (PREVV(allocated_bp)) {
            PUT(NEXTP(PREV_BLKP(allocated_bp)), HDRP(remaining_bp));
            PUT(PREVP(remaining_bp), PREVV(allocated_bp));
        } else {
            free_listp = remaining_bp;
            PUT(PREVP(remaining_bp), 0);
        }
        PUT(NEXTP(remaining_bp), NEXTV(allocated_bp));
        PUT(PREVP(NEXT_BLKP(allocated_bp)), HDRP(remaining_bp));
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));

        // Update free list pointers
        if (PREVV(bp)) {
            PUT(NEXTP(PREV_BLKP(bp)), NEXTV(bp));
            PUT(PREVP(NEXT_BLKP(bp)), PREVV(bp));
        } else {
            free_listp = NEXT_BLKP(bp);
            PUT(PREVP(NEXT_BLKP(bp)), 0);
        }
    }

    mm_checkheap(1);
}

static void *find_fit(size_t asize) {
    void *bp;
#ifdef BEST_FIT
    void *best_bp = NULL;
    int diff = MAX_INT;

    for (bp = free_listp; GET_SIZE(HDRP(bp)) > QSIZE; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            if (GET_SIZE(HDRP(bp)) - asize < diff) {
                best_bp = bp;
                diff = GET_SIZE(HDRP(bp)) - asize;
            }
        }
    }

    return best_bp ? best_bp : NULL;
#else
    // first fit
    for (bp = free_listp; GET_SIZE(HDRP(bp)) > QSIZE; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }

    return NULL;
#endif
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if (!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if (size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

static void printblock(void *bp) {
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: End Of heap, ", bp);
        printf("header: [%5ld:%c]\n", hsize, (halloc ? 'a' : 'f'));
        return;
    }

    printf("%p: header: [%5ld:%c] footer: [%5ld:%c]\n", 
            bp,  
            hsize, (halloc ? 'a' : 'f'),  
            fsize, (falloc ? 'a' : 'f'));
}

void printfreeblock(void *bp) {
    size_t hsize, halloc, fsize, falloc;
    char *next, *prev;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    next = GET(NEXTP(bp));
    prev = GET(PREVP(bp));

    if (hsize == 0) {
        printf("%p: End Of free list, ", bp);
        printf("header: [%5ld:%c]\n", hsize, (halloc ? 'a' : 'f'));
        return;
    }

    printf("%p: header: [%5ld:%c] next: [%p] prev: [%p] footer: [%5ld:%c]\n", 
            bp, 
            hsize, (halloc ? 'a' : 'f'),  
            next, 
            prev, 
            fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp) {
    if ((size_t)bp % 8) {
        printf("Error: %p is not doubleword aligned\n", bp);
    }
    if (GET(HDRP(bp)) != GET(FTRP(bp))) {
        printf("Error: header does not match footer\n");
    }
}

static void printfreelist() {
    char *bp;
    // int i = 0;

    printf("Free (%p):\n", free_listp);

    for (bp = free_listp; GET_SIZE(HDRP(bp)) > QSIZE; bp = NEXT_BLKP(bp)) {
        printf("%p\n", bp);
        checkblock(bp);
        printfreeblock(bp);
        // ++i;
        // if (i > 100) {
        //     printf("######### printfreelist traps ############\n");
        //     break; 
        // }
    }
    printfreeblock(bp);
}

/* 
 * checkheap - Minimal check of the heap for consistency 
 */
void checkheap(int verbose) {
    char *bp = heap_listp;
    int prevfree = 0, currentFree = 0;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    // Check (and print) prologue block
    if ((GET_SIZE(HDRP(heap_listp)) != QSIZE) || !GET_ALLOC(HDRP(heap_listp))) {
        printf("Bad prologue header: \n");
        printblock(bp);
        checkblock(heap_listp);
    }

    // 1. Check and print all blocks on the heap
    // 2. Check of continuous free blocks
    // 3. Check all free blocks are in the free list
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = SUCC_BLKP(bp)) {
        if (verbose)
            printblock(bp);
        checkblock(bp);

        currentFree = !GET_ALLOC(HDRP(bp));

        char *free_bp;
        if (currentFree) {
            int found = 0;
            for (free_bp = free_listp; GET_SIZE(HDRP(free_bp)) > QSIZE; free_bp = NEXT_BLKP(free_bp)) {
                if (bp == free_bp) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("### Free block not in free list ### \n");
            }
        }

        // Check continuous free blocks
        if (currentFree && prevfree) {
            printblock("### Continuous free blocks! ### \n");
            printblock(bp);
        }
        prevfree = currentFree;
    }

    // Print epilogue block
    if (verbose)
        printblock(bp);

    // Check (and print) epilogue block
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
        printf("Bad epilogue header: \n");
        printblock(bp);
    }

    printfreelist();
    printf("-----------\n");
}
