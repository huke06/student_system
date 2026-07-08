# Makefile - 教务管理系统（控制台版）
# 纯命令行交互，菜单驱动

CC=gcc
CFLAGS=-Wall -Wextra -I./include
LDFLAGS=
TARGET=edu_system.exe

SRCS=src/main.c src/menu_admin.c src/menu_teacher.c src/menu_student.c src/utils.c src/logger.c src/data_store.c src/ai_helper.c
OBJS=src/main.o src/menu_admin.o src/menu_teacher.o src/menu_student.o src/utils.o src/logger.o src/data_store.o src/ai_helper.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo ====================================
	@echo  编译完成: $(TARGET)
	@echo  运行: .\$(TARGET)
	@echo ====================================

src/main.o: src/main.c include/common.h include/utils.h include/menu.h include/logger.h include/data_store.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/menu_admin.o: src/menu_admin.c include/menu.h include/logger.h include/data_store.h include/utils.h
	$(CC) $(CFLAGS) -c src/menu_admin.c -o src/menu_admin.o

src/menu_teacher.o: src/menu_teacher.c include/menu.h
	$(CC) $(CFLAGS) -c src/menu_teacher.c -o src/menu_teacher.o

src/menu_student.o: src/menu_student.c include/menu.h include/utils.h include/data_store.h include/ai_helper.h
	$(CC) $(CFLAGS) -c src/menu_student.c -o src/menu_student.o

src/utils.o: src/utils.c include/utils.h
	$(CC) $(CFLAGS) -c src/utils.c -o src/utils.o


src/logger.o: src/logger.c include/logger.h
	$(CC) $(CFLAGS) -c src/logger.c -o src/logger.o

src/data_store.o: src/data_store.c include/data_store.h include/common.h include/logger.h
	$(CC) $(CFLAGS) -c src/data_store.c -o src/data_store.o

src/ai_helper.o: src/ai_helper.c include/ai_helper.h include/data_store.h include/utils.h
	$(CC) $(CFLAGS) -c src/ai_helper.c -o src/ai_helper.o

clean:
	rm -f src/*.o $(TARGET)
	@echo 清理完成

rebuild: clean $(TARGET)

.PHONY: clean rebuild
