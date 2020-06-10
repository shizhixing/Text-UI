#include <stdlib.h>
#include <string.h>

int alloc_size;
int alloc_block_count;

typedef struct mem_head_t {
    int size;
} mem_head_t;

void *wmalloc(int size)
{
    mem_head_t *p;
    p = (mem_head_t *)malloc(sizeof(mem_head_t) + size);
    memset(p, 0x00, sizeof(mem_head_t) + size);
    if (p) {
        p->size = size;
        alloc_size += size;
        alloc_block_count++;
        p++;
        return (void *)p;
    }
    return 0;
}

void wfree(void *p)
{
    mem_head_t *h = p;
    h--;
    alloc_size -= h->size;
    alloc_block_count--;
    free(h);
}
