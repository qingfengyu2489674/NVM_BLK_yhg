#ifndef NVM_CACHE_DEFS
#define NVM_CACHE_DEFS

#include <linux/types.h>

typedef u64 LbaType;
typedef u64 NvmCacheBlkId;
typedef u32 ResourceId;  // 对应块驱动层的硬件队列ID
typedef u64 TxnId;

// 以CACHE_BLOCK_SIZE为单位缓存，最好对齐到设备扇区大小，bio好处理
#define CACHE_BLOCK_SIZE 512

typedef enum NvmCacheIoDir {
    CACHE_READ,
    CACHE_WRITE,
} NvmCacheIoDir;

#endif


