#include "io_handler.h"
#include "test_access.h"
#include "blk_pool.h"
#include "mapper.h"
#include "core.h"
#include "format.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h> // 包含 bool 类型

#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

// --- 压力测试配置 ---
#define NUM_TEST_ROUNDS 50     // 定义测试循环的总轮数 (增加这个值以提高压力)
#define LBA_COUNT_PER_REQ 3    // 每次读/写请求操作的 LBA 数量 (保持和之前一致)
// --- 结束配置 ---

int main()
{
    // 创建虚拟 NvmCacheMapper
    NvmCacheMapper *mapper = NULL;

    // 模拟物理内存长度 (字节)
    u64 nvm_phy_length = 1024 * 10; // 10KB (可根据需要调整)

    // 检查 CACHE_BLOCK_SIZE 定义
    if (CACHE_BLOCK_SIZE == 0) {
         fprintf(stderr, "错误: CACHE_BLOCK_SIZE 未定义或为零。\n");
         return -1;
    }
    printf("CACHE_BLOCK_SIZE = %d 字节\n", CACHE_BLOCK_SIZE);

    // 初始化 cache_mapper
    mapper = cache_mapper_init(nvm_phy_length);
     if (!mapper) {
        fprintf(stderr, "错误: 初始化 cache mapper 失败\n");
        return -1;
    }
    u64 expected_elements = nvm_phy_length / CACHE_BLOCK_SIZE;
    printf("缓存映射器初始化完成. 物理大小: %llu 字节. 预期元素数: %llu. 实际元素数: %llu\n",
           nvm_phy_length, expected_elements, mapper->element_num);

    // 创建虚拟 NvmCache 对象
    NvmCache *cache;

    // 创建虚拟 NvmAccessor 和 NvmCacheBlkPool
    NvmAccessor *accessor;
    NvmCacheBlkPool *blk_pool;

    accessor = nvm_accessor_init();
    if (!accessor) {
         fprintf(stderr, "错误: 初始化 nvm accessor 失败\n");
         cache_mapper_destruct(mapper);
         return -1;
    }

    // 使用 mapper->element_num 初始化块池大小，以保持一致
    blk_pool = init_nvm_blk_pool(mapper->element_num);
     if (!blk_pool) {
         fprintf(stderr, "错误: 初始化块池失败\n");
         cache_mapper_destruct(mapper);
         return -1;
    }
    printf("块池初始化完成，包含 %llu 个块\n", mapper->element_num);

    cache = nvm_cache_init(accessor,  mapper, blk_pool);
    if (!cache) {
         fprintf(stderr, "错误: 初始化 nvm cache 失败\n");
         cache_mapper_destruct(mapper);
         return -1;
    }
    printf("Nvm Cache 初始化完成.\n");

    // 格式化缓存
    printf("正在格式化 NVM 缓存...\n");
    int result = nvm_cache_format(cache);
    if (result != 0) {
        printf("nvm_cache_format 失败，错误码: %d\n", result);
        cache_mapper_destruct(mapper);
        return -1;
    }
    printf("NVM 缓存格式化完成.\n");

    // 扫描映射器
    printf("正在扫描缓存映射器...\n");
    result = cache_mapper_scan(cache);
    if (result != 0) {
        printf("缓存映射器扫描失败，错误码: %d\n", result);
        cache_mapper_destruct(mapper);
        return -1;
    }
     printf("缓存映射器扫描完成.\n");

    printf("块池初始状态:\n");
    traverse_valid_blk(blk_pool); // 显示可用块

    // --- 准备压力测试 ---
    NvmCacheIoReq req;
    req.lba_num = LBA_COUNT_PER_REQ;        // 每次请求的 LBA 数量
    size_t buffer_size = CACHE_BLOCK_SIZE * req.lba_num;
    req.buffer = malloc(buffer_size);      // 分配 IO 缓冲区 (一次分配，循环使用)

    if (!req.buffer) {
        fprintf(stderr, "错误: 为 IO 请求分配缓冲区失败\n");
        cache_mapper_destruct(mapper);
        return -ENOMEM;
    }
    printf("为 %d 个 LBA 分配了 %zu 字节的缓冲区\n", req.lba_num, buffer_size);

    // --- 压力测试循环 ---
    printf("\n--- 开始压力测试 (共 %d 轮) ---\n", NUM_TEST_ROUNDS);
    bool overall_success = true; // 跟踪整个测试是否成功
    u64 max_lba_start = 0; // 可用的最大起始 LBA

    // 计算可以开始写的最大LBA，防止 lba + lba_num 超出范围
    // 这里用物理块数作为逻辑地址空间的近似上限，实际系统可能不同
    if (mapper->element_num >= req.lba_num) {
        max_lba_start = mapper->element_num - req.lba_num;
    } else {
        fprintf(stderr, "警告: 缓存大小 (%llu 块) 小于单次请求大小 (%d 块)，测试可能受限或失败。\n",
                mapper->element_num, req.lba_num);
        if (mapper->element_num > 0) {
            req.lba_num = mapper->element_num; // 尝试缩小请求大小
            buffer_size = CACHE_BLOCK_SIZE * req.lba_num;
             printf("警告: 请求大小已调整为 %d LBA (%zu 字节)\n", req.lba_num, buffer_size);
             max_lba_start = 0; // 只能从0开始
        } else {
            overall_success = false; // 无法测试
            printf("错误: 缓存物理块数为 0，无法进行测试。\n");
        }
    }


    if (overall_success) // 只有在基本设置合理时才开始测试
    {
        for (int round = 0; round < NUM_TEST_ROUNDS; ++round) {
            printf("\n--- 测试轮次 %d / %d ---\n", round + 1, NUM_TEST_ROUNDS);

            // 1. 计算本轮测试的 LBA (循环使用可用 LBA 范围)
            if (max_lba_start > 0 || mapper->element_num >= req.lba_num) { // 确保至少有一个起始位置
                 req.lba = (u64)(round * req.lba_num) % (max_lba_start + 1);
            } else {
                 // 如果 max_lba_start 是 0 (因为缓存太小只能放一个请求)，则 LBA 总是 0
                 req.lba = 0;
            }

            // 2. 准备要写入的数据 (每轮使用不同模式，这里用 'A'+轮数%26)
            char write_pattern = 'A' + (round % 26);
            printf("第 %d 轮: 准备向 LBA %llu 写入 %d 个块，数据模式 '%c' (ASCII %d)\n",
                   round + 1, req.lba, req.lba_num, write_pattern, write_pattern);
            memset(req.buffer, write_pattern, buffer_size);

            // 3. 执行写操作
            req.dir = CACHE_WRITE;
            printf("第 %d 轮: 提交写请求...\n", round + 1);
            result = nvm_cache_handle_io(cache, &req);
            if (result != 0) {
                printf("第 %d 轮: 写操作失败! LBA %llu, 错误码: %d\n", round + 1, req.lba, result);
                overall_success = false;
                continue; // 继续下一轮测试，或者可以选择 break 停止
            } else {
                printf("第 %d 轮: 写操作成功完成。\n", round + 1);
            }

            // 4. 准备读操作 - 清空缓冲区
            printf("第 %d 轮: 清空缓冲区准备回读...\n", round + 1);
            memset(req.buffer, 0, buffer_size); // 清空，确保读到的是新数据

            // 5. 执行读操作
            req.dir = CACHE_READ;
            printf("第 %d 轮: 提交读请求 (LBA %llu)...\n", round + 1, req.lba);
            result = nvm_cache_handle_io(cache, &req);
            bool read_succeeded = false;
            if (result != 0) {
                printf("第 %d 轮: 读操作失败! LBA %llu, 错误码: %d\n", round + 1, req.lba, result);
                overall_success = false;
                continue; // 继续下一轮
            } else {
                printf("第 %d 轮: 读操作成功完成。\n", round + 1);
                read_succeeded = true;
            }

            // 6. 验证读回的数据 (仅当读操作成功时)
            if (read_succeeded) {
                printf("第 %d 轮: 验证读回的数据 (LBA %llu)...\n", round + 1, req.lba);
                bool verified_this_round = true;
                unsigned char* read_buf = (unsigned char*)req.buffer;
                for (size_t i = 0; i < buffer_size; ++i) {
                    if (read_buf[i] != (unsigned char)write_pattern) {
                        verified_this_round = false;
                        fprintf(stderr, "第 %d 轮: 数据验证失败! LBA %llu, 偏移 %zu. 期望 '%c'(%d), 实际 '%c'(%d / 0x%02X)\n",
                                round + 1, req.lba, i,
                                write_pattern, write_pattern,
                                (isprint(read_buf[i]) ? read_buf[i] : '.'), // 尝试打印字符，否则打印点
                                read_buf[i], read_buf[i]);
                        overall_success = false; // 标记整体测试失败
                        break; // 找到第一个错误就停止本轮验证
                    }
                }

                if (verified_this_round) {
                    printf("第 %d 轮: 数据验证通过!\n", round + 1);
                } else {
                    printf("第 %d 轮: 数据验证失败!\n", round + 1);
                    // 可选: 打印读回的错误缓冲区内容
                    /*
                    printf("----- 错误数据缓冲区内容 (前 64 字节) -----\n");
                    for (size_t k=0; k < buffer_size && k < 64; ++k) printf("%02X ", read_buf[k]);
                    printf("\n----------------------------------------\n");
                    */
                }
            } // 结束验证块

        } // 结束 for 循环 (测试轮次)
    } // 结束 if (overall_success) for initial checks

    printf("\n--- 压力测试完成 ---\n");
    printf("总体测试结果: %s\n", overall_success ? "通过 (PASSED)" : "失败 (FAILED)");

    printf("块池最终状态:\n");
    traverse_valid_blk(blk_pool); // 显示测试后的块池状态

    // --- 清理 ---
    printf("\n正在清理资源...\n");
    free(req.buffer);      // 释放 IO 缓冲区
    cache_mapper_destruct(mapper); // 清理映射器
    // 注意: cache, blk_pool, accessor 可能需要各自的清理函数，这里遵循原始代码只清理 mapper
    printf("清理完成。\n");

    return overall_success ? 0 : -1; // 根据测试结果返回状态码
}

// 确保包含了 isprint 所需的头文件
