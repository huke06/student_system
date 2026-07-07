/*
 * menu_admin.c - 管理员控制台菜单
 * 含账号管理、选课配置、课程管理、系统配置、日志分析五大模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "logger.h"

/* ===== 子菜单：学生账号管理 ===== */
static void sub_student_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("学生账号管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 新增学生信息\n");
        printf("  2. 修改学生信息\n");
        printf("  3. 删除学生信息\n");
        printf("  4. 浏览与查询学生信息\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：教师账号管理 ===== */
static void sub_teacher_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("教师账号管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 新增教师信息\n");
        printf("  2. 修改教师信息\n");
        printf("  3. 删除教师信息\n");
        printf("  4. 浏览与查询教师信息\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：管理员账号管理 ===== */
static void sub_admin_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("管理员账号管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 新增次级管理员\n");
        printf("  2. 修改管理员信息\n");
        printf("  3. 删除次级管理员\n");
        printf("  4. 重置管理员密码\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：选课周期配置 ===== */
static void sub_select_config(struct Session* s)
{
    int ch;
    while(1) {
        print_header("选课周期配置");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 设置选课时间段\n");
        printf("  2. 手动控制开关（强制开启/关闭选课）\n");
        printf("  3. 选课状态监控\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：课程全局管理 ===== */
static void sub_course_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("课程全局管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 查看所有课程列表\n");
        printf("  2. 按条件筛选课程\n");
        printf("  3. 修改课程信息\n");
        printf("  4. 删除课程\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：系统基础配置 ===== */
static void sub_sys_config(struct Session* s)
{
    int ch;
    while(1) {
        print_header("系统基础配置");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 设置学期开学日期\n");
        printf("  2. 查看推算教学周\n");
        printf("  3. 维护全校专业列表\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 账号信息管理子菜单 ===== */
static void sub_account_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("账号信息管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 学生账号管理\n");
        printf("  2. 教师账号管理\n");
        printf("  3. 管理员账号管理\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        if(ch==1) sub_student_mgr(s);
        else if(ch==2) sub_teacher_mgr(s);
        else if(ch==3) sub_admin_mgr(s);
    }
}

/* ===== 子菜单：日志分析 ===== */
static void sub_log_analysis(struct Session* s)
{
    int ch, type;
    char keyword[64];
    while(1) {
        print_header("日志分析");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 查看最近20条日志\n");
        printf("  2. 按类型筛选日志\n");
        printf("  3. 搜索用户操作记录\n");
        printf("  4. 日志统计概览\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;

        if(ch==1) {
            log_show_recent(20);
            pause_and_continue();
        } else if(ch==2) {
            printf("\n  日志类型：\n");
            printf("  1.登录   2.退出   3.新增   4.修改\n");
            printf("  5.删除   6.错误   7.系统\n");
            type=get_choice("  请选择类型 [1-7]: ", 1, 7);
            log_show_by_type(type);
            pause_and_continue();
        } else if(ch==3) {
            printf("\n  请输入搜索关键词(用户/操作): ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")]='\0';
            if(strlen(keyword)>0) {
                log_search_user(keyword);
            }
            pause_and_continue();
        } else if(ch==4) {
            log_show_stats();
            pause_and_continue();
        }
    }
}

/* ===== 管理员主菜单 ===== */
void admin_menu(struct Session* session)
{
    int ch;
    while(1) {
        print_header("管理员工作台");
        printf("  当前用户: %s (管理员)\n\n", session->username);
        printf("  === 功能菜单 ===\n\n");
        printf("  1. 账号信息管理（学生/教师/管理员）\n");
        printf("  2. 选课周期配置\n");
        printf("  3. 课程全局管理\n");
        printf("  4. 系统基础配置\n");
        printf("  5. 日志分析\n");
        printf("  0. 退出登录\n\n");
        ch=get_choice("请输入选项 [0-5]: ", 0, 5);
        if(ch==0) break;
        if(ch==1) sub_account_mgr(session);
        else if(ch==2) sub_select_config(session);
        else if(ch==3) sub_course_mgr(session);
        else if(ch==4) sub_sys_config(session);
        else if(ch==5) sub_log_analysis(session);
    }
}
