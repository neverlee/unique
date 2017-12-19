
#include "redismodule.h"
#include <ctype.h>
#include "dict.h"

static RedisModuleType *UniqueType;

uint64_t hashCallback(const void *key) {
    return ((uint64_t)key);
    // return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

int compareCallback(void *privdata, const void *key1, const void *key2) {
    REDISMODULE_NOT_USED(privdata);
    return ((uint64_t)key1) == ((uint64_t)key2);
}

void keyFreeCallback(void *privdata, void *val) {
    REDISMODULE_NOT_USED(privdata);
    REDISMODULE_NOT_USED(val);
}


dictType uniqueDictType = {
    hashCallback, // hash function
    NULL,         // key dup
    NULL,         // val dup
    compareCallback, // key compare
    keyFreeCallback, // key free
    keyFreeCallback  // val free
};


int UniqueGetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    return REDISMODULE_OK;
}
int UniqueSetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    return REDISMODULE_OK;
}
int UniqueDelCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    return REDISMODULE_OK;
}


/* ========================== "uniquetype" type methods ======================= */

void *UniqueTypeRdbLoad(RedisModuleIO *rdb, int encver) {
    if (encver != 0) {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }
    // uint64_t elements = RedisModule_LoadUnsigned(rdb);
    // struct UniqueTypeObject *hto = createUniqueTypeObject();
    // while(elements--) {
    //     int64_t ele = RedisModule_LoadSigned(rdb);
    //     UniqueTypeInsert(hto,ele);
    // }
    // return hto;
    return NULL;
}

void UniqueTypeRdbSave(RedisModuleIO *rdb, void *value) {
    // struct UniqueTypeObject *hto = value;
    // struct UniqueTypeNode *node = hto->head;
    // RedisModule_SaveUnsigned(rdb,hto->len);
    // while(node) {
    //     RedisModule_SaveSigned(rdb,node->value);
    //     node = node->next;
    // }
}

void UniqueTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    // struct UniqueTypeObject *hto = value;
    // struct UniqueTypeNode *node = hto->head;
    // while(node) {
    //     RedisModule_EmitAOF(aof,"HELLOTYPE.INSERT","sl",key,node->value);
    //     node = node->next;
    // }
}

/* The goal of this function is to return the amount of memory used by
 * the UniqueType value. */
size_t UniqueTypeMemUsage(const void *value) {
    // const struct UniqueTypeObject *hto = value;
    // struct UniqueTypeNode *node = hto->head;
    // return sizeof(*hto) + sizeof(*node)*hto->len;
    return 1024;
}


void UniqueTypeFree(void *value) {
    dictRelease(value);
}

void UniqueTypeDigest(RedisModuleDigest *md, void *value) {
    // dict *d = (dict*)value
    // dictIterator *it = dictGetIterator(d);
    // dictIterator *dictGetSafeIterator(d);
    // dictEntry *dictNext(dictIterator *iter);
    // void dictReleaseIterator(dictIterator *iter);
    // RedisModule_DigestAddLongLong(md,node->value);
    RedisModule_DigestEndSequence(md);
}


/* This function must be present on each Redis module. It is used in order to
 * register the commands into the Redis server. */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx,"Unique",1,REDISMODULE_APIVER_1)
            == REDISMODULE_ERR) return REDISMODULE_ERR;

    RedisModuleTypeMethods tm = {
        .version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = UniqueTypeRdbLoad,
        .rdb_save = UniqueTypeRdbSave,
        .aof_rewrite = UniqueTypeAofRewrite,
        .mem_usage = UniqueTypeMemUsage,
        .free = UniqueTypeFree,
        .digest = UniqueTypeDigest
    };
    
    UniqueType = RedisModule_CreateDataType(ctx,"uniquetype",0,&tm);
    if (UniqueType == NULL) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "unique.set", UniqueSetCommand,
                "write fast deny-oom", 1, 1,
                1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "unique.del", UniqueDelCommand,
                "write fast deny-oom", 1, 1,
                1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "unique.get", UniqueGetCommand,
                "readonly fast", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
