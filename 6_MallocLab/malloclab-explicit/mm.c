/*
 * mm-explicit.c
 * 
 * Explicit free list, with FIFO.
 * 
 * Scheme: 
 * "predecessor" and "successor" are in terms of heap.
 * "previous" and "next" are in terms of free list.
 * 
 * From the lecture slide, "next" and "prev" points to the 
 * header of the next/previous free block.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

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

#define WSIZE 4
#define DSIZE 8
#define QSIZE 16
#define MIN_BLOCKSIZE 24
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word (4-bytes) at address p */
#define GET(p)              (*(unsigned int *)(p))
#define PUT(p, val)         (*(unsigned int *)(p) = (val))

/* Read the size or allocated fields from (a header/footer) at address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of header, footer, prev pointer and 
 * next pointer.
 */
#define HDRP(bp)  ((char *)(bp) - 3 * WSIZE)
#define NEXTP(bp) ((char *)(bp) - 2 * WSIZE)
#define PREVP(bp) ((char *)(bp) - 1 * WSIZE)
#define FTRP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)) - QSIZE)

/* Given block ptr bp, return the prev and next pointer themselves.
 */
#define NEXTV(bp) (GET(NEXTP(bp)))
#define PREVV(bp) (GET(PREVP(bp)))

/* Given block ptr bp, compute the block pointer to the predecessor 
 * and successor in the heaplist.
 */
#define PRED_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp)-4 * WSIZE)))
#define SUCC_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp)-3 * WSIZE)))

/* Given block ptr bp of a free block, compute the block pointer to 
 * the previous and next free block.
 */
#define PREV_HRDP(bp) (*((char *) (bp) - 1 * WSIZE))
#define NEXT_HDRP(bp) (*((char *) (bp) - 2 * WSIZE))

/**
 * Two list exists in our system. 
 * Heap list: a list consisting of all blocks. It contains both free and 
 * allocated blocks.
 * Free list: a list consisting purely free blocks. 
 * 
 * heap_listp is a block pointer to the first block in the heaplist.
 * free_listp is a block pointer to the first block in the freelist.
 */
static char* heap_listp = 0;  /* A block pointer pointing to the first block on the heap  */
static char* free_listp = 0;  /* A block pointer pointing to the first free block in the free list */

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
    if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void *) -1) return -1;

    PUT(heap_listp, 0);                             /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(QSIZE, 1));  /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), 0);               /* Prologue next */
    PUT(heap_listp + (3 * WSIZE), 0);               /* Prologue prev */
    PUT(heap_listp + (4 * WSIZE), PACK(QSIZE, 1));  /* Prologue footer */
    PUT(heap_listp + (5 * WSIZE), PACK(0, 1));      /* Epilogue header */

    free_listp = heap_listp + (5 * WSIZE);
    heap_listp += (4 * WSIZE);                      /* heaplist_p */

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    
    // checkheap(1);
    // printf("%d: mm_init \n",  __LINE__);
    return 0;
}

static void *extend_heap(size_t words) {
    char* nextp;  /* pointer to the second word (next pointer) of extended block */
    char* bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    
    if ((long)(nextp = mem_sbrk(size)) == -1) return NULL;
    // printf("size = %d \n", size);

    /* Initialize free block header/footer and the epilogue header */
    bp = nextp + DSIZE;
    PUT(HDRP(bp), PACK(size, 1));           /* header */
    PUT(NEXTP(bp), free_listp);    /* next pointer, points to original head of free list */
    PUT(PREVP(bp), 0);                      /* prev pointer */
    PUT(FTRP(bp), PACK(size, 1));           /* Free block footer */
    PUT(HDRP(SUCC_BLKP(bp)), PACK(0, 1));   /* New epilogue header */

    /* Coalesce if the previous block was free */
    // coalesce() cannot be called until header and footer are property updated.
    free_listp = coalesce(bp);
    return free_listp;
}

