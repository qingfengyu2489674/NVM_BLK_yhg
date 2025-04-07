#include "format.h"
#include "core.h"
#include "access.h"
#include "mapper.h"
#include "NvmBlkPoolManager/blk_pool.h"

int nvm_cache_format(NvmCache *cache) 
{
    NvmAccessor *accessor;
    NvmCacheMapper *mapper;

    u64 buffer;
    int count;
    int mapper_index;
    u64 element_num;

    // 获取 accessor，并检查是否为空
    accessor = cache->accessor;
    if (accessor == NULL) 
    {
        DEBUG_PRINT("Error in format stage: accessor is NULL.\n");
        return -1;
    }
    
    // 尝试读取第一个字节，检查缓存状态
    count = nvm_accessor_read_byte(accessor, &buffer, 0);
    if (count < sizeof(buffer)) 
    {
        DEBUG_PRINT("Error in format stage: Failed to read the buffer. Read count: %d, expected: %zu\n", count, sizeof(buffer));
        return -1;
    }

    // 检查魔数是否匹配
    if (buffer != NVM_CACHE_MAGIC_NUMBER) 
    {
        mapper = cache->mapper;
        
        // 检查 mapper 是否为空
        if (mapper == NULL) 
        {
            DEBUG_PRINT("Error in format stage: mapper is NULL.\n");
            return -1;
        }
        
        element_num = mapper->element_num;

        // 设置并写入魔数
        buffer = NVM_CACHE_MAGIC_NUMBER;
        for(mapper_index = 0; mapper_index < CACHE_BLOCK_START_INDEX; mapper_index++)
        {
            count = nvm_accessor_write_byte(accessor, &buffer, mapper_index * MAPPER_ELEMENT_SIZE);
            if (count < sizeof(buffer)) 
            {
                DEBUG_PRINT("Error in format stage: Failed to write magic number at index %d. Write count: %d, expected: %zu\n", mapper_index, count, sizeof(buffer));
                return -1;
            }
        }

        // 设置并写入格式化值
        buffer = NVM_CACHE_FORMATTED_VALUE;
        for(mapper_index = CACHE_BLOCK_START_INDEX; mapper_index < element_num + CACHE_BLOCK_START_INDEX; mapper_index++)
        {
            count = nvm_accessor_write_byte(accessor, &buffer, mapper_index * MAPPER_ELEMENT_SIZE);
            if (count < sizeof(buffer)) 
            {
                DEBUG_PRINT("Error in format stage: Failed to write formatted value at index %d. Write count: %d, expected: %zu\n", mapper_index, count, sizeof(buffer));
                return -1;
            }
        }
    }
    return 0;
}
