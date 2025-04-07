#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "../defs.h"

// 哈希节点结构体，用于存储键值对及链表指针
typedef struct HashNode 
{
    u64 key;
    u64 value;
    struct HashNode *next;
} HashNode;

// 哈希表结构体，包含桶数组、桶数量、元素数量以及用于遍历的游标
typedef struct HashTable 
{
    HashNode **buckets;   // 桶数组：每个桶为一个链表头
    size_t size;          // 桶的数量
    size_t count;         // 哈希表中元素总数

    // 游标，用于遍历哈希表
    size_t cursor_bucket; // 当前遍历到的桶索引
    HashNode *cursor_node; // 当前桶链表中的节点
} HashTable;

// 函数声明
HashTable *create_hashtable(size_t size);
int insert(HashTable *ht, u64 key, u64 value);
int delete_key(HashTable *ht, u64 key);
u64 *search(HashTable *ht, u64 key);
int update(HashTable *ht, u64 key, u64 new_value);
void reset_cursor(HashTable *ht);
HashNode *next(HashTable *ht);
void destruct_hashtable(HashTable *ht);


#endif // HASH_TABLE_H
