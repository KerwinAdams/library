# 编译器与编译选项
CC = gcc
CFLAGS = -Wall  -g   # -Wall 显示所有警告，-g 生成调试信息
LDFLAGS = -lmysqlclient -pthread  # 同时链接 MySQL 客户端库和线程库（推荐用 -pthread）

# 要生成的可执行文件
TARGETS = server client

# 伪目标：默认执行 all（生成所有可执行文件）
.PHONY: all clean
all: $(TARGETS)

# 生成 server 可执行文件（链接 MySQL 和线程库）
server: server.o db.o
	$(CC) $(CFLAGS) -o server server.o db.o  $(LDFLAGS)

# 编译 server.o（依赖 server.c、dict.h）
server.o: server.c  dict.h
	$(CC) $(CFLAGS) -c server.c

# 编译 db.o（依赖 db.c、dict.h）
db.o: db.c dict.h
	$(CC) $(CFLAGS) -c db.c

# 编译 utils.o（依赖 utils.c、utils.h）
utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

# 生成 client 可执行文件（无需链接数据库和线程库）
client: client.o utils.o
	$(CC) $(CFLAGS) -o client client.o utils.o

# 编译 client.o（依赖 client.c、utils.h）
client.o: client.c utils.h
	$(CC) $(CFLAGS) -c client.c

# 伪目标：清理编译产物
clean:
	rm -f $(TARGETS) *.o