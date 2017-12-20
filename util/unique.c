
#include <stdlib.h>
#include <string.h>
#include "adlist.h"
#include "dict.h"
#include "sds.h"
#include "zmalloc.h"
#include "unique.h"


uint64_t sdshashCallback(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

int sdscompareCallback(void *privdata, const void *key1, const void *key2) {
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void sdsfreeCallback(void *privdata, void *val) {
    DICT_NOTUSED(privdata);

    sdsfree(val);
}

dictType uniqueDictType = {
    sdshashCallback,
    NULL,
    NULL,
    sdscompareCallback,
    sdsfreeCallback,
    sdsfreeCallback
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

int retain_old(void *old, void *new, void **merge) {
    sdsfree(new);
    *merge = old;
    return 0;
}
int retain_new(void *old, void *new, void **merge) {
    sdsfree(old);
    *merge = new;
    return 0;
}
int merge_int64(void *old, void *new, void **merge) {
    sdsfree(new);
    *merge = old;
    return 0;
}
int merge_float64(void *old, void *new, void **merge) {
    sdsfree(new);
    *merge = old;
    return 0;
}

int uniquePush(unique *unique, void *key, void *val, mergefn fn) {
    dictEntry *en;
    dictEntry *entry, *existing;
    entry = dictAddRaw(unique->d,key,&existing);
    en = entry ? entry : existing;

    if (entry) {  // insert new
        en->v.val = val;
        listAddNodeTail(unique->l, en);
        return 1;
    } else {
        sdsfree(key);
        return fn(en->v.val, val, &(en->v.val));
    }
}

int uniquePop(unique *unique, void *pkey, void *pval) {
    listNode *node = listFirst(unique->l);
    if (node == NULL) {
        return -1;
    }
    listDelNode(unique->l, node);
    dictEntry *en = node->value;
    sds key, val;
    if (!en) {
        return -1;
    }
    key = sdsdup(en->key);
    val = en->v.val;
    en->v.val = NULL;
    *((sds*)pkey) = key;
    *((sds*)pval) = val;
    dictDelete(unique->d, en);
    return 0;
}

size_t uniqueLen(unique *unique) {
    return listLength(unique->l);
}



