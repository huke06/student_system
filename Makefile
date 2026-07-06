# Makefile - 教务管理系统
# 工程结构: include/ 头文件, src/ 源文件

CC=gcc
CFLAGS=-Wall -Wextra -I./include
LDFLAGS=-lws2_32
TARGET=edu_system.exe

# 所有源文件
SRCS=src/main.c src/http_server.c src/page_login.c src/page_admin.c src/page_teacher.c src/page_student.c
OBJS=src/main.o src/http_server.o src/page_login.o src/page_admin.o src/page_teacher.o src/page_student.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo ====================================
	@echo  编译完成: $(TARGET)
	@echo  运行: .\$(TARGET)
	@echo ====================================

# 各模块编译规则
src/main.o: src/main.c include/common.h include/http_server.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/http_server.o: src/http_server.c include/common.h include/http_server.h include/page.h
	$(CC) $(CFLAGS) -c src/http_server.c -o src/http_server.o

src/page_login.o: src/page_login.c include/page.h
	$(CC) $(CFLAGS) -c src/page_login.c -o src/page_login.o

src/page_admin.o: src/page_admin.c include/page.h
	$(CC) $(CFLAGS) -c src/page_admin.c -o src/page_admin.o

src/page_teacher.o: src/page_teacher.c include/page.h
	$(CC) $(CFLAGS) -c src/page_teacher.c -o src/page_teacher.o

src/page_student.o: src/page_student.c include/page.h
	$(CC) $(CFLAGS) -c src/page_student.c -o src/page_student.o

# 清理编译产物
clean:
	del /f /q src\*.o $(TARGET) 2>nul || rm -f src/*.o $(TARGET)
	@echo 清理完成

rebuild: clean $(TARGET)

.PHONY: clean rebuild
