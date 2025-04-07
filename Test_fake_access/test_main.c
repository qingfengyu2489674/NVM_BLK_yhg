#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_access.h"

int main(void) {
    // 初始化 NvmAccessor
    NvmAccessor* accessor = nvm_accessor_init();
    if (!accessor) {
        fprintf(stderr, "Failed to initialize NvmAccessor\n");
        return EXIT_FAILURE;
    }
    
    // 每块大小 512 字节，总共需要写入 4096 块（4096 * 512 = 2MB）
    const size_t blockSize = 512;
    const size_t totalBlocks = 4096;
    
    // 分配并初始化一个块数据，这里全部填充为 0xAA
    char *buffer = (char *)malloc(blockSize);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate block buffer\n");
        nvm_accessor_destory(accessor);
        return EXIT_FAILURE;
    }
    memset(buffer, 0xAA, blockSize);

    // 循环写入每个块
    for (u64 blkId = 0; blkId < totalBlocks; blkId++) {
        size_t written = nvm_accessor_write_block(accessor, buffer, blkId);
        if (written != blockSize) {
            fprintf(stderr, "Failed to write block %llu\n", blkId);
            free(buffer);
            nvm_accessor_destory(accessor);
            return EXIT_FAILURE;
        }
    }
    

    free(buffer);
    nvm_accessor_destory(accessor);
    return EXIT_SUCCESS;
}
