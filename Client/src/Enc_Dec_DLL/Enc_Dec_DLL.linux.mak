# 定义编译器
CXX = g++

# 编译选项:
# - -fPIC：生成位置无关的代码，用于动态库
# - -Wall：显示所有警告
# - -g：生成调试信息
# - -O2：优化代码
CXXFLAGS = -I../../src/include -I../../src/include/openssl -fPIC -Wall -g -O2

# 链接选项:
# - -shared：生成动态库
LDFLAGS = -shared

# 定义库的输出目录
LIB_DIR = ../../lib

# 动态库的目标文件
TARGET = $(LIB_DIR)/libEnc_Dec_DLL.so

# 源文件路径定义
SRC_DIR = ../../src
SHARE_DIR = $(SRC_DIR)/share
ENC_DEC_DIR = $(SRC_DIR)/Enc_Dec_DLL

# 包含的头文件路径
INCLUDE_DIR = $(SRC_DIR)/include
OPENSSL_DIR = $(INCLUDE_DIR)/openssl

# 源文件
SRCS = $(SHARE_DIR)/ShareMemManager.cpp \
       $(SHARE_DIR)/openssl/AesCrypto.cpp \
       $(ENC_DEC_DIR)/Enc_Dec_Tool.cpp

# 对象文件的生成规则，将源文件的 .cpp 替换成 .o
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET) clean_objects

# 生成动态库 Enc_Dec_DLL.so
$(TARGET): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^
	@echo "动态库 $(TARGET) 生成成功!"

# 生成每个 .o 对象文件的规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "编译 $< 成功，生成目标文件 $@"
	
# 自动清理所有 .o 文件，生成库文件后执行
clean_objects:
	rm -f $(OBJS)
	@echo "清理目标文件成功!"

# 清理编译生成的文件
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "清理完成!"

# 伪目标，防止与文件名冲突
.PHONY: all clean clean_objects
