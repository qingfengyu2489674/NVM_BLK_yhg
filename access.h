#ifndef NVM_ACCESSOR
#define NVM_ACCESSOR

#include "defs.h"
#include "manager.h"
#include <linux/types.h>

typedef struct NvmAccessor{
    NvmManager* nvm_addr_map_manager;
    bool is_valid;
    // TODO 内存保护：需要访问的时候建立地址映射
}NvmAccessor;

/***********************public API***********************/

NvmAccessor* nvm_accessor_init(NvmManager* nvm_addr_map_manager);

int nvm_accessor_destory(NvmAccessor* this);

size_t nvm_accessor_write_block(NvmAccessor* this, void *buffer, NvmCacheBlkId blkId);

size_t nvm_accessor_read_block(NvmAccessor* this, void *buffer, NvmCacheBlkId blkId);

size_t nvm_accessor_write_byte(NvmAccessor* this, void *buffer, u64 offset);

size_t nvm_accessor_read_byte(NvmAccessor* this, void *buffer, u64 offset);


/***********************public API***********************/
// size_t nvm_accessor_write(NvmAccessor* this, void *buffer, size_t count, u64 offset);

// size_t nvm_accessor_read(NvmAccessor* this, void *buffer, size_t count, u64 offset);

// void* get_vaddr_by_paddr(NvmAccessor* this, phys_addr_t paddr);

// 一个lab对应一个扇区，一个扇区假定是512字节
// void* get_vaddr_by_lba(NvmAccessor* this, u64 lba);


#endif