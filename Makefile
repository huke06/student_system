# Makefile - 教务管理系统
# 工程结构: include/ 头文件, src/ 源文件, static/ 前端模板

CC=gcc
CFLAGS=-Wall -Wextra -I./include
LDFLAGS=-lws2_32
TARGET=edu_system.exe

SRCS=src/main.c src/http_server.c src/page_handler.c src/utils.c src/md5.c
OBJS=src/main.o src/http_server.o src/page_handler.o src/utils.o src/md5.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo ====================================
	@echo  编译完成: $(TARGET)
	@echo  运行: .\$(TARGET)
	@echo ====================================

src/main.o: src/main.c include/common.h include/http_server.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/http_server.o: src/http_server.c include/common.h include/http_server.h include/page.h
	$(CC) $(CFLAGS) -c src/http_server.c -o src/http_server.o

src/page_handler.o: src/page_handler.c include/page.h
	$(CC) $(CFLAGS) -c src/page_handler.c -o src/page_handler.o

src/utils.o: src/utils.c include/utils.h
	$(CC) $(CFLAGS) -c src/utils.c -o src/utils.o

src/md5.o: src/md5.c include/utils.h
	$(CC) $(CFLAGS) -c src/md5.c -o src/md5.o

clean:
	rm -f src/*.o $(TARGET)
	@echo 清理完成

rebuild: clean $(TARGET)

.PHONY: clean rebuild
