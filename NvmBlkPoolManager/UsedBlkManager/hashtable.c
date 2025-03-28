#include "hashtable.h"
#include <stdlib.h>

// 内部哈希函数：简单取模
static size_t hash_func(uint64_t key, size_t size) 
{
    return key % size;
}

// 创建哈希表
HashTable *create_hashtable(size_t size) 
{
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht) return NULL;
    ht->buckets = (HashNode **)calloc(size, sizeof(HashNode *));
    if (!ht->buckets) 
    {
        free(ht);
        return NULL;
    }
    ht->size = size;
    ht->count = 0;
    ht->cursor_bucket = 0;
    ht->cursor_node = NULL;
    return ht;
}

// 插入操作：如果键存在则更新，不存在则插入新节点（插入到链表头）
int insert(HashTable *ht, uint64_t key, uint64_t value) 
{
    if (!ht) 
        return -1;
    size_t index = hash_func(key, ht->size);
    HashNode *node = ht->buckets[index];

    while (node) 
    {
        if (node->key == key) 
        {
            node->value = value;
            return 0;
        }
        node = node->next;
    }
    // 创建新节点
    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    if (!newNode) 
        return -1;
    newNode->key = key;
    newNode->value = value;
    newNode->next = ht->buckets[index];
    ht->buckets[index] = newNode;
    ht->count++;
    return 0;
}

// 删除操作：删除指定键对应的节点
int delete_key(HashTable *ht, uint64_t key) 
{
    if (!ht) 
        return -1;
    size_t index = hash_func(key, ht->size);
    HashNode *node = ht->buckets[index];
    HashNode *prev = NULL;

    while (node) 
    {
        if (node->key == key) 
        {
            if (prev)
                prev->next = node->next;
            else
                ht->buckets[index] = node->next;
            free(node);
            ht->count--;
            return 0;
        }
        prev = node;
        node = node->next;
    }
    return -1;
}

// 查找操作：返回指向键对应值的指针，找不到返回 NULL
uint64_t *search(HashTable *ht, uint64_t key) 
{
    if (!ht) 
        return NULL;
    size_t index = hash_func(key, ht->size);
    HashNode *node = ht->buckets[index];
    while (node) 
    {
        if (node->key == key)
            return &node->value;
        node = node->next;
    }
    return NULL;
}

// 更新操作：存在则更新，不存在则插入
int update(HashTable *ht, uint64_t key, uint64_t new_value) 
{
    return insert(ht, key, new_value);
}

// 重置游标，便于遍历哈希表
void reset_cursor(HashTable *ht) 
{
    if (!ht) 
        return;
    ht->cursor_bucket = 0;
    ht->cursor_node = NULL;
}

// 使用游标获取下一个节点，遍历整个哈希表
HashNode *next(HashTable *ht) 
{
    if (!ht) return NULL;
    // 当前链表中还有下一个节点，则直接返回
    if (ht->cursor_node && ht->cursor_node->next) 
    {
        ht->cursor_node = ht->cursor_node->next;
        return ht->cursor_node;
    }
    // 否则继续遍历桶数组
    while (ht->cursor_bucket < ht->size) 
    {
        HashNode *node = ht->buckets[ht->cursor_bucket];
        ht->cursor_bucket++;
        if (node) 
        {
            ht->cursor_node = node;
            return node;
        }
    }
    return NULL;
}

// 释放哈希表内存
void destruct_hashtable(HashTable *ht) 
{
    if (!ht) 
        return;
    for (size_t i = 0; i < ht->size; i++) 
    {
        HashNode *node = ht->buckets[i];
        while (node) 
        {
            HashNode *temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}
