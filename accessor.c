#include "nvm_accessor.h"

#include "nvm_manager.h"
#include "defs.h"

#include <linux/io.h>
#include <linux/slab.h>

static NvmAccessor nvm_accessor = {
    .is_valid = false
};

/***********************private API***********************/

void* get_vaddr_by_paddr(NvmAccessor* this, phys_addr_t paddr){
    void* vaddr;
    if(!this){
        pr_err("Accessor pointer unvaild!\n");
        return NULL;
    }

    vaddr = this->nvm_addr_map_manager
            ->phy_addr_map_virt_addr_func(this->nvm_addr_map_manager, paddr);
    return vaddr;
}

void* get_vaddr_by_lba(NvmAccessor* this, u64 lba){
    phys_addr_t paddr;
    if(!this){
        pr_err("Accessor pointer unvaild!\n");
        return NULL;
    }

    paddr = this->nvm_addr_map_manager->nvm_phy_start_addr
            + lba * CACHE_BLOCK_SIZE;
    return get_vaddr_by_paddr(this, paddr);
}

// TODO：安全判断,返回值
size_t nvm_accessor_write(NvmAccessor* this, void* buffer, size_t count, void* virtAddr){
    if (!buffer) {
        pr_err("buffer pointer unvaild!\n");
        return EINVAL;
    }
    if(count < 0){
        pr_err("count unvaild!\n");
        return EINVAL;
    }
    if (!virtAddr) {
        pr_err("virtAddr pointer unvaild!\n");
        return EINVAL;
    }

    memcpy(virtAddr, buffer, count);
    wmb();
    clflush(virtAddr);
    return count;
};

size_t nvm_accessor_read(NvmAccessor* this, void *buffer, size_t count, void* virtAddr){
    if (!buffer) {
        pr_err("buffer pointer unvaild!\n");
        return EINVAL;
    }
    if(count < 0){
        pr_err("count unvaild!\n");
        return EINVAL;
    }
    if (!virtAddr) {
        pr_err("virtAddr pointer unvaild!\n");
        return EINVAL;
    }

    rmb();
    memcpy(buffer, virtAddr, count);
    return count;
};
// 多线程下内存屏障要配合原子变量来进行无锁同步
/***********************private API***********************/

/***********************public API***********************/

NvmAccessor* nvm_accessor_init(NvmManager* nvm_addr_map_manager){
    if (!nvm_addr_map_manager) {
        pr_err("nvm_addr_map_manager unvailed!");
        goto parameter_unvailed;
    }

    nvm_accessor.nvm_addr_map_manager = nvm_addr_map_manager;
    return &nvm_accessor;

parameter_unvailed:
    return NULL;
}

int accessor_destory(NvmAccessor* this){
    if(!this){
        pr_err("Accessor pointer unvaild!\n");
        return EINVAL;
    }
    return 0;
}

size_t nvm_accessor_write_block(NvmAccessor* this, void *buffer, NvmCacheBlkId blkId){
    void* virtAddr = get_vaddr_by_lba(this, blkId);
    return nvm_accessor_write(this, buffer, CACHE_BLOCK_SIZE, virtAddr);
}

size_t nvm_accessor_read_block(NvmAccessor* this, void *buffer, NvmCacheBlkId blkId){
    void* virtAddr = get_vaddr_by_lba(this, blkId);
    return nvm_accessor_read(this, buffer, CACHE_BLOCK_SIZE, virtAddr);
}

size_t nvm_accessor_write_byte(NvmAccessor* this, void *buffer, u64 offset){
    u64 paddr = offset + this->nvm_addr_map_manager->nvm_phy_start_addr;
    void* virtAddr = get_vaddr_by_paddr(this, paddr);                       // get_vaddr_by_paddr 改为 get_vaddr_by_offset（偏移量）
    return nvm_accessor_write(this, buffer, 1, virtAddr);
}

size_t nvm_accessor_read_byte(NvmAccessor* this, void *buffer, u64 offset){
    u64 paddr = offset + this->nvm_addr_map_manager->nvm_phy_start_addr;
    void* virtAddr = get_vaddr_by_paddr(this, paddr);
    return nvm_accessor_read(this, buffer, 1, virtAddr);
}

/***********************public API***********************/


