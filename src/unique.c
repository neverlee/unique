
#include "redismodule.h"
#include <ctype.h>
#include "unique.h"
#include "sds.h"
#include "adlist.h"
#include "dict.h"

static RedisModuleType *UniqueType;

static int UniquePushCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc, mergefn fn) {
    if (argc != 4) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    // open the key and make sure it is indeed a Hash and not empty
    RedisModuleKey *key =
        RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);

    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != UniqueType)
    {
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    unique *unique;
    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        unique = uniqueCreate();
        RedisModule_ModuleTypeSetValue(key,UniqueType,unique);
    } else {
        unique = RedisModule_ModuleTypeGetValue(key);
    }

    sds skey, sval;
    size_t skeylen, svallen;
    const char *pkey, *pval;

    pkey = RedisModule_StringPtrLen(argv[2], &skeylen);
    pval = RedisModule_StringPtrLen(argv[3], &svallen);

    skey = sdsnewlen(pkey, skeylen);
    sval = sdsnewlen(pval, svallen);

    int n = uniquePush(unique, skey, sval, fn);
    if (n == -1) {
        RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    } else {
        RedisModule_ReplyWithLongLong(ctx, n);
    }
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}


int UniquePushUPCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    return UniquePushCommand(ctx, argv, argc, retain_new);
}
int UniquePushNXCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    return UniquePushCommand(ctx, argv, argc, retain_old);
}

int UniquePopCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    // open the key and make sure it is indeed a Hash and not empty
    RedisModuleKey *key =
        RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);

    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    if (RedisModule_ModuleTypeGetType(key) != UniqueType) { 
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    unique *unique = RedisModule_ModuleTypeGetValue(key);
    
    sds skey, sval;
    int r = uniquePop(unique, &skey, &sval);
    if (r == 0) {
        RedisModule_ReplyWithArray(ctx, 2);
        RedisModule_ReplyWithStringBuffer(ctx, skey, sdslen(skey));
        RedisModule_ReplyWithStringBuffer(ctx, sval, sdslen(sval));
        sdsfree(skey);
        sdsfree(sval);
    } else {
        RedisModule_ReplyWithNull(ctx);
    }
    return REDISMODULE_OK;
}

int UniqueLenCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    // open the key and make sure it is indeed a Hash and not empty
    RedisModuleKey *key =
        RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);

    int type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        RedisModule_ReplyWithNull(ctx);
        return REDISMODULE_OK;
    }

    if (RedisModule_ModuleTypeGetType(key) != UniqueType) { 
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    unique *unique = RedisModule_ModuleTypeGetValue(key);
    
    size_t len = uniqueLen(unique);
    RedisModule_ReplyWithLongLong(ctx, len);
    return REDISMODULE_OK;
}




/* ========================== "uniquetype" type methods ======================= */

void *UniqueTypeRdbLoad(RedisModuleIO *rdb, int encver) {
    if (encver != 0) {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }

    uint64_t elements = RedisModule_LoadUnsigned(rdb);
    
    unique *unique = uniqueCreate();
    char *pkey, *pval;
    size_t lkey, lval;
    sds key, val;
    while(elements--) {
        pkey = RedisModule_LoadStringBuffer(rdb, &lkey);
        pval = RedisModule_LoadStringBuffer(rdb, &lval);
        key = sdsnewlen(pkey, lkey);
        val = sdsnewlen(pval, lval);
        RedisModule_Free(pkey);
        RedisModule_Free(pval);
        uniquePush(unique, key, val, retain_new);
    }
    return unique;
}

void UniqueTypeRdbSave(RedisModuleIO *rdb, void *value) {
    unique *unique = value;
    listIter *it = listGetIterator(unique->l, AL_START_HEAD);
    listNode *node;
    RedisModule_SaveUnsigned(rdb, listLength(unique->l));
    for (node = listNext(it); node; node = listNext(it)) {
        dictEntry *en = node->value;
        RedisModule_SaveStringBuffer(rdb, en->key, sdslen(en->key));
        RedisModule_SaveStringBuffer(rdb, en->v.val, sdslen(en->v.val));
    }
}

void UniqueTypeAofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    unique *unique = value;
    listIter *it = listGetIterator(unique->l, AL_START_HEAD);
    listNode *node;
    sds k, v;
    for (node = listNext(it); node; node = listNext(it)) {
        dictEntry *en = node->value;
        k = en->key;
        v = en->v.val;
        RedisModule_EmitAOF(aof,"unique.push","sbb",key,k, sdslen(k), v, sdslen(v));
    }

}

/* The goal of this function is to return the amount of memory used by
 * the UniqueType value. */
size_t UniqueTypeMemUsage(const void *value) {
    // const struct UniqueTypeObject *hto = value;
    // struct UniqueTypeNode *node = hto->head;
    // return sizeof(*hto) + sizeof(*node)*hto->len;
    return 0;
}


void UniqueTypeFree(void *value) {
    uniqueRelease(value);
}

void UniqueTypeDigest(RedisModuleDigest *md, void *value) {
    unique *unique = value;
    listIter *it = listGetIterator(unique->l, AL_START_HEAD);
    listNode *node;
    for (node = listNext(it); node; node = listNext(it)) {
        dictEntry *en = node->value;
        RedisModule_DigestAddStringBuffer(md, en->key, sdslen(en->key));
        RedisModule_DigestAddStringBuffer(md, en->v.val, sdslen(en->v.val));
    }
    RedisModule_DigestEndSequence(md);
}


/* This function must be present on each Redis module. It is used in order to
 * register the commands into the Redis server. */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx,"unique",1,REDISMODULE_APIVER_1)
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

    UniqueType = RedisModule_CreateDataType(ctx,"uniqueTyp",0,&tm);
    if (UniqueType == NULL) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "unique.pushup", UniquePushUPCommand,
                "write fast deny-oom", 1, 1,
                1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "unique.pushnx", UniquePushNXCommand,
                "write fast deny-oom", 1, 1,
                1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;


    if (RedisModule_CreateCommand(ctx, "unique.pop", UniquePopCommand,
                "write fast deny-oom", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "unique.len", UniqueLenCommand,
                "readonly fast", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
