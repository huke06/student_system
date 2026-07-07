/*
 * main.c - 教务管理系统程序入口
 * 设置控制台编码并启动HTTP服务器
 */

#include <stdio.h>
#include <stdlib.h>
#include "http_server.h"
#include <windows.h>

int main(void)
{
    /* 设置控制台代码页为UTF-8 */
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printf("Starting Edu Management System...\n");

    /* 启动HTTP服务器，进入主循环 */
    if(http_server_start() != 0) {
        printf("Server start failed!\n");
        system("pause");
        return 1;
    }

    return 0;
}
