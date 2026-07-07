/*
 * menu_teacher.c - 教师控制台菜单
 * 含课程管理、选课名单、成绩录入三大模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"

/* ===== 子菜单：课程开设与管理 ===== */
static void sub_course_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("课程开设与管理");
        printf("  当前用户: %s (教师)\n\n", s->username);
        printf("  1. 新增课程\n");
        printf("  2. 修改课程信息\n");
        printf("  3. 查看我的课程\n");
        printf("  4. 发布课程（未发布->选课中）\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：选课名单管理 ===== */
static void sub_roster(struct Session* s)
{
    int ch;
    while(1) {
        print_header("选课名单管理");
        printf("  当前用户: %s (教师)\n\n", s->username);
        printf("  1. 查看课程选课名单\n");
        printf("  2. 搜索选课学生\n");
        printf("  3. 导出选课名单（文本格式）\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：成绩录入与管理 ===== */
static void sub_grade_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("成绩录入与管理");
        printf("  当前用户: %s (教师)\n\n", s->username);
        printf("  1. 课程结课操作\n");
        printf("  2. 逐条录入成绩\n");
        printf("  3. 修改已有成绩\n");
        printf("  4. 成绩统计分析\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 教师主菜单 ===== */
void teacher_menu(struct Session* session)
{
    int ch;
    while(1) {
        print_header("教师工作台");
        printf("  当前用户: %s (教师)\n\n", session->username);
        printf("  === 功能菜单 ===\n\n");
        printf("  1. 课程开设与管理\n");
        printf("  2. 选课名单管理\n");
        printf("  3. 成绩录入与管理\n");
        printf("  0. 退出登录\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        if(ch==1) sub_course_mgr(session);
        else if(ch==2) sub_roster(session);
        else if(ch==3) sub_grade_mgr(session);
    }
}
