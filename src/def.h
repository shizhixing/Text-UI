#ifndef DEF_H
#define DEF_H
extern int alloc_size;
extern int alloc_block_count;
extern void *wmalloc(int size);
extern void wfree(void *p);
#define MALLOC(x) wmalloc(x)
#define FREE(x) wfree(x)

#define OK 0
#define NO -1
#define TRUE 1
#define FALSE 0
#endif
