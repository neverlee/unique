
#include <stdlib.h>
#include <string.h>
#include "adlist.h"
#include "dict.h"
#include "sds.h"
#include "zmalloc.h"


typedef struct unique{
    dict *d;
    list *l;
    size_t mem;
}unique;

uint64_t hashCallback(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

int compareCallback(void *privdata, const void *key1, const void *key2) {
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void freeCallback(void *privdata, void *val) {
    DICT_NOTUSED(privdata);

    sdsfree(val);
}

dictType uniqueDictType = {
    hashCallback,
    NULL,
    NULL,
    compareCallback,
    freeCallback,
    freeCallback
};

/* Create a new unique. The created unique can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new unique. */
unique *uniqueCreate(void)
{
    unique *unique = zmalloc(sizeof(*unique));
    if (NULL == unique)
        return NULL;

    unique->d = dictCreate(&uniqueDictType, NULL);
    unique->l = listCreate();
    const size_t init_size = sizeof(struct unique) + sizeof(dict) + sizeof(list);
    unique->mem = init_size;

    return unique;
}

/* Free the whole unique.
 *
 * This function can't fail. */
void uniqueRelease(unique *unique)
{
    listRelease(unique->l);
    dictRelease(unique->d);
    unique->mem = 0;
    zfree(unique);
}

void *retain_old(void *old, void *new) {
    sdsfree(new);
    return old;
}
void *retain_new(void *old, void *new) {
    sdsfree(old);
    return new;
}
void *merge_int64(void *old, void *new) {
    sdsfree(old);
    return new;
}
void *merge_float64(void *old, void *new) {
    sdsfree(old);
    return new;
}

int uniquePush(unique *unique, void *key, void *val) {
    dictEntry *en = dictAddOrFind(unique->d, key);
    if (en->v.val == NULL) {  // insert new
        en->v.val = val;
        unique->l = listAddNodeTail(unique->l, en);
        return 1;
    } else {
        sdsfree(key);
        en->v.val = retain_old(en->v.val, val);
    }
    return 0;
}

int uniquePop(unique *unique, void **key, void **val) {
    listNode *node = listFirst(unique->l);
    if (node == NULL) {
        return -1;
    }
    dictEntry *en = node->value;
    if (en == NULL) {
        return -1;
    }
    listDelNode(unique->l, node);
    *key = en->key;
    *val = en->v.val;
    en->key = en->v.val = NULL;
    dictDelete(unique->d, en->key);
    return 0;
}

