#include <gtest/gtest.h>

// 使用 extern "C" 来确保 C 语言代码以 C 语言链接

#include "hashtable.h"  // 包含 C 代码的头文件

// 测试哈希表的创建
TEST(HashTableTest, CreateTable) {
    HashTable *ht = create_hashtable(10);
    ASSERT_NE(ht, nullptr);  // 确保哈希表已成功创建
    ASSERT_EQ(ht->count, static_cast<uint64_t>(0));  // 初始时元素数量应为 0
    destruct_hashtable(ht);
}

// 测试插入和查找
TEST(HashTableTest, InsertAndSearch) {
    HashTable *ht = create_hashtable(10);
    ASSERT_EQ(insert(ht, 1, 100), 0);  // 插入成功
    uint64_t *val = search(ht, 1);
    ASSERT_NE(val, nullptr);  // 查找时应能找到
    ASSERT_EQ(*val, static_cast<uint64_t>(100));  // 查找的值应为 100
    destruct_hashtable(ht);
}

// 测试更新操作
TEST(HashTableTest, Update) {
    HashTable *ht = create_hashtable(10);
    ASSERT_EQ(insert(ht, 1, 100), 0);
    ASSERT_EQ(update(ht, 1, 200), 0);  // 更新成功
    uint64_t *val = search(ht, 1);
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, static_cast<uint64_t>(200));  // 更新后的值应为 200
    destruct_hashtable(ht);
}

// 测试删除操作
TEST(HashTableTest, DeleteKey) {
    HashTable *ht = create_hashtable(10);
    ASSERT_EQ(insert(ht, 1, 100), 0);
    ASSERT_EQ(delete_key(ht, 1), 0);  // 删除成功
    uint64_t *val = search(ht, 1);
    ASSERT_EQ(val, nullptr);  // 查找应返回 NULL
    destruct_hashtable(ht);
}

// 测试遍历哈希表
TEST(HashTableTest, CursorTraversal) {
    HashTable *ht = create_hashtable(10);
    insert(ht, 1, 100);
    insert(ht, 2, 200);
    insert(ht, 3, 300);

    reset_cursor(ht);
    HashNode *node;
    int count = 0;
    while ((node = next(ht)) != NULL) {
        count++;
    }
    ASSERT_EQ(count, 3);  // 哈希表中应有 3 个元素
    destruct_hashtable(ht);
}
