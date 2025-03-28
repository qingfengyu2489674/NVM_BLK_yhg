#include <gtest/gtest.h>
#include "queue.h"

// 测试初始化队列
TEST(QueueTest, InitQueue) {
    Queue queue;
    init_queue(&queue);
    
    ASSERT_EQ(queue.size, 0UL);
    ASSERT_TRUE(is_empty(&queue));
}

// 测试入队操作
TEST(QueueTest, Enqueue) {
    Queue queue;
    init_queue(&queue);
    
    enqueue(&queue, 10);
    enqueue(&queue, 20);
    enqueue(&queue, 30);
    
    ASSERT_EQ(queue.size, 3UL);
    ASSERT_FALSE(is_empty(&queue));

}

// 测试出队操作
TEST(QueueTest, Dequeue) {
    Queue queue;
    init_queue(&queue);
    
    enqueue(&queue, 10);
    enqueue(&queue, 20);
    enqueue(&queue, 30);
    
    uint64_t value = dequeue(&queue);
    ASSERT_EQ(value, 10UL);
    ASSERT_EQ(queue.size, 2UL);

}

// 测试销毁队列
TEST(QueueTest, DestroyQueue) {
    Queue queue;
    init_queue(&queue);
    
    enqueue(&queue, 10);
    enqueue(&queue, 20);
    enqueue(&queue, 30);
    
    destruct_queue(&queue);
    ASSERT_EQ(queue.size, 0UL);
    ASSERT_TRUE(is_empty(&queue));

}

// 综合测试：入队、出队、销毁队列
TEST(QueueTest, FullTest) {
    Queue queue;
    init_queue(&queue);
    
    enqueue(&queue, 10);
    enqueue(&queue, 20);
    enqueue(&queue, 30);
    
    ASSERT_EQ(queue.size, 3UL);
    ASSERT_EQ(dequeue(&queue), 10UL);
    
    enqueue(&queue, 40);
    
    ASSERT_EQ(queue.size, 3UL);
    ASSERT_EQ(dequeue(&queue), 20UL);
    ASSERT_EQ(dequeue(&queue), 30UL);
    ASSERT_EQ(dequeue(&queue), 40UL);
    
    ASSERT_TRUE(is_empty(&queue));
}
