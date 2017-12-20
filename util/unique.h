#ifndef __UNIQUE_H__
#define __UNIQUE_H__

typedef struct unique unique;

/* Prototypes */
unique *uniqueCreate(void);
void uniqueRelease(unique *unique);
int uniquePush(unique *unique, void *key, void *val);
size_t uniqueLen(unique *unique);

void *retain_old(void *old, void *new);
void *retain_new(void *old, void *new);
void *merge_int64(void *old, void *new);
void *merge_float64(void *old, void *new);
int uniquePop(unique *unique, void *pkey, void *pval);


/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1


#endif /* __UNIQUE_H__ */
