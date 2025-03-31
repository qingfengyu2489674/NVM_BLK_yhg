#ifndef NVM_MANAGER
#define NVM_MANAGER

#include <linux/types.h>

// #define PAGE_ALIGN_4K       (1 << 12)
// #define PAGE_ALIGN_MASK_4K  (~(PAGE_ALIGN_4K - 1))

/* //TODO 处理多段映射关系
    struct NvmAddrMapEntry{
        phys_addr_t nvm_phy_start_addr;
        unsigned long nvm_phy_length;
        void *nvm_virt_start_addr;
        unsigned long nvm_virt_length;
        void* (*phy_addr_map_virt_addr)(phys_addr_t);
    }
    struct NvmAddrMapManager{
        *NvmAddrMapEntry nvm_addr_map_list;
    }
*/

// 只映射一段物理内存
typedef struct NvmManager{
    bool is_valid;
    phys_addr_t nvm_phy_start_addr;
    unsigned long nvm_phy_length;
    void *nvm_virt_start_addr;
    unsigned long nvm_virt_length;
    void* (*phy_addr_map_virt_addr_func)(struct NvmManager*, phys_addr_t);
    void* nvm_cache_blk_pool;
}NvmManager;

/***********************public API***********************/

NvmManager* nvm_addr_map_manager_init(
        phys_addr_t nvm_phy_start_addr, unsigned long nvm_phy_length);

int nvm_addr_map_manager_destory(NvmManager* this);

// 提供给访问器accessor的接口
void* nvm_phy_addr_map_virt_addr(NvmManager* this, phys_addr_t phy_addr);

int resgister_nvm_cache_blk_pool(NvmManager* this, void* nvm_cache_blk_pool);

void* get_nvm_cache_blk_pool(NvmManager* this);

/***********************public API***********************/

#endif