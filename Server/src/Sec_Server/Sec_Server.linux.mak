#定义编译器
CXX = g++

#定义编译标志，注意json的库包含到json目录内会报错，所以这里只包含到include
CXXFLAGS = -I../include -I../include/openssl -I../include -std=c++11 -Wall -g


# 定义链接标志
LDFLAGS = -L../../lib -lprotobuf -lssl -lcrypto -ljson_linux-gcc-4.8.5_libmt -pthread

# 定义输出目录和可执行文件名
BIN_DIR = ../../bin
TARGET = $(BIN_DIR)/Sec_Server

# 定义源文件目录
SRC_DIR = ../../src
SHARE_DIR = $(SRC_DIR)/share
SEC_SERVER_DIR = $(SRC_DIR)/Sec_Server

# 定义源文件
SHARE_SRCS = $(SHARE_DIR)/ShareMemManager.cpp \
                         $(SHARE_DIR)/TcpServer.cpp \
						 $(SHARE_DIR)/TcpSocket.cpp \
                         $(SHARE_DIR)/openssl/AesCrypto.cpp \
						 $(SHARE_DIR)/openssl/Hash.cpp \
						 $(SHARE_DIR)/openssl/RsaCrypto.cpp
						 
SEC_SERVER_SRCS = $(SEC_SERVER_DIR)/Codec.cpp \
                                  $(SEC_SERVER_DIR)/main.cpp \
								  $(SEC_SERVER_DIR)/Message.pb.cc \
                                  $(SEC_SERVER_DIR)/RequestCodec.cpp \
								  $(SEC_SERVER_DIR)/ResponCodec.cpp \
								  $(SEC_SERVER_DIR)/SecServer.cpp

# 定义对象文件，把.cpp都先替换成.o
OBJS = $(SHARE_SRCS:.cpp=.o) $(SEC_SERVER_SRCS:.cpp=.o)

# 默认目标
all: $(TARGET) clean_objects

# 编译规则
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 生成对象文件的规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
# 自动清理所有 .o 文件,.pb.cc文件除外，生成可执行文件后执行
clean_objects:
	rm -f $(filter-out %.pb.cc, $(OBJS))
	@echo "清理目标文件成功!"

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)

# 伪目标
.PHONY: all clean clean_objects
