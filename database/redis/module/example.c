#include "utils.h"

/* EXAMPLE.PARSE [SUM <x> <y>] | [PROD <x> <y>]
*  Demonstrates the automatic arg parsing utility.
*  If the command receives "SUM <x> <y>" it returns their sum
*  If it receives "PROD <x> <y>" it returns their product
*/
int ParseCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) 
{
    if(argc < 4) return RedisModule_WrongArity(ctx); 

    // init auto memory for created strings
    RedisModule_AutoMemory(ctx);
    long long x, y;

    // If we got SUM - return the sum of 2 consecutive arguments
    if (parseArgsAfter("SUM", argv, argc, "ll", &x, &y) == REDISMODULE_OK) 
    {
        RedisModule_ReplyWithLongLong(ctx, x + y);
        return REDISMODULE_OK;
    }

    // If we got PROD - return the product of 2 consecutive arguments
    if (parseArgsAfter("PROD", argv, argc, "ll", &x, &y) == REDISMODULE_OK) 
    {
        RedisModule_ReplyWithLongLong(ctx, x * y);
        return REDISMODULE_OK;
    }

    RedisModule_ReplyWithError(ctx, "Invalid arguments"); //Neither sum nor prod returned
    return REDISMODULE_ERR;
}

extern int RedisModule_OnLoad(RedisModuleCtx *ctx) 
{
    if(RedisModule_Init(ctx, "example", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) 
    {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "example.parse", ParseCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR) 
    {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

