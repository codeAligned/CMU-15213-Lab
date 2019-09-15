/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new() {
    queue_t *q = malloc(sizeof(queue_t));

    /* What if malloc returned NULL? */
    if (q) {
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
    }
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q) {
    /* How about freeing the list elements and the strings? */
    if (q) {
        list_ele_t *curr = q->head, *next = NULL;
        while (curr) {
            // Free string
            free(curr->value);

            // Keep track of next element
            next = curr->next;

            // Free current element
            free(curr);

            // Proceed to next
            curr = next;
        }

        /* Free queue structure */
        free(q);
    }
}

static int stringLength(char *s) {
    // Return the length of the C-string(char array), including null character
    int numOfValidChars = 0;
    while (s[numOfValidChars] != '\0') {
        ++numOfValidChars;
    }
    return numOfValidChars + 1;
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s) {
    if (!q) {  // q is NULL
        return false;
    }

    /* What should you do if the q is NULL? */
    list_ele_t *newh = malloc(sizeof(list_ele_t));
    if (!newh) {  // q is NULL or fail to allocate space
        return false;
    }

    /* Don't forget to allocate space for the string and copy it */
    // Allocate space
    char *copiedStringPtr = malloc(sizeof(char) * stringLength(s));

    /* What if either call to malloc returns NULL? */
    if (!copiedStringPtr) {
        free(newh);  // IMPORTANT!! OTHERWISE YOU FAIL TEST10 AND TEST11
        return false;
    }

    // Call strcpy
    strcpy(copiedStringPtr, s);
    newh->value = copiedStringPtr;

    newh->next = q->head;
    q->head = newh;

    // If q->tail is NULL, we are inserting into an empty queue.
    // The head should be the same as the head.
    // Otherwise, q->tail is unchanged.
    if (!q->tail) {
        q->tail = newh;
    }
    ++q->size;

    return true;
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s) {
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */

    // If q is NULL, return false
    if (!q) {
        return false;
    }

    // Try allocate space for list element and string
    list_ele_t *newtail = malloc(sizeof(list_ele_t));
    if (!newtail) {
        return false;
    }
    char *copiedStringPtr = malloc(sizeof(char) * stringLength(s));
    if (!copiedStringPtr) {
        free(newtail);  // IMPORTANT!! OTHERWISE YOU FAIL TEST10 AND TEST11
        return false;
    }

    // Copy string content
    strcpy(copiedStringPtr, s);

    // update newtail
    newtail->value = copiedStringPtr;
    newtail->next = NULL;

    if (q->tail) {
        // If q->tail is not NULL, the queue is not empty
        q->tail->next = newtail;
        q->tail = newtail;
    } else {
        // q->tail is NULL, the queue is empty
        q->head = newtail;
        q->tail = newtail;
    }

    ++q->size;

    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize) {
    /* You need to fix up this code. */

    // If q is NULL or q is empty(q->head is NULL)
    if (!q || !q->head) {
        return false;
    }

    list_ele_t *oldhead = q->head, *newhead = q->head->next;

    // If sp is not NULL, copy as much as possible into sp
    if (sp) {
        int i;
        for (i = 0; i < bufsize - 1 && *(oldhead->value + i) != '\0'; ++i) {
            *(sp + i) = *(oldhead->value + i);
        }
        *(sp + i) = '\0';
    }

    // free space used by oldhead
    free(oldhead->value);
    free(oldhead);

    // update q->head
    q->head = newhead;

    // update q->size
    --q->size;

    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q) {
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */

    if (!q || !q->head) {
        return 0;
    } else {
        return q->size;
    }
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q) {
    /* You need to write the code for this function */

    // Do real work only if q is not NULL or EMPTY
    if (q && q->head) {
        list_ele_t *oldhead = q->head, *oldtail = q->tail;

        // Start reversing
        list_ele_t *curr = q->head, *prev = NULL, *next = NULL;
        while (curr) {
            next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }

        q->head = oldtail;
        q->tail = oldhead;
    }
}