
#include "redismodule.h"
#include <ctype.h>
#include "unique.h"
#include "sds.h"

static RedisModuleType *UniqueType;

int UniquePushCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
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

    uniquePush(unique, skey, sval);

    RedisModule_ReplyWithSimpleString(ctx, "OK");

    return REDISMODULE_OK;
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
    return 0;
}


void UniqueTypeFree(void *value) {
    uniqueRelease(value);
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

    if (RedisModule_CreateCommand(ctx, "unique.push", UniquePushCommand,
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
