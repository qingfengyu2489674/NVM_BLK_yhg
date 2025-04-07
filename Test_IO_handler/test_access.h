// test_access.h

#ifndef TEST_ACCESS_H
#define TEST_ACCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"

// 定义 NvmAccessor 结构体
typedef struct NvmAccessor {
    int is_valid;  // 模拟 NvmAccessor 是否有效
} NvmAccessor;

// 初始化 NvmAccessor
NvmAccessor* nvm_accessor_init();

// 写入一个块
size_t nvm_accessor_write_block(NvmAccessor* accessor, void *buffer, u64 blkId);

// 读取一个块
size_t nvm_accessor_read_block(NvmAccessor* accessor, void *buffer, u64 blkId);

// 写入字节
size_t nvm_accessor_write_byte(NvmAccessor* accessor, void *buffer, u64 offset);

// 读取字节
size_t nvm_accessor_read_byte(NvmAccessor* accessor, void *buffer, u64 offset);

// 销毁 NvmAccessor
int nvm_accessor_destory(NvmAccessor* accessor);

#endif // TEST_ACCESS_H
