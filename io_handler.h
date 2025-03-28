#ifndef NVM_CACHE_IO_HANDLER
#define NVM_CACHE_IO_HANDLER

#include "core.h"
#include "defs.h"

typedef struct NvmCache NvmCache;

typedef struct NvmCacheIoReq 
{
    LbaType lba;
    uint64_t lba_num;
    void *buffer;
    NvmCacheIoDir dir;

    // 收到IO请求的队列的下标，范围[0, hw_queue_count)
    // 如果设计为资源换并发，可用于获取每队列独占资源
    ResourceId resource_id;
} NvmCacheIoReq;

/***********************public API***********************/

// I/O请求处理总入口
int nvm_cache_handle_io(NvmCache *cache, NvmCacheIoReq *req);

/***********************public API***********************/

#endif