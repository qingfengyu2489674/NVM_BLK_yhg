#include "io_handler.h"
#include "lower_dev.h"
#include "NvmBlkPoolManager/blk_pool.h"
#include "mapper.h"
#include "core.h"
#include "defs.h"


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

            blk_id = CALCULATE_BLK_ID(cache->mapper->element_num) + *nvm_blk_ptr;

            void *target_buffer = req->buffer + lba_offset * CACHE_BLOCK_SIZE;

            // 执行读取操作
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
            LowerDevIoReq *lower_req;
            lower_req = kmalloc(sizeof(LowerDevIoReq), GFP_KERNEL);
            if (!lower_req) 
            {
                pr_err("Error: Failed to allocate memory for LowerDevIoReq\n");
                return -ENOMEM;
            }

            lower_req->sector     = req->lba + lba_offset;      // 不确定是否可以使用上层传来的 lba 作为起始扇区号
            lower_req->sector_num = 1;                          // 目前一次只发送一个 sector
            lower_req->buffer     = req->buffer + lba_offset * CACHE_BLOCK_SIZE;
            lower_req->dir        = req->dir;

            ret = lower_dev_io(cache->lower_bdev, lower_req);
            if (ret != 0) 
            {
                pr_err("Error: IO request failed for LBA: %llu, sector: %llu, sector_num: %llu\n", 
                       req->lba, lower_req->sector, lower_req->sector_num);
            }
            kfree(lower_req);  // 释放内存，避免泄露
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
        u64 *nvm_blk_ptr = NULL;
        nvm_blk_ptr = search_nvm_blk_by_lba(cache->blk_pool, req->lba + lba_offset);

        // 判断 lba 是否在 nvm_blk 中，若在，进入内部逻辑，不在，直接转发至 lower_dev 接口
        if(nvm_blk_ptr)
        {
            NvmCacheBlkId blk_id;

            blk_id = CALCULATE_BLK_ID(cache->mapper->element_num) + *nvm_blk_ptr;

            void *target_buffer = req->buffer + lba_offset * CACHE_BLOCK_SIZE;

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
            ret = get_empty_block(cache->blk_pool, nvm_blk_ptr);
            if(ret == 1 && *nvm_blk_ptr == UINT64_MAX)
            {
                pr_err("Error: blk_pool allocation failed. Invalid block ID (nvm_blk_ptr: %llu) for lba: %llu\n", 
                    *nvm_blk_ptr, req->lba + lba_offset);
            }

            // 如果返回的 nvm 块有效且已被分配，需要先将其写回，得到空闲的块后再进行写操作
            if(ret)
            {   
                void *write_back_buffer = NULL;
                write_back_buffer = kmalloc(CACHE_BLOCK_SIZE, GFP_KERNEL);
                if (!write_back_buffer) 
                {
                    pr_err("Failed to allocate memory for write_back_buffer\n");  
                    return -ENOMEM;    
                }

                u64 *write_back_lba = NULL;

                // 查询 nvm 上的映射数组，将 *nvm_blk_ptr 转化为对应的 lba 号，写入 *write_back_lba。
                ret = get_lba(cache->accessor, write_back_lba, *nvm_blk_ptr);
                if (ret < 0) 
                {
                    DEBUG_PRINT("get_lba() failed with error: %d\n", ret);
                    return ret;
                } 
                else 
                {
                    DEBUG_PRINT("get_lba() succeeded, lba: %lu\n", write_back_lba);
                }
                
                if(write_back_lba == NULL)
                {
                    pr_err("Error: Failed to find NVM block for LBA, nvm_blk=%llu\n", *nvm_blk_ptr);
                }

                // TODO: 将数据从 nvm 块中读到 write_back_buffer 中，为下一步写回做准备
                NvmCacheBlkId write_back_blk_id;

                write_back_blk_id = CALCULATE_BLK_ID(cache->mapper->element_num) + *write_back_lba;
    
                ret = nvm_accessor_read_block(cache->accessor, write_back_buffer, write_back_blk_id);
                if (ret < 0) 
                {
                    DEBUG_PRINT("Read block failed, write_back_blk_id: %zu, error: %d\n", write_back_blk_id, ret);
                    return -1;
                } 
                else 
                {
                    DEBUG_PRINT("write_back_blk_id %zu read successfully into buffer\n", write_back_blk_id);
                }

                LowerDevIoReq *lower_req;
                lower_req = kmalloc(sizeof(LowerDevIoReq), GFP_KERNEL);
                if (!lower_req) {
                    pr_err("Error: Failed to allocate memory for LowerDevIoReq\n");
                    return -ENOMEM;
                }

                lower_req->sector     = *write_back_lba;
                lower_req->sector_num = 1;             // 目前一次只发送一个 sector
                lower_req->buffer     = write_back_buffer;
                lower_req->dir        = req->dir;

                ret = lower_dev_io(cache->lower_bdev, lower_req);
                if (ret != 0) 
                {
                    pr_err("Error: IO request failed for LBA: %llu, sector: %llu, sector_num: %llu\n", 
                           req->lba, lower_req->sector, lower_req->sector_num);
                }

                // TODO: 对 write_back_blk_id 所指向的 nvm 块进行写操作

                void *target_buffer = req->buffer + lba_offset * CACHE_BLOCK_SIZE;

                ret = nvm_accessor_write_block(cache->accessor, target_buffer, write_back_blk_id);
                if (ret < 0) 
                {
                    DEBUG_PRINT("Write block failed, blk_id: %zu, error: %d\n", write_back_blk_id, ret);
                    return -1;
                } 
                else 
                {
                    DEBUG_PRINT("Block %zu write successfully into buffer\n", write_back_blk_id);
                }


                // 在 blk_pool 中进行记录
                build_nvm_block(cache->blk_pool, nvm_blk_ptr, req->lba + lba_offset);

                kfree(lower_req);      // 释放 lower_req 内存
                kfree(write_back_buffer);  // 释放 write_back_buffer 内存
            }
        }
        lba_offset++;
    }
    return 0;
}


