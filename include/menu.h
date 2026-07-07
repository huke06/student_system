/*
 * menu.h - 控制台菜单系统声明
 */

#ifndef MENU_H
#define MENU_H

#include "common.h"

/*
 * 管理员菜单系统
 * session: 当前登录会话
 */
void admin_menu(struct Session* session);

/*
 * 教师菜单系统
 * session: 当前登录会话
 */
void teacher_menu(struct Session* session);

/*
 * 学生菜单系统
 * session: 当前登录会话
 */
void student_menu(struct Session* session);

/*
 * 清屏并打印系统标题
 */
void print_header(const char* title);

/*
 * 读取用户输入的整数选项
 * prompt: 提示文字
 * min/max: 选项范围
 * 返回：用户选择
 */
int get_choice(const char* prompt, int min, int max);

/*
 * 暂停，等待用户按回车继续
 */
void pause_and_continue(void);

#endif /* MENU_H */
