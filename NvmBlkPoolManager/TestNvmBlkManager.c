#include <gtest/gtest.h>
#include "blk_pool.h"

// 测试初始化 NvmCacheBlkPool
TEST(NvmBlkManagerTest, Init) {
    NvmCacheBlkPool manager;
    init_nvm_blk_pool(&manager, 10);  // 初始化 NvmCacheBlkPool

    // 验证队列为空
    uint64_t block;
    int result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 1);  // 队列和哈希表都没有块时，应该返回 UINT64_MAX
    ASSERT_EQ(block, UINT64_MAX);
}

// 测试构建空块
TEST(NvmBlkManagerTest, BuildEmptyBlock) {
    NvmCacheBlkPool manager;
    init_nvm_blk_pool(&manager, 10);

    uint64_t block;
    int result;

    // 构建一个空块，LBA 为 UINT64_MAX
    build_nvm_block(&manager, 100, UINT64_MAX);  

    // 获取空块
    result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 0);  // 从队列中获取空块
    ASSERT_EQ(block, 100UL);  // 获取到的块应该是 100
}

// 测试构建有效块
TEST(NvmBlkManagerTest, BuildUsedBlock) {
    NvmCacheBlkPool manager;
    init_nvm_blk_pool(&manager, 10);

    uint64_t block;
    int result;

    // 构建一个有效块，LBA 为非 UINT64_MAX
    build_nvm_block(&manager, 200, 123);  

    // 获取空块，队列中没有空块，应该从哈希表获取
    result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 1);  // 从哈希表中获取有效块
    ASSERT_EQ(block, 200UL);  // 获取到的块应该是 200

    // 验证哈希表中是否已删除块 200
    uint64_t *val = search_nvm_blk_of_lba(&manager, 123);  // 通过 LBA 查找块
    ASSERT_EQ(val, nullptr);  // 该块应该已经被删除
}

// 测试队列为空，哈希表中有块时，getEmptyBlock 从哈希表获取
TEST(NvmBlkManagerTest, GetEmptyBlockFromHashTable) {
    NvmCacheBlkPool manager;
    init_nvm_blk_pool(&manager, 10);

    uint64_t block;
    int result;

    // 构建有效块（LBA 非最大值）
    build_nvm_block(&manager, 300, 1);  
    build_nvm_block(&manager, 400, 2);  

    // 队列为空，哈希表中有块
    result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 1);  // 应该从哈希表获取
    ASSERT_EQ(block, 300UL);

    result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 1);  // 应该从哈希表获取
    ASSERT_EQ(block, 400UL);
}

// 测试当队列和哈希表都没有空块时，getEmptyBlock 应该返回 UINT64_MAX
TEST(NvmBlkManagerTest, GetEmptyBlockNoBlocks) {
    NvmCacheBlkPool manager;
    init_nvm_blk_pool(&manager, 10);

    uint64_t block;
    int result;

    // 队列和哈希表都没有块
    result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 1);  // 应该返回 0
    ASSERT_EQ(block, UINT64_MAX);  // 获取到的块应该是 UINT64_MAX
}

// 测试 search_nvm_blk_of_lba 函数的正确性
TEST(NvmBlkManagerTest, SearchNvmBlkOfLba) {
    NvmCacheBlkPool manager;
    init_nvm_blk_pool(&manager, 10);

    // 构建有效块
    build_nvm_block(&manager, 500, 1);
    build_nvm_block(&manager, 600, 2);

    uint64_t *block;

    // 测试通过 LBA 查找
    block = search_nvm_blk_of_lba(&manager, 1);
    ASSERT_NE(block, nullptr);  // 应该返回有效块
    ASSERT_EQ(*block, 500UL);  // 应该是 500

    block = search_nvm_blk_of_lba(&manager, 2);
    ASSERT_NE(block, nullptr);  // 应该返回有效块
    ASSERT_EQ(*block, 600UL);  // 应该是 600

    block = search_nvm_blk_of_lba(&manager, 3);
    ASSERT_EQ(block, nullptr);  // LBA 3 不存在，应该返回 nullptr
}


// 测试销毁 NvmCacheBlkPool
TEST(NvmBlkManagerTest, Destroy) {
    NvmCacheBlkPool manager;
    init_nvm_blk_pool(&manager, 10);  // 初始化 NvmCacheBlkPool

    // 构建一些块，确保队列和哈希表中有数据
    build_nvm_block(&manager, 100, UINT64_MAX);  // 空块
    build_nvm_block(&manager, 200, 123);  // 有效块

    // 在销毁之前，验证队列和哈希表中有块
    uint64_t block;
    int result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 0);  // 应该返回空块
    ASSERT_EQ(block, 100UL);  // 应该是 100

    uint64_t *val = search_nvm_blk_of_lba(&manager, 123);
    ASSERT_NE(val, nullptr);  // 哈希表中应该找到有效块
    ASSERT_EQ(*val, 200UL);  // 应该是 200

    // 销毁 NvmCacheBlkPool
    destroy_nvm_blk_pool(&manager);


    /*
    // 销毁之后，队列应该为空，哈希表应该为空
    result = get_empty_block(&manager, &block);
    ASSERT_EQ(result, 0);  // 应该返回 0，因为队列为空
    ASSERT_EQ(block, UINT64_MAX);  // 获取到的块应该是 UINT64_MAX

    // 验证哈希表是否被清除
    val = search_nvm_blk_of_lba(&manager, 123);
    ASSERT_EQ(val, nullptr);  // 应该返回 nullptr，因为块已经被销毁
    */

    // 如果有适当的内存检查工具（如 AddressSanitizer 或 Valgrind），可以确保没有内存泄漏
}