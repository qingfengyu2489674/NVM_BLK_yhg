#include "io_handler.h"
#include "test_access.h"
#include "blk_pool.h"
#include "mapper.h"
#include "core.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

int nvm_cache_handle_read_io(NvmCache *cache, NvmCacheIoReq *req)
{
    int ret;
    u64 lba_offset = 0;

    while(lba_offset < req->lba_num)
    {
        u64 *nvm_blk_ptr = NULL;
        nvm_blk_ptr = search_nvm_blk_by_lba(cache->blk_pool, req->lba + lba_offset);

        // 判断 lba 是否在 nvm_blk 中，若在，进入内部逻辑，不在，直接转发至 lower_dev 接口
        if(nvm_blk_ptr)
        {
            NvmCacheBlkId blk_id;
            blk_id = CALCULATE_BLK_NUM(cache->mapper->element_num) + *nvm_blk_ptr;
            void *target_buffer = req->buffer + lba_offset * CACHE_BLOCK_SIZE;

            ret = nvm_accessor_read_block(cache->accessor, target_buffer, blk_id);
            if (ret < 0) 
            {
                DEBUG_PRINT("Read block failed, blk_id: %zu, error: %d\n", blk_id, ret);
                return -1;
            } 
            else 
            {
                DEBUG_PRINT("Block %zu read successfully into buffer\n", blk_id);
            }

        }
        else
        {                
            // LowerDevIoReq *lower_req;
            // lower_req = malloc(sizeof(LowerDevIoReq));  // 使用 malloc 替换 kmalloc
            // if (!lower_req) 
            // {
            //     fprintf(stderr, "Error: Failed to allocate memory for LowerDevIoReq\n");
            //     return -ENOMEM;
            // }

            // lower_req->sector     = req->lba + lba_offset;      // 不确定是否可以使用上层传来的 lba 作为起始扇区号
            // lower_req->sector_num = 1;                          // 目前一次只发送一个 sector
            // lower_req->buffer     = req->buffer + lba_offset * CACHE_BLOCK_SIZE;
            // lower_req->dir        = req->dir;

            // ret = lower_dev_io(cache->lower_bdev, lower_req);
            // if (ret != 0) 
            // {
            //     fprintf(stderr, "Error: IO request failed for LBA: %llu, sector: %llu, sector_num: %llu\n", 
            //             req->lba, lower_req->sector, lower_req->sector_num);
            // }
            // free(lower_req);  // 使用 free 替换 kfree，避免内存泄漏
        }
        lba_offset++;
    }
    return 0;
}

