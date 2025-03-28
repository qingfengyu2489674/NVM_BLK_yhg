CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -O2

# Google Test 库和头文件路径
GTEST_LIBS = -lgtest -lgtest_main -pthread
GTEST_INC = -I/usr/src/gtest/include

# 源文件
SRCS = queue.c TestQueue.c

# 输出目标
TARGET = TestQueue

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(GTEST_INC) -o $(TARGET) $(GTEST_LIBS)

clean:
	rm -f $(TARGET) *.o
