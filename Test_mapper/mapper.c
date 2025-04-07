#include "mapper.h"
#include "test_access.h"
#include "blk_pool.h"
#include "core.h"


int cache_mapper_init(NvmCacheMapper *mapper, u64 nvm_phy_length) 
{
    if (!mapper) 
    {
        return -1;
    }

    u64 element_size = CACHE_BLOCK_SIZE + MAPPER_ELEMENT_SIZE;
    u64 reserved_length = CACHE_BLOCK_START_INDEX * MAPPER_ELEMENT_SIZE;
    u64 element_num = (nvm_phy_length - reserved_length) / element_size;

    mapper->element_num = element_num;

    mapper->func_array = (mapper_scan_func*)malloc(NUM_MAPPER_SCAN_FUNCS * sizeof(mapper_scan_func));
    mapper->func_count = 0;
    
    return 0;
}

void register_mapper_scan_func(NvmCacheMapper *mapper, mapper_scan_func func) 
{
    if (mapper->func_count < NUM_MAPPER_SCAN_FUNCS) 
    {
        mapper->func_array[mapper->func_count] = func;
        mapper->func_count++;
    } 
    else 
    {
        return;
        // TODO:错误处理,函数数组已满
    }
}

void cache_mapper_destruct(NvmCacheMapper *self) 
{
    if (!self) 
    {
        return;
    }

    if (self->func_array) 
    {
        free(self->func_array);
        self->func_array = NULL;
    }
    
    self->element_num = 0;
    self->func_count = 0;
}

int get_lba(NvmAccessor *accessor, LbaType *lba, NvmCacheBlkId id) 
{
    u64 buffer;
    u64 offset = (id + CACHE_BLOCK_START_INDEX) * MAPPER_ELEMENT_SIZE;

    int count;

    count = nvm_accessor_read_byte(accessor, &buffer, offset);
    if (count < sizeof(buffer)) 
    {
        return -1;
    }

    *lba = buffer;

    return 0;
}

int set_lba(NvmAccessor *accessor, LbaType *lba, NvmCacheBlkId id) 
{
    u64 buffer;
    u64 offset = (id + 1) * MAPPER_ELEMENT_SIZE;

    int count;
    count = nvm_accessor_write_byte(accessor, lba, offset);
    if (count < sizeof(buffer)) 
    {
        return -1;
    }
    return 0;
}

int cache_mapper_scan(NvmCache *cache)
{
    NvmAccessor *accessor;
    NvmCacheMapper *mapper;
    NvmCacheBlkPool *blk_pool; 

    NvmCacheBlkId blk_index;
    LbaType lba;
    size_t func_index;

    int ret;
    mapper = cache->mapper;
    accessor = cache->accessor;
    blk_pool = cache->blk_pool;

    register_mapper_scan_func(mapper, build_nvm_empty_block);
    register_mapper_scan_func(mapper, build_nvm_valid_block);

    for(blk_index = 0; blk_index < mapper->element_num; blk_index++)
    {
        ret = get_lba(accessor, &lba, blk_index);
        if(ret)
        {
            return -1;
        }

        for (size_t func_index = 0; func_index < mapper->func_count; func_index++) 
        {
            mapper_scan_func current_func = mapper->func_array[func_index];
            current_func(blk_pool, blk_index, lba);
        }
    }

    return 0;
}