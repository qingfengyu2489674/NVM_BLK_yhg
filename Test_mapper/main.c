#include <stdio.h>
#include "mapper.h"
#include "test_access.h"
#include "blk_pool.h"
#include "core.h"


int main() {
    // 创建一个虚拟的 NvmCacheMapper
    NvmCacheMapper *mapper = NULL;

    // 模拟物理内存长度，单位为字节
    u64 nvm_phy_length = 1024 * 1024;  // 1GB

    // 初始化 cache_mapper
    mapper = cache_mapper_init(nvm_phy_length);

    printf("Cache Mapper Initialized with %llu elements\n", mapper->element_num);

    // 创建虚拟的 NvmCache 对象
    NvmCache *cache;

    
    // 创建虚拟的 NvmAccessor 和 NvmCacheBlkPool
    NvmAccessor *accessor;
    NvmCacheBlkPool *blk_pool;

    accessor = nvm_accessor_init();

    init_nvm_blk_pool(blk_pool, 10);

    cache = nvm_cache_init(accessor,  mapper, blk_pool);
    

    int result = cache_mapper_scan(cache);
    if (result != 0) {
        printf("Cache mapper scan failed\n");
        return -1;
    }

    printf("Cache mapper scan completed successfully\n");

    // 清理
    cache_mapper_destruct(mapper);
    return 0;
}
