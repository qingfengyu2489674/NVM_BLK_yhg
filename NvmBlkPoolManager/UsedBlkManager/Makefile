CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -O2
GTEST_DIR = /usr/src/gtest  # 如果你将 Google Test 安装到此目录

# 哈希表源文件
SRCS = hashtable.c

# Google Test 库和头文件路径
GTEST_LIBS = -lgtest -lgtest_main -pthread
GTEST_INC = -I/usr/src/gtest/include  # 修正头文件路径

# 测试文件
TEST_SRCS = TestHashTable.c

# 最终可执行文件
TEST_BIN = TestHashTable

all: $(TEST_BIN)

# 编译并链接测试文件
$(TEST_BIN): $(SRCS) $(TEST_SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(TEST_SRCS) $(GTEST_INC) -o $(TEST_BIN) $(GTEST_LIBS)

clean:
	rm -f $(TEST_BIN) *.o