/**
 * When coalescing in expicit list scheme, should look forward/backward with
 * blocksize, instead of using prev, next pointer.
 * mm_free() would only setting the allocation bit to 0 in header and footer.
*/
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PRED_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(SUCC_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { /* Case 1 */
        // Both predcessor and successor are allocated.
        printf("coalesce(): case 1\n");
        return bp;
    } 

    // else if (prev_alloc && !next_alloc) { /* Case 2 */
    //     // Predecessor is allocated, successor is free
    //     printf("coalesce(): case 2\n");

    //     // 1. Adjust the size
    //     size += GET_SIZE(HDRP(SUCC_BLKP(bp)));
    //     PUT(HDRP(bp), PACK(size, 0));
    //     PUT(FTRP(bp), PACK(size, 0));

    //     // 2. Change pointers of blocks of predecessor and successor
    //     PUT(NEXTP(PREV_BLKP(bp)), NEXTV(bp));
    //     PUT(PREVP(NEXT_BLKP(bp)), PREVV(bp));

    //     // 3. Change pointers of the aggregated block
    //     PUT(NEXTP(bp), free_listp); /* next points to original head of free list */
    //     PUT(PREVP(bp), 0);          /* prev points to NULL */

    //     // 4. Update free list
    //     free_listp = bp;
    // }

    // else if (!prev_alloc && next_alloc) { /* Case 3 */
    //     // Successor is allocated, predecessor is free
    //     printf("coalesce(): case 3\n");

    //     // 1. Adjust the size
    //     size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    //     PUT(FTRP(bp), PACK(size, 0));
    //     PUT(HDRP(PRED_BLKP(bp)), PACK(size, 0));

    //     // 2. Change pointers of blocks of predecessor and successor
    //     bp = PRED_BLKP(bp);

    //     // Code below is not needed, unless you want strict FIFO.
    //     PUT(NEXTP(PREV_BLKP(bp)), NEXTV(bp));
    //     PUT(PREVP(NEXT_BLKP(bp)), PREVV(bp));

    //     // 3. Change pointers of the aggregated block
    //     PUT(NEXTP(bp), free_listp); /* next points to original head of free list */
    //     PUT(PREVP(bp), 0);          /* prev points to NULL */

    //     // 4. Update free list
    //     free_listp = bp;
    // }

    // else { /* Case 4 */
    //     // Both predecessor and successor are free
    //     printf("coalesce(): case 4\n");

    //     // 1. Adjust the size
    //     size += GET_SIZE(HDRP(PRED_BLKP(bp))) + GET_SIZE(FTRP(SUCC_BLKP(bp)));
    //     PUT(HDRP(PRED_BLKP(bp)), PACK(size, 0));
    //     PUT(FTRP(SUCC_BLKP(bp)), PACK(size, 0));

    //     // 2. Change pointers of blocks of predecessor and successor
    //     PUT(NEXTP(PREV_BLKP(PRED_BLKP(bp))), NEXTV(PRED_BLKP(bp)));
    //     PUT(PREVP(NEXT_BLKP(PRED_BLKP(bp))), PREVV(PRED_BLKP(bp)));

    //     PUT(NEXTP(PREV_BLKP(SUCC_BLKP(bp))), NEXTV(SUCC_BLKP(bp)));
    //     PUT(PREVP(NEXT_BLKP(SUCC_BLKP(bp))), PREVV(SUCC_BLKP(bp)));

    //     bp = PRED_BLKP(bp);

    //     // 3. Change pointers of the aggregated block
    //     PUT(NEXTP(bp), free_listp); /* next points to original head of free list */
    //     PUT(PREVP(bp), 0);          /* prev points to NULL */

    //     // 4. Update free list
    //     free_listp = bp;
    // }

    return bp;
}

/*
 * mm_free
 */
void mm_free(void *bp) {
    if (bp == 0)
        return;

    size_t size = GET_SIZE(HDRP(bp));
    if (heap_listp == 0) {
        mm_init();
    }

    // Update allocation bit in header and footer
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    // coalesce check
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
    size_t asize;       /* Adjusted block size */
    size_t extendsize;  /* Amount to extend heap if no fit */
    char*bp;           /* Block pointer */

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

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) return NULL;
    place(bp, asize);
    return bp;
}

/**
 * Current policy: always split the block if the remaining is not smaller
 * than the minimum block size.
 */
static void place(void *bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= MIN_BLOCKSIZE) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        // bp = NEXT_BLKP(bp);

        // Update header and footer
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));

        // Add to free list
        PUT(NEXTP(bp), free_listp);
        PUT(PREVP(bp), 0);
        free_listp = bp;
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

static void *find_fit(size_t asize) {
    void *bp;

    // for (bp = free_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    //     if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
    //         return bp;
    //     }
    // }

    return NULL; /* No fit */
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

    // checkheap(0);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: End Of List\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp,
           hsize, (halloc ? 'a' : 'f'),
           fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp) {
    if ((size_t) bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
        printf("Error: header does not match footer\n");
}

static void printfreelist() {
    char *bp;

    printf("Free (%p):\n", free_listp);

    for (bp = free_listp; GET_SIZE(bp) > 0; bp = NEXT_HDRP(bp)) {
        checkblock(bp);
        printblock(bp);
    }
}

/* 
 * checkheap - Minimal check of the heap for consistency 
 */
void checkheap(int verbose) {
    char* bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    if ((GET_SIZE(HDRP(heap_listp)) != QSIZE) || !GET_ALLOC(HDRP(heap_listp)))
        printf("Bad prologue header\n");
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = SUCC_BLKP(bp)) {
        if (verbose)
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");

    printfreelist();
    printf("\n");
}
