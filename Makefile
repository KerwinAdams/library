CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lmysqlclient -pthread

# 源文件分组（按实际依赖划分）
SRCS_SERVER = server.c db.c       # 服务端源文件
SRCS_CLIENT = client.c            # 客户端源文件
SRCS_UTILS  = utils.c             # 工具类源文件

# 生成目标文件列表（.c替换为.o）
OBJS_SERVER = $(SRCS_SERVER:.c=.o)  # server.o db.o
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)  # client.o
OBJS_UTILS  = $(SRCS_UTILS:.c=.o)   # utils.o

# 可执行文件目标
TARGETS = server client

# 伪目标（避免与同名文件冲突）
.PHONY: all clean

# 默认目标：编译所有可执行文件
all: $(TARGETS)

# 通用模式规则：所有.c文件编译为.o文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 显式头文件依赖（确保头文件修改时自动重编译）
server.o: dict.h         # server.c依赖dict.h
db.o: dict.h             # db.c依赖dict.h
client.o: dict.h utils.h # client.c依赖dict.h和utils.h
utils.o: utils.h dict.h  # utils.c依赖utils.h和dict.h

# 编译server：仅依赖服务端目标文件
server: $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 编译client：依赖客户端目标文件和工具类目标文件
client: $(OBJS_CLIENT) $(OBJS_UTILS)
	$(CC) $(CFLAGS) -o $@ $^

# 清理编译产物
clean:
	rm -f $(TARGETS) $(OBJS_SERVER) $(OBJS_CLIENT) $(OBJS_UTILS)
