#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "queue.h"

// 初始化队列
void init_queue(Queue *queue) 
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}

// 判断队列是否为空
int is_empty(Queue *queue) 
{
    return queue->size == 0;
}

// 从队列头插入数据
void enqueue(Queue *queue, uint64_t value) 
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) 
    {
        fprintf(stderr, "Memory allocation failed!\n");
        return;
    }
    new_node->data = value;
    new_node->next = queue->head;
    queue->head = new_node;
    if (queue->size == 0) 
    {
        queue->tail = new_node;
    }
    queue->size++;
}

// 从队列尾删除并返回数据
uint64_t dequeue(Queue *queue) 
{
    if (is_empty(queue)) 
    {
        fprintf(stderr, "Queue is empty!\n");
        return 0; // 可根据需要处理异常情况
    }
    
    Node *current = queue->head;
    Node *previous = NULL;
    while (current->next != NULL) 
    {
        previous = current;
        current = current->next;
    }
    
    uint64_t value = current->data;
    
    // 删除尾部节点
    if (previous == NULL) 
    {
        queue->head = NULL;
        queue->tail = NULL;
    } 
    else 
    {
        previous->next = NULL;
        queue->tail = previous;
    }
    
    free(current);
    queue->size--;
    
    return value;
}

// 销毁队列，释放所有节点的内存
void destruct_queue(Queue *queue) 
{
    while (!is_empty(queue)) 
    {
        dequeue(queue);
    }
}

