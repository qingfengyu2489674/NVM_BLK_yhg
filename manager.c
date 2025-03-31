#include "nvm_manager.h"

#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/io.h>

static NvmManager nvm_manager = {
    .is_valid = false
};

/***********************private API***********************/

// 根据偏移量反映射
void* phy_addr_map_virt_addr(NvmManager *this, phys_addr_t phy_addr){
    long offset_between_paddr_and_vaddr = (unsigned long)this->nvm_virt_start_addr - this->nvm_phy_start_addr;
    return (void*) (phy_addr += offset_between_paddr_and_vaddr);
}

/***********************private API***********************/

/***********************public API***********************/


// ! 不能对物理地址进行裁剪
// 建立映射
NvmManager* nvm_addr_map_manager_init(
        phys_addr_t nvm_phy_start_addr, unsigned long nvm_phy_length){
    void *nvm_virt_start_addr;

    // 仅允许映射一次地址
    if(nvm_manager.is_valid == true)
        return &nvm_manager;

    // 声明独立使用 //? 映射方法是否需要抽象出来？
    if (!request_mem_region(nvm_phy_start_addr, nvm_phy_length, "NVM")) {
      pr_err("request_mem_region fail!\n");
      goto request_mem_region_fail;
    }

    // TODO：是否映射为非缓存内存
    nvm_virt_start_addr =
        memremap(nvm_phy_start_addr, nvm_phy_length, MEMREMAP_WB);
    if (!nvm_virt_start_addr) {
        pr_err("memremap fail!\n");
        goto memremap_fail;
    }

    nvm_manager.nvm_phy_start_addr             = nvm_phy_start_addr;
    nvm_manager.nvm_phy_length                 = nvm_phy_length;
    nvm_manager.nvm_virt_start_addr            = nvm_virt_start_addr;
    nvm_manager.nvm_virt_length                = nvm_phy_length;
    nvm_manager.phy_addr_map_virt_addr_func    = phy_addr_map_virt_addr;
    nvm_manager.is_valid                       = true;

    return &nvm_manager;

memremap_fail:
    release_region(nvm_phy_start_addr, nvm_phy_length);
request_mem_region_fail:
    return NULL;
}

// 取消映射
int nvm_addr_map_manager_destory(NvmManager* this){
    if (this->is_valid != true) {
        pr_err("nvm_manager is unvaild!\n");
        return EINVAL;
    }
    this->is_valid = false;
    memunmap(this->nvm_virt_start_addr);
    release_region(this->nvm_phy_start_addr, this->nvm_phy_length);
    return 0;
}

// 查找映射
void* nvm_phy_addr_map_virt_addr(NvmManager *this, phys_addr_t phy_addr){
    void* nvm_virt_addr;

    if (this == NULL || this->is_valid != true){
        return NULL;
    }

    if(this->nvm_phy_start_addr > phy_addr ||
       this->nvm_phy_start_addr + this->nvm_phy_length < phy_addr){
        pr_info("nvm_phy_addr out of the mapped phyaddr!\n");
        return NULL;
    }
    
    nvm_virt_addr = this->phy_addr_map_virt_addr_func(this, phy_addr);
    if(!nvm_virt_addr){
        pr_info("phy_addr_map_virt_addr fail!\n");
        return NULL;
    }

    return nvm_virt_addr;
}

int resgister_nvm_cache_blk_pool(NvmManager* this, void* nvm_cache_blk_pool){
    if (this == NULL || this->is_valid != true){
        return EINVAL;
    }
    this->nvm_cache_blk_pool = nvm_cache_blk_pool;
    return 0;
}

void* get_nvm_cache_blk_pool(NvmManager* this){
        if (this == NULL || this->is_valid != true){
        return NULL;
    }
    return this->nvm_cache_blk_pool;
}
/***********************public API***********************/

/***********************abort APT************************/


/***********************abort APT************************/