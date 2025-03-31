#ifndef NVM_CACHE_FORMAT
#define NVM_CACHE_FORMAT

// 提供 NVM 缓存的格式化接口，用于初始化 NVM 设备的元数据结构和检查现有缓存结构的完整性，确保缓存系统正确识别及使用底层存储。
typedef struct NvmAccessor NvmAccessor;
typedef struct NvmCache NvmCache;

#define NVM_CACHE_FORMATTED_VALUE 0xFFFFFFFFFFFFFFFF


/***********************public API***********************/

int nvm_cache_format(NvmCache *cache);

/***********************public API***********************/

#endif