#ifndef NVM_CACHE_DEFS
#define NVM_CACHE_DEFS

#include <linux/types.h>

typedef uint64_t  u64;
typedef uint32_t u32;

typedef u64 LbaType;
typedef u64 NvmCacheBlkId;
typedef u32 ResourceId;  // 对应块驱动层的硬件队列ID
typedef u64 TxnId;

// 以CACHE_BLOCK_SIZE为单位缓存，最好对齐到设备扇区大小，bio好处理
#define CACHE_BLOCK_SIZE 512

// 在用户空间或内核中选择调试输出方式
#ifdef __KERNEL__
    // 在内核中使用 pr_err 输出错误信息
    #define DEBUG_PRINT(fmt, ...) pr_err(fmt, ##__VA_ARGS__)
#else
    // 在用户空间中使用 fprintf 输出错误信息
    #include <stdio.h>
    #define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#endif

typedef enum NvmCacheIoDir {
    CACHE_READ,
    CACHE_WRITE,
} NvmCacheIoDir;

#endif


