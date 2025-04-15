# 设置编译器和编译选项
CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -O0 -g

# Google Test 库和头文件路径
GTEST_LIBS = -lgtest -lgtest_main -pthread
GTEST_INC = -I/usr/src/gtest/include  # 请根据你的 Google Test 安装路径修改
GTEST_LIB = -L/usr/src/gtest/lib  # Google Test 库文件路径，如果没有可以不设置

# 源文件
# 使用通配符查找所有的 .c 文件
SRCS = $(wildcard *.c)

# 输出的目标可执行文件
TARGET = Test_handler

# 默认目标：编译并生成可执行文件
all: $(TARGET)

# 编译目标可执行文件
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(GTEST_INC) $(GTEST_LIB) -o $(TARGET) $(GTEST_LIBS)

# 运行测试
run: $(TARGET)
	./$(TARGET)

# 清理目标文件
clean:
	rm -f $(TARGET) *.o
