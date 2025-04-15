#include "io_handler.h"
#include "test_access.h"
#include "blk_pool.h"
#include "mapper.h"
#include "core.h"
#include "format.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

int main() 
{
    // Create a virtual NvmCacheMapper
    NvmCacheMapper *mapper = NULL;

    // Simulate physical memory length, in bytes
    u64 nvm_phy_length = 1024 * 10;  // 1MB (can change as needed)

    // Initialize cache_mapper
    mapper = cache_mapper_init(nvm_phy_length);
    printf("Cache Mapper Initialized with %llu elements\n", mapper->element_num);

    // Create virtual NvmCache object
    NvmCache *cache;

    // Create virtual NvmAccessor and NvmCacheBlkPool
    NvmAccessor *accessor;
    NvmCacheBlkPool *blk_pool;

    accessor = nvm_accessor_init();

    blk_pool = init_nvm_blk_pool(10);

    cache = nvm_cache_init(accessor,  mapper, blk_pool);  
    
    int result = nvm_cache_format(cache);
    if (result != 0) {
        printf("nvm_cache_format failed\n");
        return -1;
    }

    result = cache_mapper_scan(cache);
    if (result != 0) {
        printf("Cache mapper scan failed\n");
        return -1;
    }

    traverse_valid_blk(blk_pool);

    // Prepare a test I/O request
    NvmCacheIoReq req;
    req.lba = 0;            // Starting LBA
    req.lba_num = 2;        // Number of LBAs to read/write
    req.buffer = malloc(CACHE_BLOCK_SIZE * req.lba_num);  // Allocate buffer
    req.dir = CACHE_READ;   // Set direction as read (could be CACHE_WRITE for testing write)

    if (!req.buffer) {
        fprintf(stderr, "Failed to allocate buffer for I/O request\n");
        return -ENOMEM;
    }

    // Test read I/O request
    printf("Testing read I/O request...\n");
    result = nvm_cache_handle_io(cache, &req);
    if (result != 0) {
        printf("nvm_cache_handle_io failed for read\n");
    } else {
        printf("Read I/O request completed successfully\n");
    }

    // Modify the request for write I/O
    req.dir = CACHE_WRITE;  // Change direction to write

    // Test write I/O request
    printf("Testing write I/O request...\n");
    result = nvm_cache_handle_io(cache, &req);
    if (result != 0) {
        printf("nvm_cache_handle_io failed for write\n");
    } else {
        printf("Write I/O request completed successfully\n");
    }

    // Clean up
    free(req.buffer);  // Free allocated buffer for I/O
    cache_mapper_destruct(mapper);

    return 0;
}
