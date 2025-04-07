#include "defs.h"
#include "core.h"
#include "access.h"
#include "lower_dev.h"
#include "mapper.h"
#include "NvmBlkPoolManager/blk_pool.h"

int nvm_cache_init(NvmCache *cache, NvmAccessor *accessor, NvmManager *accessor_manager, 
                   NvmCacheLowerDev *lower_bdev, NvmCacheMapper *mapper, NvmCacheBlkPool *blk_pool) 
{
    if (cache == NULL) 
    {
        DEBUG_PRINT("Error in nvm_cache_init: cache is NULL.\n");
        return -1;
    }

    if (accessor == NULL || accessor_manager == NULL || lower_bdev == NULL || mapper == NULL || blk_pool == NULL) 
    {
        DEBUG_PRINT("Error in nvm_cache_init: One or more required parameters are NULL.\n");
        return -1;
    }

    cache->accessor = accessor;
    cache->accessor_manager = accessor_manager;
    cache->lower_bdev = lower_bdev;
    cache->mapper = mapper;
    cache->blk_pool = blk_pool;

    DEBUG_PRINT("NvmCache initialized successfully.\n");
    return 0;  // 初始化成功
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

    // 销毁 accessor_manager 资源
    if (self->accessor_manager != NULL) 
    {
        DEBUG_PRINT("Destroying accessor_manager resources.\n");
        // nvm_accessor_manager_destruct(self->accessor_manager);  // 假设存在析构函数
        self->accessor_manager = NULL;
    } 
    else 
    {
        DEBUG_PRINT("Warning: accessor_manager is NULL, skipping destruction.\n");
    }

    // 最后将指针清空，确保不会出现悬空指针
    self = NULL;  // 或者可以不再使用该对象
    DEBUG_PRINT("NvmCache destructed successfully.\n");
}
