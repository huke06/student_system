/*
 * menu_student.c - 学生控制台菜单
 * 含个人信息、选课中心、成绩查询、AI助手四大模块
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "utils.h"

/* ===== 子菜单：个人信息管理 ===== */
static void sub_my_info(struct Session* s)
{
    int ch;
    while(1) {
        print_header("个人信息管理");
        printf("  当前用户: %s (学生)\n\n", s->username);
        printf("  1. 查看个人基本信息\n");
        printf("  2. 修改联系电话\n");
        printf("  3. 修改登录密码\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：选课中心 ===== */
static void sub_course_select(struct Session* s)
{
    int ch;
    while(1) {
        print_header("选课中心");
        printf("  当前用户: %s (学生)\n\n", s->username);
        printf("  1. 浏览可选课程列表\n");
        printf("  2. 查看课程详情\n");
        printf("  3. 执行选课操作\n");
        printf("  4. 查看我的课表\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：成绩查询与统计 ===== */
static void sub_grade_query(struct Session* s)
{
    int ch;
    while(1) {
        print_header("成绩查询与统计");
        printf("  当前用户: %s (学生)\n\n", s->username);
        printf("  1. 查看已出成绩列表\n");
        printf("  2. 查看成绩明细（平时/期末/总评/绩点）\n");
        printf("  3. 成绩统计分析（加权平均/GPA）\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        printf("\n  [提示] 该功能将在数据持久化模块完成后实现。\n");
        pause_and_continue();
    }
}

/* ===== 子菜单：AI智能学习助手 ===== */
static void sub_ai_analysis(struct Session* s)
{
    print_header("AI智能学习助手");
    printf("  当前用户: %s (学生)\n\n", s->username);
    printf("  该功能将拉取您的全部已出成绩数据，\n");
    printf("  通过AI大模型进行分析，输出：\n\n");
    printf("    - 成绩画像\n");
    printf("    - 优势学科分析\n");
    printf("    - 薄弱点提示\n");
    printf("    - 学习改进建议\n\n");
    printf("  [提示] 该功能将在成绩数据模块完成后实现。\n");
    pause_and_continue();
}

/* ===== 学生主菜单 ===== */
void student_menu(struct Session* session)
{
    int ch;
    while(1) {
        print_header("学生中心");
        printf("  当前用户: %s (学生)\n\n", session->username);
        printf("  === 功能菜单 ===\n\n");
        printf("  1. 个人信息管理\n");
        printf("  2. 选课中心\n");
        printf("  3. 成绩查询与统计\n");
        printf("  4. AI智能学习助手\n");
        printf("  0. 退出登录\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) sub_my_info(session);
        else if(ch==2) sub_course_select(session);
        else if(ch==3) sub_grade_query(session);
        else if(ch==4) sub_ai_analysis(session);
    }
}
