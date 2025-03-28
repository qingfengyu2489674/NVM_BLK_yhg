#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "blk_pool.h"
#include "EmptyBlkManager/queue.h"
#include "UsedBlkManager/hashtable.h"

// 初始化 NvmCacheBlkPool
void init_nvm_blk_pool(NvmCacheBlkPool *manager, size_t hash_table_size) 
{
    init_queue(&manager->empty_blocks_queue);
    manager->used_blocks_table = create_hashtable(hash_table_size);
}

// 构建 NVM 块
void build_nvm_block(NvmCacheBlkPool *manager, uint64_t nvm_blk_id, uint64_t lba) 
{
    if (lba == UINT64_MAX) 
    {
        enqueue(&manager->empty_blocks_queue, nvm_blk_id);  // LBA 为最大值时，构建为空块，使用队列管理
    } 
    else 
    {
        insert(manager->used_blocks_table, lba, nvm_blk_id);  // 否则，构建为有效块，使用哈希表管理
    }
}

// 获取空的 NVM 块号
int get_empty_block(NvmCacheBlkPool *manager, uint64_t *nvm_blk_id) 
{

    // 检查队列中是否有空块
    if (!is_empty(&manager->empty_blocks_queue)) 
    {
        *nvm_blk_id = dequeue(&manager->empty_blocks_queue);
        return 0;
    }

    // 如果队列中没有空块，从哈希表中返回下一个被分配的块号
    HashNode *node = next(manager->used_blocks_table);
    if (node != NULL) 
    {
        *nvm_blk_id = node->value;
        delete_key(manager->used_blocks_table, node->key);
        return 1;
    }

    *nvm_blk_id = UINT64_MAX;
    return 1;
}


uint64_t *search_nvm_blk_of_lba(NvmCacheBlkPool *manager, uint64_t key)
{
    uint64_t *nvm_blk_ptr = search(manager->used_blocks_table, key);
    return nvm_blk_ptr;
}

void destroy_nvm_blk_pool(NvmCacheBlkPool *manager)
{
    destruct_queue(&manager->empty_blocks_queue);
    destruct_hashtable(manager->used_blocks_table);
}
