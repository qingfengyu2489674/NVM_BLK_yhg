#include "format.h"
#include "core.h"
#include "accessor.h"
#include "mapper.h"
#include "NvmBlkPoolManager/blk_pool.h"


int nvm_cache_format(NvmCache *cache) 
{
    NvmAccessor *accessor;
    NvmCacheMapper *mapper;

    u64 buffer;
    int count;
    int mapper_index;
    u64 block_num;

    accessor = cache->accessor;
    count = nvm_accessor_read_byte(accessor, &buffer, 0);

    if (count < sizeof(buffer)) 
    {
        return -1;
    }

    if (buffer != NVM_CACHE_FORMATTED_VALUE) 
    {
        mapper = cache->mapper;
        block_num = mapper->block_num;

        buffer = NVM_CACHE_FORMATTED_VALUE;

        for(mapper_index = 0; mapper_index < block_num + CACHE_BLOCK_START_INDEX; mapper_index++)
        {
            count = nvm_accessor_write_byte(accessor, &buffer, mapper_index * MAPPER_ELEMENT_SIZE);
            if (count < sizeof(buffer)) 
            {
                return -1;
            }
        }
    }
    return 0;
}
