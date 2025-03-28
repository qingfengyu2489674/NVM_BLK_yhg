#ifndef NVM_CACHE_CORE
#define NVM_CACHE_CORE

//typedef struct NvmAccessor NvmAccessor;
typedef struct block_device block_device;
//typedef struct NvmCacheMapper NvmCacheMapper;
typedef struct NvmCacheBlkPool NvmCacheBlkPool;
//typedef struct NvmTransactionManager NvmTransactionManager;
typedef struct NvmCacheLowerDev NvmCacheLowerDev;

// 保存所有关键数据结构的引用
typedef struct NvmCache {
    //NvmAccessor *accessor;
    NvmCacheLowerDev *lower_bdev;
    //NvmCacheMapper *mapper;
    NvmCacheBlkPool *blk_pool;  // 此处范例为NVM上只有一个全局缓存池
    //NvmTransactionManager *txn_mgr;

    // TODO
} NvmCache;

/***********************public API***********************/

// 构造NvmCache对象，成功返回0，否则返回错误码。如果需要错误恢复（日志回滚），在此函数中进行
// 注意cache不拥有accessor的所有权，析构时不需要对accessor做任何操作
// cache对lower_bdev, mapper, blk_pool, txn_mgr都拥有所有权，析构时需要析构并释放这些数据结构
//int nvm_cache_init(NvmCache *cache, NvmAccessor *accessor);

// 析构NvmCache对象
//void nvm_cache_destruct(NvmCache *self);

/***********************public API***********************/

#endif