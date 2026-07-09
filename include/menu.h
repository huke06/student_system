/* menu.h - 控制台菜单系统声明 */

#ifndef MENU_H
#define MENU_H

#include "common.h"
/*管理员/教师/学生菜单入口，session 为当前登录会话*/
void admin_menu(struct Session* session);
void teacher_menu(struct Session* session);
void student_menu(struct Session* session);
/*清屏并打印系统标题*/
void print_header(const char* title);
/*读取整数选项，带范围校验，返回用户选择*/
int get_choice(const char* prompt, int min, int max);
/*暂停，等待用户按回车继续*/
void pause_and_continue(void);
/*从系统配置加载专业列表，展示并让用户单选，返回选中编号(0=手输)*/
int pick_major(const char* label, char* out);
/*专业多选器（用于课程适用专业），输出逗号分隔列表，返回选中数量*/
int pick_majors_multi(const char* label, char* out);

#endif /* MENU_H */
