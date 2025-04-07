// test_access.c

#include "test_access.h"

// 模拟一个文件路径作为模拟设备
#define NVM_SIMULATED_DEVICE "/tmp/nvm_simulated_device.bin"

// 初始化 NvmAccessor
NvmAccessor* nvm_accessor_init() {
    NvmAccessor* accessor = (NvmAccessor*)malloc(sizeof(NvmAccessor));
    if (!accessor) {
        return NULL;
    }

    accessor->is_valid = 1;  // 模拟有效

    // 确保模拟设备文件存在，如果不存在则创建它
    FILE *file = fopen(NVM_SIMULATED_DEVICE, "r+b");
    if (!file) {
        // 文件不存在，尝试创建并初始化
        file = fopen(NVM_SIMULATED_DEVICE, "w+b");
        if (!file) {
            perror("Failed to create simulated device");
            free(accessor);
            return NULL;
        }
        // 关闭文件创建模拟设备
        fclose(file);
    }

    return accessor;
}

// 写入一个块
size_t nvm_accessor_write_block(NvmAccessor* accessor, void *buffer, u64 blkId) {
    if (!accessor || !accessor->is_valid || !buffer) {
        return 0;
    }

    FILE *file = fopen(NVM_SIMULATED_DEVICE, "r+b");
    if (!file) {
        perror("Failed to open simulated device");
        return 0;
    }

    fseek(file, blkId * 512, SEEK_SET);  // 定位到块的位置，假设块大小为 512 字节
    size_t written = fwrite(buffer, 1, 512, file);  // 写入 512 字节
    fclose(file);

    if (written != 512) {
        perror("Failed to write block");
        return 0;
    }

    printf("Writing block %llu with size 512 bytes\n", blkId);
    return 512;  // 返回写入的字节数
}

// 读取一个块
size_t nvm_accessor_read_block(NvmAccessor* accessor, void *buffer, u64 blkId) {
    if (!accessor || !accessor->is_valid || !buffer) {
        return 0;
    }

    FILE *file = fopen(NVM_SIMULATED_DEVICE, "rb");
    if (!file) {
        perror("Failed to open simulated device");
        return 0;
    }

    fseek(file, blkId * 512, SEEK_SET);  // 定位到块的位置
    size_t read = fread(buffer, 1, 512, file);  // 读取 512 字节
    fclose(file);

    if (read != 512) {
        perror("Failed to read block");
        return 0;
    }

    printf("Reading block %llu into buffer\n", blkId);
    return 512;  // 返回读取的字节数
}

// 写入字节
size_t nvm_accessor_write_byte(NvmAccessor* accessor, void *buffer, u64 offset) {
    if (!accessor || !accessor->is_valid || !buffer) {
        return 0;
    }

    FILE *file = fopen(NVM_SIMULATED_DEVICE, "r+b");
    if (!file) {
        perror("Failed to open simulated device");
        return 0;
    }

    fseek(file, offset, SEEK_SET);  // 定位到指定偏移位置
    size_t written = fwrite(buffer, 1, sizeof(u64), file);  // 写入 8 字节（即一个 u64）
    fclose(file);

    if (written != sizeof(u64)) {
        perror("Failed to write byte");
        return 0;
    }

    printf("Writing byte at offset %llu with value %llu\n", offset, *((u64*)buffer));
    return sizeof(u64);  // 返回写入的字节数
}

// 读取字节
size_t nvm_accessor_read_byte(NvmAccessor* accessor, void *buffer, u64 offset) {
    if (!accessor || !accessor->is_valid || !buffer) {
        return 0;
    }

    FILE *file = fopen(NVM_SIMULATED_DEVICE, "rb");
    if (!file) {
        perror("Failed to open simulated device");
        return 0;
    }

    fseek(file, offset, SEEK_SET);  // 定位到指定偏移位置
    size_t read = fread(buffer, 1, sizeof(u64), file);  // 读取 8 字节
    fclose(file);

    if (read != sizeof(u64)) {
        perror("Failed to read byte");
        return 0;
    }

    printf("Reading byte at offset %llu\n", offset);

    return sizeof(u64);  // 返回读取的字节数
}

// 销毁 NvmAccessor
int nvm_accessor_destory(NvmAccessor* accessor) {
    if (!accessor || !accessor->is_valid) {
        return -1;
    }

    // 假设释放资源
    free(accessor);
    return 0;
}