int nvm_cache_handle_write_io(NvmCache *cache, NvmCacheIoReq *req)
{
    int ret;
    u64 lba_offset = 0;

    while(lba_offset < req->lba_num)
    {
        u64 *nvm_blk_ptr;

        nvm_blk_ptr = NULL;
        nvm_blk_ptr = search_nvm_blk_by_lba(cache->blk_pool, req->lba + lba_offset);

        void *target_buffer = req->buffer + lba_offset * CACHE_BLOCK_SIZE;

        if(nvm_blk_ptr)
        {
            NvmCacheBlkId blk_id;
            blk_id = CALCULATE_BLK_NUM(cache->mapper->element_num) + *nvm_blk_ptr;

            ret = nvm_accessor_write_block(cache->accessor, target_buffer, blk_id);
            if (ret < 0) 
            {
                DEBUG_PRINT("Write block failed, blk_id: %zu, error: %d\n", blk_id, ret);
                return -1;
            } 
            else 
            {
                DEBUG_PRINT("Block %zu write successfully into buffer\n", blk_id);
            }

        }
        else
        {
            u64 block_id_variable;

            ret = get_empty_block(cache->blk_pool, &block_id_variable);
            NvmCacheBlkId target_back_blk_id;
            target_back_blk_id = CALCULATE_BLK_NUM(cache->mapper->element_num) + block_id_variable;

            nvm_blk_ptr = &block_id_variable;
            if(ret == 1 && block_id_variable == UINT64_MAX)
            {
                fprintf(stderr, "Error: blk_pool allocation failed. Invalid block ID (nvm_blk_ptr: %llu) for lba: %llu\n", 
                        block_id_variable, req->lba + lba_offset);
                return -ENOMEM;  // 如果分配失败，直接返回错误
            }

            // 如果返回的 nvm 块有效且已被分配，需要先将其写回，得到空闲的块后再进行写操作
            // 此时get_empty_block返回1，且nvm_blk_ptr的结果为要写回的nvm块
            if(ret)
            {   
                void *write_back_buffer = malloc(CACHE_BLOCK_SIZE);
                if (!write_back_buffer) 
                {
                    fprintf(stderr, "Failed to allocate memory for write_back_buffer\n");  
                    return -ENOMEM;    
                }

                u64 *write_back_lba = (u64*)malloc(sizeof(u64));
                if (!write_back_lba) 
                {
                    fprintf(stderr, "Error: Failed to allocate memory for write_back_lba\n");
                    free(write_back_buffer);
                    return -ENOMEM;
                }

                // 查询 nvm 上的映射数组，将 *nvm_blk_ptr 转化为对应的 lba 号，写入 *write_back_lba。
                ret = get_lba(cache->accessor, write_back_lba, block_id_variable);
                if (ret < 0) 
                {
                    DEBUG_PRINT("get_lba() failed with error: %d\n", ret);
                    free(write_back_buffer);
                    free(write_back_lba);
                    return ret;
                } 
                else 
                {
                    DEBUG_PRINT("get_lba() succeeded, lba: %lu\n", *write_back_lba);
                }
                
                if(write_back_lba == NULL)
                {
                    fprintf(stderr, "Error: Failed to find NVM block for LBA, nvm_blk=%llu\n", block_id_variable);
                    free(write_back_buffer);
                    free(write_back_lba);
                    return -EINVAL;
                }
    
                ret = nvm_accessor_read_block(cache->accessor, write_back_buffer, target_back_blk_id);
                if (ret < 0) 
                {
                    DEBUG_PRINT("Read block failed, target_back_blk_id: %zu, error: %d\n", target_back_blk_id, ret);
                    free(write_back_buffer);
                    free(write_back_lba);
                    return -1;
                } 
                else 
                {
                    DEBUG_PRINT("target_back_blk_id %zu read successfully into buffer\n", target_back_blk_id);
                }

                // LowerDevIoReq *lower_req;
                // lower_req = malloc(sizeof(LowerDevIoReq));  // 使用 malloc 替换 kmalloc
                // if (!lower_req) {
                //     fprintf(stderr, "Error: Failed to allocate memory for LowerDevIoReq\n");
                //     free(write_back_buffer);
                //     free(write_back_lba);
                //     return -ENOMEM;
                // }

                // lower_req->sector     = *write_back_lba;
                // lower_req->sector_num = 1;             // 目前一次只发送一个 sector
                // lower_req->buffer     = write_back_buffer;
                // lower_req->dir        = req->dir;

                // ret = lower_dev_io(cache->lower_bdev, lower_req);
                // if (ret != 0) 
                // {
                //     fprintf(stderr, "Error: IO request failed for LBA: %llu, sector: %llu, sector_num: %llu\n", 
                //             req->lba, lower_req->sector, lower_req->sector_num);
                // }
            }

            ret = nvm_accessor_write_block(cache->accessor, target_buffer, target_back_blk_id);
            if (ret < 0) 
            {
                DEBUG_PRINT("Write block failed, blk_id: %zu, error: %d\n", target_back_blk_id, ret);
                return -1;
            } 
            else 
            {
                DEBUG_PRINT("Block %zu write successfully into buffer\n", target_back_blk_id);
            }
            build_nvm_valid_block(cache->blk_pool, block_id_variable, req->lba + lba_offset);
        }
        lba_offset++;
    }
    return 0;
}

int nvm_cache_handle_io(NvmCache *cache, NvmCacheIoReq *req)
{
    if(req->dir == CACHE_READ)
    {
        return nvm_cache_handle_read_io(cache, req);
    }
    else
    {
        return nvm_cache_handle_write_io(cache, req);
    }
}
