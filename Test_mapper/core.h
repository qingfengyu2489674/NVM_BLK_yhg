#ifndef NVM_CACHE_CORE
#define NVM_CACHE_CORE

// 定义指向各个数据结构的类型别名
typedef struct NvmAccessor NvmAccessor;
typedef struct NvmCacheMapper NvmCacheMapper;
typedef struct NvmCacheBlkPool NvmCacheBlkPool;
typedef struct NvmManager NvmManager;
typedef struct NvmCacheLowerDev NvmCacheLowerDev;

// NVM 缓存对象的定义
// 注意：此结构体定义了缓存系统中的核心对象及其依赖关系
typedef struct NvmCache {
    NvmAccessor *accessor;  // NVM 存储的访问器，提供对存储的访问
    // NvmManager *accessor_manager;  // 缓存管理器，负责管理缓存
    NvmCacheLowerDev *lower_bdev;  // NVM 存储的底层设备（通常是块设备）
    NvmCacheMapper *mapper;  // 用于管理缓存数据块映射关系
    NvmCacheBlkPool *blk_pool;  // 缓存块池，管理缓存中的数据块
    // TODO: 其他成员，未来根据需求扩展
} NvmCache;

/*********************** Public API ***********************/

// 初始化 NvmCache 对象，返回 0 表示成功，其他返回值表示错误。
// 注意：cache 不拥有 accessor 的所有权，析构时不需要处理 accessor。
// cache 对 lower_bdev, mapper, blk_pool 等有所有权，析构时需要释放它们。
NvmCache* nvm_cache_init(NvmAccessor *accessor,
            NvmCacheMapper *mapper, NvmCacheBlkPool *blk_pool);

// 析构 NvmCache 对象，释放所有相关资源
void nvm_cache_destruct(NvmCache *self);

#endif // NVM_CACHE_CORE_H
