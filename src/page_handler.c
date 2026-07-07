/*
 * page_handler.c - 页面处理器
 * 读取HTML模板文件，替换占位符，生成侧边栏
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "page.h"

static char buf[65536];

/*
 * 读取文件全部内容到字符串
 * filepath: 文件路径
 * 返回：文件内容字符串（需free），失败返回NULL
 */
static char* read_file(const char* filepath)
{
    FILE* fp;
    long fsize;
    char* content;

    fp=fopen(filepath, "rb");
    if(fp==NULL) return NULL;

    fseek(fp, 0, SEEK_END);
    fsize=ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* 多分配8KB给占位符替换留空间 */
    content=(char*)malloc(fsize+8192);
    if(content==NULL) {
        fclose(fp);
        return NULL;
    }
    memset(content, 0, fsize+8192);

    fread(content, 1, fsize, fp);
    content[fsize]='\0';
    fclose(fp);
    return content;
}

/*
 * 在字符串中替换所有出现的占位符
 * str: 源字符串（原地修改，需保证足够空间）
 * from: 占位符，如 "{{username}}"
 * to: 替换值
 */
static void replace_all(char* str, const char* from, const char* to)
{
    char* pos;
    int from_len, to_len, tail_len;

    from_len=strlen(from);
    to_len=strlen(to);

    pos=strstr(str, from);
    while(pos!=NULL) {
        tail_len=strlen(pos+from_len);

        /* 如果替换后长度不同，需要移动尾部数据 */
        if(to_len != from_len) {
            memmove(pos+to_len, pos+from_len, tail_len+1);
        }
        memcpy(pos, to, to_len);

        pos=strstr(pos+to_len, from);
    }
}

/* ================================================================ */
/*                     侧边栏生成                                    */
/* ================================================================ */

/*
 * 生成带参数的URL
 */
static void mkurl(char* out, const char* path,
                  const char* userid, const char* username, const char* role)
{
    sprintf(out, "%s?userid=%s&username=%s&role=%s",
            path, userid, username, role);
}

/*
 * 生成侧边栏导航链接
 * active: 当前页面名，用于高亮
 */
static void nav_link(char* out, const char* userid, const char* username,
                     const char* role, const char* path, const char* label,
                     const char* active)
{
    char u[256];
    mkurl(u, path, userid, username, role);
    sprintf(out, "  <a href=\"%s\"%s>%s</a>\n",
        u, strcmp(active, path) == 0 ? " class=\"active\"" : "", label);
}

/*
 * 生成管理员侧边栏HTML
 */
static char* sidebar_admin(const char* username, const char* userid,
                           const char* active)
{
    char* s=(char*)malloc(4096);
    char h[512], u[256];

    if(s==NULL) return NULL;

    mkurl(u, "/admin", userid, username, "admin");
    sprintf(s,
        "<div class=\"sidebar\">\n"
        "  <div class=\"logo\"><a href=\"%s\">教务管理系统</a></div>\n"
        "  <div class=\"user-info\">当前用户<span>%s (管理员)</span></div>\n",
        u, username);

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">账号信息管理</div>\n");
    nav_link(h, userid, username, "admin", "/admin/student_mgr", "学生账号管理", active); strcat(s, h);
    nav_link(h, userid, username, "admin", "/admin/teacher_mgr", "教师账号管理", active); strcat(s, h);
    nav_link(h, userid, username, "admin", "/admin/admin_mgr", "管理员账号管理", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">选课周期配置</div>\n");
    nav_link(h, userid, username, "admin", "/admin/select_config", "选课时间设置", active); strcat(s, h);
    nav_link(h, userid, username, "admin", "/admin/select_force", "强制开关控制", active); strcat(s, h);
    nav_link(h, userid, username, "admin", "/admin/select_monitor", "选课状态监控", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">课程全局管理</div>\n");
    nav_link(h, userid, username, "admin", "/admin/course_list", "查看所有课程", active); strcat(s, h);
    nav_link(h, userid, username, "admin", "/admin/course_edit", "修改课程信息", active); strcat(s, h);
    nav_link(h, userid, username, "admin", "/admin/course_delete", "删除课程", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">系统基础配置</div>\n");
    nav_link(h, userid, username, "admin", "/admin/sys_semester", "学期日期设置", active); strcat(s, h);
    nav_link(h, userid, username, "admin", "/admin/sys_majors", "专业列表维护", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "</div>\n");
    return s;
}

/*
 * 生成教师侧边栏HTML
 */
static char* sidebar_teacher(const char* username, const char* userid,
                             const char* active)
{
    char* s=(char*)malloc(4096);
    char h[512], u[256];

    if(s==NULL) return NULL;

    mkurl(u, "/teacher", userid, username, "teacher");
    sprintf(s,
        "<div class=\"sidebar\">\n"
        "  <div class=\"logo\"><a href=\"%s\">教务管理系统</a></div>\n"
        "  <div class=\"user-info\">当前用户<span>%s (教师)</span></div>\n",
        u, username);

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">课程开设与管理</div>\n");
    nav_link(h, userid, username, "teacher", "/teacher/course_add", "新增课程", active); strcat(s, h);
    nav_link(h, userid, username, "teacher", "/teacher/course_edit", "修改课程信息", active); strcat(s, h);
    nav_link(h, userid, username, "teacher", "/teacher/course_list", "查看我的课程", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">选课名单管理</div>\n");
    nav_link(h, userid, username, "teacher", "/teacher/roster", "查看选课名单", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">成绩录入与管理</div>\n");
    nav_link(h, userid, username, "teacher", "/teacher/grade_input", "成绩录入", active); strcat(s, h);
    nav_link(h, userid, username, "teacher", "/teacher/grade_edit", "成绩修改", active); strcat(s, h);
    nav_link(h, userid, username, "teacher", "/teacher/grade_stats", "成绩统计", active); strcat(s, h);
    nav_link(h, userid, username, "teacher", "/teacher/course_close", "课程结课", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "</div>\n");
    return s;
}

