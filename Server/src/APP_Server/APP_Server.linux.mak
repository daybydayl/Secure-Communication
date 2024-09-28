#定义编译器
CXX = g++

#定义编译标志，注意json的库包含到json目录内会报错，所以这里只包含到include
CXXFLAGS = -I../include -I../include/openssl -I../include -std=c++11 -Wall -g


# 定义链接标志
LDFLAGS = -L../../lib -lEnc_Dec_DLL -lprotobuf -lssl -lcrypto -ljson_linux-gcc-4.8.5_libmt -pthread

# 定义输出目录和可执行文件名
BIN_DIR = ../../bin
TARGET = $(BIN_DIR)/APP_Server

# 定义源文件目录
SRC_DIR = ../../src
SHARE_DIR = $(SRC_DIR)/share
SEC_CLIENT_DIR = $(SRC_DIR)/APP_Server

# 定义源文件
SHARE_SRCS = $(SHARE_DIR)/ShareMemManager.cpp \
						 $(SHARE_DIR)/TcpSocket.cpp \
                         $(SHARE_DIR)/openssl/AesCrypto.cpp \
						 $(SHARE_DIR)/TcpServer.cpp
						 
SEC_CLIENT_SRCS = $(SEC_CLIENT_DIR)/APP_Server.cpp 

# 定义对象文件，把.cpp都先替换成.o
OBJS = $(SHARE_SRCS:.cpp=.o) $(SEC_CLIENT_SRCS:.cpp=.o)

# 默认目标
all: $(TARGET) clean_objects

# 编译规则
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 生成对象文件的规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 自动清理所有 .o 文件,.pb.cc除外，生成可执行文件后执行
clean_objects:
	rm -f $(OBJS)
	@echo "清理目标文件成功!"

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean clean_objects
