#include "defs.h"
#include "core.h"
#include "test_access.h"
#include "mapper.h"
#include "blk_pool.h"

NvmCache* nvm_cache_init(NvmAccessor *accessor, 
                         NvmCacheMapper *mapper, NvmCacheBlkPool *blk_pool) 
{
    // 动态分配 NvmCache 结构体内存
    NvmCache *cache = (NvmCache*)malloc(sizeof(NvmCache));
    
    // 检查内存分配是否成功
    if (cache == NULL) 
    {
        DEBUG_PRINT("Error in nvm_cache_init: Memory allocation for cache failed.\n");
        return NULL; // 返回 NULL 表示内存分配失败
    }

    // 检查输入参数
    if (accessor == NULL || mapper == NULL || blk_pool == NULL) 
    {
        DEBUG_PRINT("Error in nvm_cache_init: One or more required parameters are NULL.\n");
        free(cache);  // 如果参数为空，释放已分配的内存
        return NULL;  // 返回 NULL
    }

    // 初始化结构体成员
    cache->accessor = accessor;
    cache->mapper = mapper;
    cache->blk_pool = blk_pool;

    DEBUG_PRINT("NvmCache initialized successfully.\n");
    
    return cache;  // 返回指向分配的 NvmCache 结构体的指针
}



void nvm_cache_destruct(NvmCache *self) 
{
    if (self == NULL) 
    {
        DEBUG_PRINT("Error in nvm_cache_destruct: self is NULL.\n");
        return;
    }

    if (self->lower_bdev != NULL) 
    {
        DEBUG_PRINT("Destroying lower_bdev resources.\n");
        // 假设 `NvmCacheLowerDev` 需要释放某些资源
        // nvm_cache_lower_dev_destruct(self->lower_bdev); // 假设存在析构函数
        self->lower_bdev = NULL;
    } 
    else 
    {
        DEBUG_PRINT("Warning: lower_bdev is NULL, skipping destruction.\n");
    }

    // 销毁 NvmCacheMapper 资源
    if (self->mapper != NULL) 
    {
        DEBUG_PRINT("Destroying mapper resources.\n");
        // 假设 `NvmCacheMapper` 需要释放某些资源
        // nvm_cache_mapper_destruct(self->mapper); // 假设存在析构函数
        self->mapper = NULL;
    } 
    else 
    {
        DEBUG_PRINT("Warning: mapper is NULL, skipping destruction.\n");
    }

    // 销毁 NvmCacheBlkPool 资源
    if (self->blk_pool != NULL) 
    {
        DEBUG_PRINT("Destroying blk_pool resources.\n");
        // 假设 `NvmCacheBlkPool` 需要释放某些资源
        // nvm_cache_blk_pool_destruct(self->blk_pool); // 假设存在析构函数
        self->blk_pool = NULL;
    } 
    else 
    {
        DEBUG_PRINT("Warning: blk_pool is NULL, skipping destruction.\n");
    }

    // 销毁 accessor 资源（如果需要的话）
    if (self->accessor != NULL) 
    {
        DEBUG_PRINT("Destroying accessor resources.\n");
        // nvm_accessor_destruct(self->accessor);  // 假设存在析构函数
        self->accessor = NULL;
    } 
    else 
    {
        DEBUG_PRINT("Warning: accessor is NULL, skipping destruction.\n");
    }


    // 最后将指针清空，确保不会出现悬空指针
    self = NULL;  // 或者可以不再使用该对象
    DEBUG_PRINT("NvmCache destructed successfully.\n");
}
