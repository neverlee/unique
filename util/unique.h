#ifndef __UNIQUE_H__
#define __UNIQUE_H__

typedef unique unique;

// typedef struct listIter {
//     listNode *next;
//     int direction;
// } listIter;

/* Prototypes */
unique *uniqueCreate(void);
void uniqueRelease(unique *unique);
int uniquePush(unique *unique, void *key, void *val);

void *retain_old(void *old, void *new);
void *retain_new(void *old, void *new);
void *merge_int64(void *old, void *new);
void *merge_float64(void *old, void *new);
int uniquePop(unique *unique, void **key, void **val);


// unique *uniqueAddNodeHead(unique *unique, void *value);
// unique *uniqueAddNodeTail(unique *unique, void *value);
// unique *uniqueInsertNode(unique *unique, uniqueNode *old_node, void *value, int after);
// void uniqueDelNode(unique *unique, uniqueNode *node);
// uniqueIter *uniqueGetIterator(unique *unique, int direction);
// uniqueNode *uniqueNext(uniqueIter *iter);
// void uniqueReleaseIterator(uniqueIter *iter);
// unique *uniqueDup(unique *orig);
// uniqueNode *uniqueSearchKey(unique *unique, void *key);
// uniqueNode *uniqueIndex(unique *unique, long index);
// void uniqueRewind(unique *unique, uniqueIter *li);
// void uniqueRewindTail(unique *unique, uniqueIter *li);
// void uniqueRotate(unique *unique);
// void uniqueJoin(unique *l, unique *o);

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1


#endif /* __UNIQUE_H__ */
