/* menu.h - 控制台菜单系统声明 */

#ifndef MENU_H
#define MENU_H

#include "common.h"
/*管理员/教师/学生菜单入口，session 为当前登录会话*/
void admin_menu(struct Session* session);
void teacher_menu(struct Session* session);
void student_menu(struct Session* session);

#endif /* MENU_H */
