#include <math.h>  // For pow(x,y)
#include <stdio.h>
#include <stdlib.h>  // For atoi()
#include "cachelab.h"
#include "getopt.h"  // For getopt()

void parse(int *s_ptr, int *S_ptr, int *E_ptr, int *b_ptr, int *B_ptr,
           char **trace_file_ptr, int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's':
                *s_ptr = atoi(optarg);
                *S_ptr = pow(2, *s_ptr);
                break;
            case 'E':
                *E_ptr = atoi(optarg);
                break;
            case 'b':
                *b_ptr = atoi(optarg);
                *B_ptr = pow(2, *b_ptr);
                break;
            case 't':
                *trace_file_ptr = optarg;
                break;
            default:
                printf("Error: Unrecognized program option!");
                exit(1);
        }
    }

    if (*S_ptr <= 0 || *E_ptr <= 0 || *B_ptr <= 0 || trace_file_ptr == NULL) {
        printf("Error: Invalid program option");
        exit(1);
    }
}

typedef struct cache_line {
    // Only valid bit and set index is required. The block offset is not used.
    int valid_bit;
    long long tag;  // long long used for 64-bit address
    int access_time;
} cache_line_t;

typedef cache_line_t *cache_line_ptr;

int if_cache_hit(int set, int E, cache_line_ptr *cache,
                 int tag_bits, int *hit_ptr, int time) {
    int line;

    cache_line_ptr *set_start = cache + E * set;
    cache_line_ptr current_cl_ptr;
    for (line = 0; line < E; ++line) {
        current_cl_ptr = *(set_start + line);
        if (current_cl_ptr->valid_bit && current_cl_ptr->tag == tag_bits) {
            ++(*hit_ptr);

            current_cl_ptr->access_time = time;

            printf("hit ");
            return 1;
        }
    }
    return 0;
}

int if_empty_line(int set, int E, cache_line_ptr *cache,
                  int tag_bits, int *miss_ptr, int time) {
    int line;
    cache_line_ptr *set_start = cache + E * set, current_cl_ptr;

    for (line = 0; line < E; ++line) {
        current_cl_ptr = (*(set_start + line));
        if (current_cl_ptr->valid_bit == 0) {
            ++(*miss_ptr);

            current_cl_ptr->valid_bit = 1;
            current_cl_ptr->tag = tag_bits;
            current_cl_ptr->access_time = time;

            printf("miss ");
            return 1;
        }
    }

    return 0;
}

int LRU_line_idx(cache_line_ptr *set_start, int E, int time) {
    // Given a pointer to the cache_line_ptr of the 1st line in the set,
    // return the index of the LRU line.
    int min_time, idx, line;
    cache_line_ptr current_cl_ptr;

    min_time = time;
    for (line = 0; line < E; ++line) {
        current_cl_ptr = *(set_start + line);
        if (current_cl_ptr->access_time < min_time) {
            min_time = current_cl_ptr->access_time;
            idx = line;
        }
    }

    return idx;
}

void replace(int set, int E, cache_line_ptr *cache,
             int tag_bits, int *miss_ptr, int *evict_ptr, int time) {
    int replaced_line_idx;
    cache_line_ptr *set_start = cache + E * set;
    cache_line_ptr replaced_cl_ptr;

    replaced_line_idx = LRU_line_idx(set_start, E, time);
    replaced_cl_ptr = *(set_start + replaced_line_idx);

    replaced_cl_ptr->valid_bit = 1;
    replaced_cl_ptr->tag = tag_bits;
    replaced_cl_ptr->access_time = time;
    ++(*miss_ptr);
    ++(*evict_ptr);
    printf("miss evict ");
}

int load_to_cache(int set, int E, cache_line_ptr *cache, int tag_bits,
                  int *hit_ptr, int *miss_ptr, int *evict_ptr, int time) {
    // Check for cache hit
    if (if_cache_hit(set, E, cache, tag_bits, hit_ptr, time)) {
        return 0;
    }

    // No cache hit, check empty line
    if (if_empty_line(set, E, cache, tag_bits, miss_ptr, time)) {
        return 1;
    }

    // No empyt line, perform replacement
    replace(set, E, cache, tag_bits, miss_ptr, evict_ptr, time);
    return 2;
}

cache_line_ptr *init_cache(int S, int E) {
    int set, line;
    cache_line_ptr *cache;

    cache = malloc(sizeof(cache_line_ptr) * S * E);
    cache_line_ptr current_cl_ptr;
    for (set = 0; set < S; ++set) {
        for (line = 0; line < E; ++line) {
            *(cache + E * set + line) = malloc(sizeof(cache_line_t));
            current_cl_ptr = *(cache + E * set + line);
            current_cl_ptr->valid_bit = 0;
        }
    }
    return cache;
}

FILE * openFile(char *path, char* mode) {
    FILE * file = fopen(path, mode);
    if (!file) {
        printf("Error: Cannot open file %s", path);
        exit(1);
    }

    return file;
}

int main(int argc, char *argv[]) {
    int s, S, E, b, B;
    char *file_path;

    int set;
    cache_line_ptr *cache;  // A pointer to a cache_line_ptr

    FILE *file;
    char *operationString;
    char operation;
    long long address;  // long long used for 64-bit address
    int size;
    long long unsigned set_mask, set_bits, tag_bits;
    int hit, miss, evict;
    int time;

    // 1. Parse command line options
    s = S = E = b = B = 0;
    file_path = NULL;

    parse(&s, &S, &E, &b, &B, &file_path, argc, argv);
    printf("s = %d, E = %d, b = %d, file = \"%s\"\n", s, E, b, file_path);

    // 2. Construct cache
    cache = init_cache(S, E);

    // 3. Read from trace file and simulate
    file = openFile(file_path, "r");

    operationString = malloc(sizeof(char) * 2);
    set_mask = ~((-1) << s);
    hit = miss = evict = 0;
    time = 0;
    while (fscanf(file, "%s %llx,%d\n", operationString, &address, &size) != EOF) {
        ++time;
        operation = *operationString;
        set_bits = set_mask & (address >> b);
        tag_bits = address >> (b + s);  // Remove set index bits.
        // printf("\n%c %llx,%d, tag: 0x%08llx, set: 0x%08llx, ", operation, address, size, tag_bits, set_bits);

        set = set_bits;
        if (operation == 'L') {
            load_to_cache(set, E, cache, tag_bits, &hit, &miss, &evict, time);
        } else if (operation == 'S') {
            load_to_cache(set, E, cache, tag_bits, &hit, &miss, &evict, time);
        } else if (operation == 'M') {
            load_to_cache(set, E, cache, tag_bits, &hit, &miss, &evict, time);

            // The writing operation would always head for "M"
            ++hit;
            printf("hit ");
        }
    }

    printSummary(hit, miss, evict);
    return 0;
}
