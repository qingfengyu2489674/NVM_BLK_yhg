#ifndef NVM_CACHE_MAPPER
#define NVM_CACHE_MAPPER


#include "defs.h"

// 缓存的底层块设备扇区地址形式是逻辑块地址（LBA），NVM 上需要持久化当前缓存的 LBA 逻辑块地址和缓存块号的映射关系，NvmCacheMapper 管理 NVM 上这些数据和元数据。

typedef struct NvmCache NvmCache;
typedef struct NvmTransaction NvmTransaction;
typedef struct NvmCacheBlkPool NvmCacheBlkPool;
typedef struct NvmAccessor NvmAccessor;

#define NUM_MAPPER_SCAN_FUNCS 10
#define CACHE_BLOCK_START_INDEX 1
#define MAPPER_ELEMENT_SIZE 8 
#define CALCULATE_BLK_ID(element_num) \
    ((size_t)((element_num + CACHE_BLOCK_START_INDEX) * MAPPER_ELEMENT_SIZE + (CACHE_BLOCK_SIZE - 1)) / CACHE_BLOCK_SIZE)


typedef void (*mapper_scan_func)(NvmCacheBlkPool *, u64, u64);


// 简单实现：以扇区为单位管理缓存，可以确保IO不会访问到单个缓存块的部分范围，方便管理
typedef struct NvmCacheMapper 
{
    // TODO
    // 这种数据结构在NVM上通用，可以封装起来单独实现，这里变成更上层的包装
    u64 element_num;
    mapper_scan_func *func_array;
    size_t func_count; 

} NvmCacheMapper;

/***********************public API***********************/

NvmCacheMapper* cache_mapper_init(u64 nvm_phy_length);
void cache_mapper_destruct(NvmCacheMapper *self);

void register_mapper_scan_func(NvmCacheMapper *mapper, mapper_scan_func func);

int get_lba(NvmAccessor *accessor, LbaType *lba, NvmCacheBlkId id);
int set_lba(NvmAccessor *accessor, LbaType *lba, NvmCacheBlkId id);

int cache_mapper_scan(NvmCache *cache);

/***********************public API***********************/

#endif