#ifndef __UNIQUE_H__
#define __UNIQUE_H__

#include "adlist.h"
#include "dict.h"

typedef struct unique{
    dict *d;
    list *l;
    size_t mem;
}unique;


typedef int mergefn(void *old, void *new, void **merge);

/* Prototypes */
unique *uniqueCreate(void);
void uniqueRelease(unique *unique);
int uniquePush(unique *unique, void *key, void *val, mergefn fn);
size_t uniqueLen(unique *unique);

int retain_old(void *old, void *new, void **merge);
int retain_new(void *old, void *new, void **merge);
int merge_int64(void *old, void *new, void **merge);
int merge_float64(void *old, void *new, void **merge);

int uniquePop(unique *unique, void *pkey, void *pval);


/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1


#endif /* __UNIQUE_H__ */
