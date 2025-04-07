#include <gtest/gtest.h>
#include "defs.h"
#include "test_access.h"

// 测试初始化 NvmAccessor
TEST(NvmAccessorTest, Init) {
    NvmAccessor* accessor = nvm_accessor_init();
    ASSERT_NE(accessor, nullptr);
    ASSERT_EQ(accessor->is_valid, 1);  // 确保 NvmAccessor 被初始化为有效
    nvm_accessor_destory(accessor);  // 销毁 NvmAccessor
}

// 测试写入和读取块
TEST(NvmAccessorTest, WriteReadBlock) {
    NvmAccessor* accessor = nvm_accessor_init();
    ASSERT_NE(accessor, nullptr);

    char write_buffer[512] = {0};  // 假设一个 512 字节块
    memset(write_buffer, 0xAA, 512);  // 填充测试数据

    // 写入一个块
    size_t written = nvm_accessor_write_block(accessor, write_buffer, 1);
    ASSERT_EQ(written, 512);  // 应该写入 512 字节

    // 读取块
    char read_buffer[512] = {0};
    size_t read = nvm_accessor_read_block(accessor, read_buffer, 1);
    ASSERT_EQ(read, 512);  // 应该读取 512 字节

    // 比较数据是否一致
    for (int i = 0; i < 512; ++i) {
        ASSERT_EQ(write_buffer[i], read_buffer[i]);
    }

    nvm_accessor_destory(accessor);  // 销毁 NvmAccessor
}

// 测试写入和读取字节
TEST(NvmAccessorTest, WriteReadByte) {
    NvmAccessor* accessor = nvm_accessor_init();
    ASSERT_NE(accessor, nullptr);

    u64 data_to_write = 9876543210;
    u64 data_read = 0;

    // 写入字节
    size_t written = nvm_accessor_write_byte(accessor, &data_to_write, 100);
    ASSERT_EQ(written, sizeof(u64));  // 应该写入 8 字节

    // 读取字节
    size_t read = nvm_accessor_read_byte(accessor, &data_read, 100);
    ASSERT_EQ(read, sizeof(u64));  // 应该读取 8 字节
    ASSERT_EQ(data_read, data_to_write);

    nvm_accessor_destory(accessor);  // 销毁 NvmAccessor
}

// 多轮重复测试块操作
TEST(NvmAccessorTest, RepeatedBlockOperations) {
    NvmAccessor* accessor = nvm_accessor_init();
    ASSERT_NE(accessor, nullptr);

    const int iterations = 10;
    for (int i = 0; i < iterations; ++i) {
        char write_buffer[512] = {0};
        // 不同轮次使用不同数据：写入 i 值
        memset(write_buffer, i, 512);

        // 写入块 i
        size_t written = nvm_accessor_write_block(accessor, write_buffer, i);
        ASSERT_EQ(written, 512);

        char read_buffer[512] = {0};
        size_t read = nvm_accessor_read_block(accessor, read_buffer, i);
        ASSERT_EQ(read, 512);

        // 验证数据是否一致
        for (int j = 0; j < 512; ++j) {
            ASSERT_EQ(write_buffer[j], read_buffer[j]);
        }
    }

    nvm_accessor_destory(accessor);
}

// 多轮重复测试字节操作
TEST(NvmAccessorTest, RepeatedByteOperations) {
    NvmAccessor* accessor = nvm_accessor_init();
    ASSERT_NE(accessor, nullptr);

    const int iterations = 10;
    // 测试不同偏移量的写入与读取
    for (int i = 0; i < iterations; ++i) {
        u64 data_to_write = 1000 + i;
        u64 data_read = 0;
        u64 offset = 200 + i * sizeof(u64);  // 不同偏移量

        size_t written = nvm_accessor_write_byte(accessor, &data_to_write, offset);
        ASSERT_EQ(written, sizeof(u64));

        size_t read = nvm_accessor_read_byte(accessor, &data_read, offset);
        ASSERT_EQ(read, sizeof(u64));
        ASSERT_EQ(data_read, data_to_write);
    }

    nvm_accessor_destory(accessor);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
