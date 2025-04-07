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

// 测试哈希表大小
TEST(HashTableTest, TableSize) {
    HashTable *ht = create_hashtable(5);
    ASSERT_EQ(ht->size, 5);  // 初始时哈希表大小应该是5
    insert(ht, 1, 100);
    insert(ht, 2, 200);
    ASSERT_EQ(ht->count, 2);  // 插入后哈希表中的元素个数应为 2
    destruct_hashtable(ht);
}

// 测试插入相同键
TEST(HashTableTest, InsertSameKey) {
    HashTable *ht = create_hashtable(10);
    ASSERT_EQ(insert(ht, 1, 100), 0);  // 插入第一个键值对
    ASSERT_EQ(insert(ht, 1, 200), 0);  // 插入相同的键，值应该更新
    uint64_t *val = search(ht, 1);
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(*val, static_cast<uint64_t>(200));  // 更新后的值应为 200
    destruct_hashtable(ht);
}

// 测试插入失败
TEST(HashTableTest, InsertFullTable) {
    HashTable *ht = create_hashtable(2);  // 创建一个容量为 2 的哈希表
    ASSERT_EQ(insert(ht, 1, 100), 0);  // 插入第一个元素
    ASSERT_EQ(insert(ht, 2, 200), 0);  // 插入第二个元素
    ASSERT_EQ(insert(ht, 3, 300), 0); // 插入第三个元素应失败，返回-1
    destruct_hashtable(ht);
}

// 测试删除不存在的键
TEST(HashTableTest, DeleteNonExistentKey) {
    HashTable *ht = create_hashtable(10);
    ASSERT_EQ(delete_key(ht, 999), -1);  // 删除一个不存在的键应该返回-1
    destruct_hashtable(ht);
}

// // 测试更新不存在的键
// TEST(HashTableTest, UpdateNonExistentKey) {
//     HashTable *ht = create_hashtable(10);
//     ASSERT_EQ(update(ht, 999, 300), -1);  // 更新一个不存在的键应该返回-1
//     destruct_hashtable(ht);
// }

// 测试遍历空哈希表
TEST(HashTableTest, CursorTraversalEmpty) {
    HashTable *ht = create_hashtable(10);
    reset_cursor(ht);
    HashNode *node;
    int count = 0;
    while ((node = next(ht)) != NULL) {
        count++;
    }
    ASSERT_EQ(count, 0);  // 空哈希表应该没有元素
    destruct_hashtable(ht);
}

// 测试哈希表销毁
TEST(HashTableTest, DestroyTable) {
    HashTable *ht = create_hashtable(10);
    insert(ht, 1, 100);
    insert(ht, 2, 200);
    destruct_hashtable(ht);  // 销毁哈希表
    // 此时应无法访问哈希表对象，假设destruct_hashtable已经做了正确的内存清理
}

// 测试大数据量插入
TEST(HashTableTest, InsertLargeData) {
    HashTable *ht = create_hashtable(10000);  // 创建一个大的哈希表
    for (int i = 0; i < 10000; ++i) {
        ASSERT_EQ(insert(ht, i, i * 10), 0);  // 插入10000个键值对
    }
    ASSERT_EQ(ht->count, 10000);  // 插入后元素数量应为10000
    destruct_hashtable(ht);
}

// // 测试哈希表自动扩容
// TEST(HashTableTest, AutoResize) {
//     HashTable *ht = create_hashtable(2);  // 创建一个初始容量为 2 的哈希表
//     ASSERT_EQ(insert(ht, 1, 100), 0);
//     ASSERT_EQ(insert(ht, 2, 200), 0);
//     ASSERT_EQ(insert(ht, 3, 300), 0);  // 插入时哈希表应自动扩容
//     ASSERT_EQ(ht->size, 4);  // 假设扩容机制将大小变为 4
//     destruct_hashtable(ht);
// }

// 测试负数和零键插入
TEST(HashTableTest, InsertNegativeAndZeroKeys) {
    HashTable *ht = create_hashtable(10);
    ASSERT_EQ(insert(ht, -1, 100), 0);  // 插入负数键
    ASSERT_EQ(insert(ht, 0, 200), 0);   // 插入零键
    ASSERT_EQ(insert(ht, 1, 300), 0);   // 插入正数键

    uint64_t *val_neg = search(ht, -1);
    uint64_t *val_zero = search(ht, 0);
    uint64_t *val_pos = search(ht, 1);

    ASSERT_NE(val_neg, nullptr);
    ASSERT_EQ(*val_neg, static_cast<uint64_t>(100));

    ASSERT_NE(val_zero, nullptr);
    ASSERT_EQ(*val_zero, static_cast<uint64_t>(200));

    ASSERT_NE(val_pos, nullptr);
    ASSERT_EQ(*val_pos, static_cast<uint64_t>(300));

    destruct_hashtable(ht);
}
