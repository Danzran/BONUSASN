#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Define cache parameters
#define CACHE_SIZE 32
#define BLOCK_SIZE 4
#define NUM_BLOCKS (CACHE_SIZE / BLOCK_SIZE)
#define TAG_BITS 20
#define INDEX_BITS 5
#define OFFSET_BITS 2

// Define cache types
#define DIRECT_MAPPED 1
#define TWO_WAY 2
#define FOUR_WAY 4
#define FULLY_ASSOCIATIVE 0

// Set cache type here
#define CACHE_TYPE FOUR_WAY

typedef struct {
    bool valid;
    unsigned int tag;
    unsigned char data[BLOCK_SIZE];
    int access_time;
} cache_line_t;

cache_line_t cache[NUM_BLOCKS];

void init_cache() {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        cache[i].valid = false;
    }
}

int get_tag(unsigned int addr) {
    return addr >> (INDEX_BITS + OFFSET_BITS);
}

int get_index(unsigned int addr) {
    return (addr >> OFFSET_BITS) & ((1 << INDEX_BITS) - 1);
}

void cache_access(unsigned int addr, int* hits, int* misses) {
    int index = get_index(addr);
    int tag = get_tag(addr);
    int min_time = cache[index].access_time;
    int min_idx = index;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (cache[index].valid && cache[index].tag == tag) {
            cache[index].access_time = rand();
            *hits += 1;
            return;
        }
        if (!cache[index].valid) {
            min_idx = index;
            break;
        }
        if (cache[index].access_time < min_time) {
            min_time = cache[index].access_time;
            min_idx = index;
        }
    }
    *misses += 1;
    cache[min_idx].valid = true;
    cache[min_idx].tag = tag;
    cache[min_idx].access_time = rand();
    for (int i = 0; i < BLOCK_SIZE; i++) {
        cache[min_idx].data[i] = rand() & 0xff;
    }
}

int main() {
    init_cache();
    FILE* fp = fopen("traces.txt", "r");
    if (fp == NULL) {
        printf("Error: could not open input file.\n");
        return 1;
    }
    char line[100];
    int hits_direct_mapped = 0, misses_direct_mapped = 0;
    int hits_two_way = 0, misses_two_way = 0;
    int hits_four_way = 0, misses_four_way = 0;
    int hits_fully_associative = 0, misses_fully_associative = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        unsigned int addr = strtoul(line, NULL, 16);
        bool found = false;
        switch(CACHE_TYPE) {
            case DIRECT_MAPPED:
                cache_access(addr, &hits_direct_mapped, &misses_direct_mapped);
                break;
            case TWO_WAY:
                cache_access(addr, &hits_two_way, &misses_two_way);
                cache_access(addr ^ BLOCK_SIZE, &hits_two_way, &misses_two_way);
                break;
            case FOUR_WAY:
                cache_access(addr, &hits_four_way, &misses_four_way);
                cache_access(addr ^ BLOCK_SIZE, &hits_four_way, &misses_four_way);
                cache_access(addr ^ (BLOCK_SIZE * 2), &hits_four_way, &misses_four_way);
                cache_access(addr ^ (BLOCK_SIZE * 3), &hits_four_way, &misses_four_way);
                break;
            case FULLY_ASSOCIATIVE:
                
                for (int i = 0; i < NUM_BLOCKS; i++) {
                    if (cache[i].valid && cache[i].tag == get_tag(addr)) {
                        cache[i].access_time = rand();
                        hits_fully_associative += 1;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    misses_fully_associative += 1;
                    int min_time = cache[0].access_time;
                    int min_idx = 0;
                    for (int i = 1; i < NUM_BLOCKS; i++) {
                        if (cache[i].access_time < min_time) {
                            min_time = cache[i].access_time;
                            min_idx = i;
                        }
                    }
                    cache[min_idx].valid = true;
                    cache[min_idx].tag = get_tag(addr);
                    cache[min_idx].access_time = rand();
                    for (int i = 0; i < BLOCK_SIZE; i++) {
                        cache[min_idx].data[i] = rand() & 0xff;
                    }
                }
                break;
            default:
                printf("Error: invalid cache type.\n");
                return 1;
        }
    }
    printf("Direct Mapped:\nHits: %d\nMisses: %d\nHit rate: %f%%\n", hits_direct_mapped, misses_direct_mapped, (double)hits_direct_mapped / (hits_direct_mapped + misses_direct_mapped) * 100);
    printf("\nTwo-Way Set Associative:\nHits: %d\nMisses: %d\nHit rate: %f%%\n", hits_two_way, misses_two_way, (double)hits_two_way / (hits_two_way + misses_two_way) * 100);
    printf("\nFour-Way Set Associative:\nHits: %d\nMisses: %d\nHit rate: %f%%\n", hits_four_way, misses_four_way, (double)hits_four_way / (hits_four_way + misses_two_way) * 100);

}