/*
 * 生成学生侧边栏HTML
 */
static char* sidebar_student(const char* username, const char* userid,
                             const char* active)
{
    char* s=(char*)malloc(4096);
    char h[512], u[256];

    if(s==NULL) return NULL;

    mkurl(u, "/student", userid, username, "student");
    sprintf(s,
        "<div class=\"sidebar\">\n"
        "  <div class=\"logo\"><a href=\"%s\">教务管理系统</a></div>\n"
        "  <div class=\"user-info\">当前用户<span>%s (学生)</span></div>\n",
        u, username);

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">个人信息管理</div>\n");
    nav_link(h, userid, username, "student", "/student/my_info", "查看个人信息", active); strcat(s, h);
    nav_link(h, userid, username, "student", "/student/edit_info", "修改个人信息", active); strcat(s, h);
    nav_link(h, userid, username, "student", "/student/change_pwd", "修改密码", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">选课中心</div>\n");
    nav_link(h, userid, username, "student", "/student/course_select", "浏览可选课程", active); strcat(s, h);
    nav_link(h, userid, username, "student", "/student/my_schedule", "我的课表", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">成绩查询与统计</div>\n");
    nav_link(h, userid, username, "student", "/student/my_grades", "已出成绩列表", active); strcat(s, h);
    nav_link(h, userid, username, "student", "/student/grade_stats", "成绩统计分析", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "<div class=\"nav-group\"><div class=\"group-title\">AI智能学习助手</div>\n");
    nav_link(h, userid, username, "student", "/student/ai_analysis", "AI成绩分析", active); strcat(s, h);
    strcat(s, "</div>\n");

    strcat(s, "</div>\n");
    return s;
}

/* ================================================================ */
/*                     对外接口                                      */
/* ================================================================ */

/*
 * 渲染页面：读取模板文件 + 替换占位符 + 注入侧边栏
 * template: 模板文件路径，如 "static/admin/home.html"
 * username: 用户名
 * userid: 用户ID
 * role: 角色(admin/teacher/student)
 * active: 当前页面路径（用于侧边栏高亮），如 "/admin/student_mgr"
 * error_msg: 错误提示信息（仅登录页使用，其他页面传NULL）
 * 返回：完整HTML字符串
 */
char* page_render(const char* template, const char* username,
                  const char* userid, const char* role,
                  const char* active, const char* error_msg)
{
    char* html;
    char* sidebar_html=NULL;
    char error_html[512]="";

    /* 读取模板文件 */
    html=read_file(template);
    if(html==NULL) {
        /* 模板文件不存在，返回404 */
        html=read_file("static/404.html");
        if(html==NULL) {
            strcpy(buf, "HTTP/1.1 500 Internal Error\r\n\r\nTemplate not found");
            return buf;
        }
        /* 404页替换占位符 */
        replace_all(html, "{{userid}}", userid ? userid : "");
        replace_all(html, "{{username}}", username ? username : "");
        replace_all(html, "{{role}}", role ? role : "");
        replace_all(html, "{{sidebar}}", "");
        return html;
    }

    /* 生成错误提示HTML（仅登录页） */
    if(error_msg!=NULL && strlen(error_msg)>0) {
        sprintf(error_html,
            "<div class=\"error-tip\">%s</div>\n  ", error_msg);
    }

    /* 根据角色生成侧边栏 */
    if(role!=NULL && strcmp(role, "admin")==0) {
        sidebar_html=sidebar_admin(username, userid, active);
    } else if(role!=NULL && strcmp(role, "teacher")==0) {
        sidebar_html=sidebar_teacher(username, userid, active);
    } else if(role!=NULL && strcmp(role, "student")==0) {
        sidebar_html=sidebar_student(username, userid, active);
    }

    /* 替换占位符 */
    replace_all(html, "{{userid}}", userid ? userid : "");
    replace_all(html, "{{username}}", username ? username : "");
    replace_all(html, "{{role}}", role ? role : "");
    replace_all(html, "{{sidebar}}", sidebar_html ? sidebar_html : "");
    replace_all(html, "{{error_msg}}", error_html);

    if(sidebar_html!=NULL) free(sidebar_html);
    return html;
}

/*
 * 登录页面（适配旧接口）
 */
char* page_login(const char* error_msg)
{
    return page_render("static/login.html", "", "", "", "", error_msg);
}

/*
 * 404页面
 */
char* page_404(void)
{
    return page_render("static/404.html", "", "", "", "", NULL);
}

/*
 * 兼容旧接口：page_login.c 导出layout_css
 * 现在CSS在HTML模板里，不需要了，但保留空实现避免链接错误
 */
const char* get_layout_css(void)
{
    return "";
}
