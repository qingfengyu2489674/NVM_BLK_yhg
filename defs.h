#ifndef NVM_CACHE_DEFS
#define NVM_CACHE_DEFS

#include <linux/types.h>

typedef u64_t u64;
typedef u32_t u32;

typedef u64 LbaType;
typedef u64 NvmCacheBlkId;
typedef u32 ResourceId;  // 对应块驱动层的硬件队列ID
typedef u64 TxnId;

// 以CACHE_BLOCK_SIZE为单位缓存，最好对齐到设备扇区大小，bio好处理
#define CACHE_BLOCK_SIZE 
#define MAPPER_ELEMENT_SIZE 8  // Element size of 8 bytes for each cache element

typedef enum NvmCacheIoDir {
    CACHE_READ,
    CACHE_WRITE,
} NvmCacheIoDir;

#endif


