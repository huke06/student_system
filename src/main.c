/*教务管理系统控制台入口，纯命令行交互，菜单驱动*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <io.h>
#include <windows.h>
#include "common.h"
#include "utils.h"
#include "menu.h"
#include "logger.h"
#include "data_store.h"

/*用户登录验证
role: 用户角色（由主菜单传入）
返回：1登录成功，0失败
框架阶段：密码"123456"验证通过*/
static int login_flow(struct Session* session, int role)
{
    char userid[32], pwd[32], hash[33];/*hash代表经过凯撒加密后的输出*/
    const char* role_name;
    if(role==ROLE_ADMIN) role_name="管理员";
    else if(role==ROLE_TEACHER) role_name="教师";
    else role_name="学生";
    print_header("用户登录");
    printf("  登录身份: %s\n\n", role_name);
    printf("请输入账号: ");
    fgets(userid, sizeof(userid), stdin);
    userid[strcspn(userid, "\n")]='\0';/*截取换行符前的字符串*/
    str_trim(userid);
    /*读取密码：终端下用*号回显，管道下用fgets*/
    printf("请输入密码: ");
    if(_isatty(_fileno(stdin))) {/*判断是否在控制台窗口*/
        /*交互式终端：逐字读取，回显**/
        int idx=0;
        char ch;
        while(1) {
            ch=getch();/*不会自动打印到屏幕*/
            if(ch=='\r' || ch=='\n') {/*换行符、enter键*/
                pwd[idx]='\0';/*结束字符串*/
                printf("\n");
                break;
            }
            if(ch=='\b' || ch==127) {/*回车键*/
                if(idx>0) {
                    idx--;
                    printf("\b \b");/*\b：光标往左移一格；空格：覆盖掉原来打印的 *；
                    再一个 \b：光标重新回到空位，等待下一次输入。视觉上就是星号消失，实现删除效果。*/
                }
                continue;
            }
            if(idx<31) {
                pwd[idx]=ch;
                idx++;
                printf("*");
            }
        }
    } else {
        /*管道/重定向输入*/
        fgets(pwd, sizeof(pwd), stdin);
        pwd[strcspn(pwd, "\n")]='\0';
    }
    /*凯撒加密*/
    caesar_encrypt(pwd, 3, hash);
    /*从数据文件验证账号密码*/
    {
        int login_ok;
        login_ok=0;
        if(role==ROLE_ADMIN) {
            struct Admin a;
            if(ds_admin_find_by_id(userid, &a)) {
                if(strcmp(hash, a.password)==0) {
                    strcpy(session->username, a.name);
                    login_ok=1;
                }
            }
        } else if(role==ROLE_TEACHER) {
            struct Teacher t;
            if(ds_teacher_find_by_id(userid, &t)) {
                if(strcmp(hash, t.password)==0) {
                    strcpy(session->username, t.name);
                    login_ok=1;
                }
            }
        } else if(role==ROLE_STUDENT) {
            struct Student st;
            if(ds_student_find_by_id(userid, &st)) {
                if(strcmp(hash, st.password)==0) {
                    strcpy(session->username, st.name);
                    login_ok=1;
                }
            }
        }
        if(!login_ok) {
            printf("\n  [错误] 账号或密码错误！\n");
            {
                char log_user[128];
                sprintf(log_user, "%s(%s)", role_name, userid);/*按格式化规则拼接成一段字符串,存入log_user*/
                log_write(LOG_TYPE_ERROR, log_user, "登录失败");
            }
            pause_and_continue();
            return 0;
        }
    }
    /*设置会话*/
    strcpy(session->userid, userid);
    session->role=role;
    /*记录登录成功日志*/
    {
        char log_user[128];
        sprintf(log_user, "%s(%s)", role_name, userid);
        log_write(LOG_TYPE_LOGIN, log_user, "登录成功");
    }
    return 1;
}

/*主函数*/
int main(void)
{
    struct Session session;
    int role;
    /*控制台UTF-8编码*/
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    /*初始化数据存储（首次运行自动创建种子数据）*/
    ds_init();
    while(1) {
        print_header("主菜单");
        printf("  1. 管理员登录\n");
        printf("  2. 教师登录\n");
        printf("  3. 学生登录\n");
        printf("  0. 退出系统\n\n");
        role=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(role==0) {
            print_header("");
            printf("  感谢使用教务管理系统，再见！\n\n");
            log_write(LOG_TYPE_SYSTEM, "系统", "系统退出");
            break;
        }
        /*执行登录（role已由主菜单确定）*/
        if(!login_flow(&session, role)) continue;
        /*进入对应菜单*/
        if(role==ROLE_ADMIN) {
            admin_menu(&session);
        } else if(role==ROLE_TEACHER) {
            teacher_menu(&session);
        } else if(role==ROLE_STUDENT) {
            student_menu(&session);
        }
        /*从角色菜单返回，记录退出登录*/
        {
            char log_user[128];
            sprintf(log_user, "%s(%s)", session.username, session.userid);
            log_write(LOG_TYPE_LOGOUT, log_user, "退出登录");
        }
    }

    return 0;
}
