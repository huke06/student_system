/*
 * page_admin.c - 管理员全部页面
 * 含侧边栏导航布局 + 11个子功能页面
 */

#include <stdio.h>
#include <string.h>
#include "page.h"

static char buf[65536];

/* 生成带参数的URL */
static void mkurl(char* out, const char* path,
                  const char* userid, const char* username)
{
    sprintf(out, "%s?userid=%s&username=%s&role=admin", path, userid, username);
}

/* 生成管理员侧边栏 */
static void sidebar(char* out, const char* username, const char* userid,
                    const char* active)
{
    char u[256];
    char h[512];

    mkurl(u, "/admin", userid, username);

    sprintf(out,
        "<div class=\"sidebar\">\n"
        "  <div class=\"logo\"><a href=\"%s\" style=\"color:#fff;text-decoration:none;\">教务管理系统</a></div>\n"
        "  <div class=\"user-info\">当前用户<span>%s (管理员)</span></div>\n",
        u, username);

    /* 导航组 */
    #define NL(path,label) mkurl(u,path,userid,username); \
        sprintf(h,"%s%s%s",strcmp(active,#path)==0?" active":"",u,label); \
        strcat(out,h);

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">账号信息管理</div>\n");
    mkurl(u,"/admin/student_mgr",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>学生账号管理</a>\n",u,strcmp(active,"student_mgr")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/admin/teacher_mgr",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>教师账号管理</a>\n",u,strcmp(active,"teacher_mgr")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/admin/admin_mgr",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>管理员账号管理</a>\n",u,strcmp(active,"admin_mgr")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">选课周期配置</div>\n");
    mkurl(u,"/admin/select_config",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>选课时间设置</a>\n",u,strcmp(active,"select_config")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/admin/select_force",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>强制开关控制</a>\n",u,strcmp(active,"select_force")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/admin/select_monitor",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>选课状态监控</a>\n",u,strcmp(active,"select_monitor")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">课程全局管理</div>\n");
    mkurl(u,"/admin/course_list",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>查看所有课程</a>\n",u,strcmp(active,"course_list")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/admin/course_edit",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>修改课程信息</a>\n",u,strcmp(active,"course_edit")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/admin/course_delete",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>删除课程</a>\n",u,strcmp(active,"course_delete")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">系统基础配置</div>\n");
    mkurl(u,"/admin/sys_semester",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>学期日期设置</a>\n",u,strcmp(active,"sys_semester")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/admin/sys_majors",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>专业列表维护</a>\n",u,strcmp(active,"sys_majors")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out,"</div>\n");
    #undef NL
}

/* 生成完整页面框架 */
static void frame(char* out, const char* title, const char* username,
                  const char* userid, const char* active, const char* content)
{
    char s[4096];
    char h[256];
    mkurl(h, "/logout", userid, username);

    sidebar(s, username, userid, active);

    sprintf(out,
        "<!DOCTYPE html>\n<html lang=\"zh-CN\">\n<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">\n"
        "  <title>%s - 教务管理系统</title>\n"
        "  %s\n"
        "</head>\n<body>\n"
        "%s\n"
        "  <div class=\"main-area\">\n"
        "    <div class=\"top-bar\">\n"
        "      <a href=\"%s\">退出登录</a>\n"
        "    </div>\n"
        "    <div class=\"content\">\n"
        "%s\n"
        "    </div>\n"
        "    <div class=\"footer\">教务管理系统 v1.0</div>\n"
        "  </div>\n"
        "</body>\n</html>\n",
        title, get_layout_css(), s, h, content);
}

/* ===== 增删改查通用CSS ===== */
static const char* mgr_css=
    "<style>\n"
    "  .toolbar{margin-bottom:16px;display:flex;gap:10px;flex-wrap:wrap;align-items:center;}\n"
    "  .toolbar input,.toolbar select{padding:7px 10px;border:1px solid #ccd;border-radius:3px;font-size:13px;}\n"
    "  .toolbar button{padding:7px 16px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;font-size:13px;cursor:pointer;}\n"
    "  .toolbar button:hover{background:#2a5280;}\n"
    "  table.data-table{width:100%;border-collapse:collapse;font-size:13px;}\n"
    "  table.data-table th{background:#f5f6f8;color:#333;padding:10px 8px;border:1px solid #dde1e6;text-align:left;font-weight:bold;}\n"
    "  table.data-table td{padding:8px;border:1px solid #dde1e6;color:#444;}\n"
    "  table.data-table tr:hover{background:#f9fafb;}\n"
    "  .btn-sm{padding:4px 10px;font-size:12px;border:1px solid #ccd;border-radius:2px;cursor:pointer;background:#fff;color:#333;margin:0 2px;}\n"
    "  .btn-sm.danger{color:#c0392b;border-color:#e0b4b4;}\n"
    "  .btn-sm.danger:hover{background:#fef0f0;}\n"
    "  .btn-sm.primary{color:#1e3a5f;border-color:#a0b8d0;}\n"
    "  .btn-sm.primary:hover{background:#eef3f8;}\n"
    "  .section-title{font-size:15px;color:#1e3a5f;margin:18px 0 10px;padding-bottom:6px;border-bottom:1px solid #eef1f5;}\n"
    "  .form-inline{display:flex;gap:10px;flex-wrap:wrap;align-items:flex-end;}\n"
    "  .form-inline .field{display:flex;flex-direction:column;}\n"
    "  .form-inline .field label{font-size:12px;color:#666;margin-bottom:3px;}\n"
    "  .form-inline .field input,.form-inline .field select{padding:6px 8px;border:1px solid #ccd;border-radius:3px;font-size:13px;width:130px;}\n"
    "  .info-text{font-size:13px;color:#666;line-height:1.8;}\n"
    "  .stat-box{display:inline-block;background:#fff;border:1px solid #dde1e6;padding:16px 24px;margin:6px;text-align:center;min-width:100px;}\n"
    "  .stat-box .num{font-size:24px;color:#1e3a5f;font-weight:bold;}\n"
    "  .stat-box .lbl{font-size:12px;color:#999;margin-top:4px;}\n"
    "</style>\n";

/* ================================================================ */
/*                    管理员主页                                     */
/* ================================================================ */
char* page_admin_home(const char* username, const char* userid)
{
    char content[4096];
    char h[256], u[256];

    mkurl(h,"/admin/student_mgr",userid,username);
    mkurl(u,"/admin/teacher_mgr",userid,username);
    sprintf(content,
        "%s\n"
        "<div class=\"welcome\"><h2>管理员工作台</h2><p>欢迎使用教务管理系统，请通过左侧导航栏选择操作功能</p></div>\n"
        "<div class=\"card-grid\">\n"
        "  <div class=\"card\"><h3>账号信息管理</h3><ul class=\"func-list\">\n"
        "    <li><a href=\"%s\">学生账号管理 - 新增、修改、删除、查询学生信息</a></li>\n"
        "    <li><a href=\"%s\">教师账号管理 - 新增、修改、删除、查询教师信息</a></li>\n"
        "    <li><a href=\"%s\">管理员账号管理 - 管理次级管理员账号</a></li>\n"
        "  </ul></div>\n",
        mgr_css, h, u,
        ({mkurl(u,"/admin/admin_mgr",userid,username);u;}));

    mkurl(h,"/admin/select_config",userid,username);
    mkurl(u,"/admin/select_force",userid,username);
    {
        char t[256], s[256];
        mkurl(t,"/admin/select_monitor",userid,username);
        mkurl(s,"/admin/course_list",userid,username);
        sprintf(content+strlen(content),
        "  <div class=\"card\"><h3>选课周期配置</h3><ul class=\"func-list\">\n"
        "    <li><a href=\"%s\">选课时间设置 - 设定选课开启与关闭时间</a></li>\n"
        "    <li><a href=\"%s\">强制开关控制 - 手动强制开启/关闭选课</a></li>\n"
        "    <li><a href=\"%s\">选课状态监控 - 查看选课完成率统计</a></li>\n"
        "  </ul></div>\n"
        "  <div class=\"card\"><h3>课程全局管理</h3><ul class=\"func-list\">\n"
        "    <li><a href=\"%s\">查看所有课程 - 按条件筛选课程列表</a></li>\n"
        "    <li><a href=\"%s\">修改课程信息 - 调整课程各项属性</a></li>\n"
        "    <li><a href=\"%s\">删除课程 - 删除课程及关联数据</a></li>\n"
        "  </ul></div>\n",
        h, u, t, s,
        ({mkurl(u,"/admin/course_edit",userid,username);u;}),
        ({mkurl(u,"/admin/course_delete",userid,username);u;}));
    }

    {
        char h2[256], u2[256];
        mkurl(h2,"/admin/sys_semester",userid,username);
        mkurl(u2,"/admin/sys_majors",userid,username);
        sprintf(content+strlen(content),
        "  <div class=\"card\"><h3>系统基础配置</h3><ul class=\"func-list\">\n"
        "    <li><a href=\"%s\">学期日期设置 - 设定开学日期自动推算教学周</a></li>\n"
        "    <li><a href=\"%s\">专业列表维护 - 维护全校专业列表数据</a></li>\n"
        "  </ul></div>\n"
        "</div>\n",
        h2, u2);
    }

    frame(buf, "管理员工作台", username, userid, "home", content);
    return buf;
}

/* ================================================================ */
/*               子页面1：学生账号管理                                */
/* ================================================================ */
char* page_admin_student_mgr(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>学生账号管理</h2></div>\n"
        /* 搜索栏 */
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/admin/student_mgr\" style=\"display:flex;gap:8px;flex-wrap:wrap;align-items:center;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <input type=\"text\" name=\"keyword\" placeholder=\"学号或姓名\">\n"
        "    <select name=\"grade\"><option value=\"\">全部年级</option><option>2024</option><option>2023</option><option>2022</option><option>2021</option></select>\n"
        "    <select name=\"major\"><option value=\"\">全部专业</option><option>计算机科学与技术</option><option>软件工程</option><option>数学与应用数学</option></select>\n"
        "    <button type=\"submit\">查询</button>\n"
        "  </form>\n"
        "</div>\n"
        /* 学生列表表格（示例数据） */
        "<table class=\"data-table\">\n"
        "  <tr><th>学号</th><th>姓名</th><th>性别</th><th>年级</th><th>专业</th><th>联系电话</th><th>操作</th></tr>\n"
        "  <tr><td>2024010001</td><td>张三</td><td>男</td><td>2024</td><td>计算机科学与技术</td><td>13800138001</td>\n"
        "    <td><button class=\"btn-sm primary\">修改</button><button class=\"btn-sm danger\">删除</button><button class=\"btn-sm\">重置密码</button></td></tr>\n"
        "  <tr><td>2024010002</td><td>李四</td><td>女</td><td>2024</td><td>软件工程</td><td>13800138002</td>\n"
        "    <td><button class=\"btn-sm primary\">修改</button><button class=\"btn-sm danger\">删除</button><button class=\"btn-sm\">重置密码</button></td></tr>\n"
        "  <tr><td>2023010001</td><td>王五</td><td>男</td><td>2023</td><td>数学与应用数学</td><td>13800138003</td>\n"
        "    <td><button class=\"btn-sm primary\">修改</button><button class=\"btn-sm danger\">删除</button><button class=\"btn-sm\">重置密码</button></td></tr>\n"
        "</table>\n"
        /* 新增学生表单 */
        "<div class=\"section-title\">新增学生</div>\n"
        "<form method=\"POST\" action=\"/admin/student_mgr/add\">\n"
        "  <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "  <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "  <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "  <div class=\"form-inline\">\n"
        "    <div class=\"field\"><label>学号</label><input type=\"text\" name=\"sid\" placeholder=\"10位数字\" required></div>\n"
        "    <div class=\"field\"><label>姓名</label><input type=\"text\" name=\"name\" required></div>\n"
        "    <div class=\"field\"><label>性别</label><select name=\"gender\"><option>男</option><option>女</option></select></div>\n"
        "    <div class=\"field\"><label>年级</label><input type=\"text\" name=\"grade\" placeholder=\"如2024\" required></div>\n"
        "    <div class=\"field\"><label>专业</label><input type=\"text\" name=\"major\" required></div>\n"
        "    <div class=\"field\"><label>联系电话</label><input type=\"text\" name=\"phone\"></div>\n"
        "    <div class=\"field\"><label>&nbsp;</label><button type=\"submit\">添加</button></div>\n"
        "  </div>\n"
        "</form>\n",
        mgr_css, userid, username, userid, username);

    frame(buf, "学生账号管理", username, userid, "student_mgr", c);
    return buf;
}

/* ================================================================ */
/*               子页面2：教师账号管理                                */
/* ================================================================ */
char* page_admin_teacher_mgr(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>教师账号管理</h2></div>\n"
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/admin/teacher_mgr\" style=\"display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <input type=\"text\" name=\"keyword\" placeholder=\"工号或姓名\">\n"
        "    <select name=\"dept\"><option value=\"\">全部院系</option><option>计算机学院</option><option>数学学院</option><option>物理学院</option></select>\n"
        "    <button type=\"submit\">查询</button>\n"
        "  </form>\n"
        "</div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>工号</th><th>姓名</th><th>性别</th><th>所属院系</th><th>联系电话</th><th>操作</th></tr>\n"
        "  <tr><td>100001</td><td>陈教授</td><td>男</td><td>计算机学院</td><td>13900139001</td>\n"
        "    <td><button class=\"btn-sm primary\">修改</button><button class=\"btn-sm danger\">删除</button><button class=\"btn-sm\">重置密码</button></td></tr>\n"
        "  <tr><td>100002</td><td>刘副教授</td><td>女</td><td>数学学院</td><td>13900139002</td>\n"
        "    <td><button class=\"btn-sm primary\">修改</button><button class=\"btn-sm danger\">删除</button><button class=\"btn-sm\">重置密码</button></td></tr>\n"
        "</table>\n"
        "<div class=\"section-title\">新增教师</div>\n"
        "<form method=\"POST\" action=\"/admin/teacher_mgr/add\">\n"
        "  <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "  <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "  <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "  <div class=\"form-inline\">\n"
        "    <div class=\"field\"><label>工号</label><input type=\"text\" name=\"tid\" placeholder=\"6位数字\" required></div>\n"
        "    <div class=\"field\"><label>姓名</label><input type=\"text\" name=\"name\" required></div>\n"
        "    <div class=\"field\"><label>性别</label><select name=\"gender\"><option>男</option><option>女</option></select></div>\n"
        "    <div class=\"field\"><label>所属院系</label><input type=\"text\" name=\"dept\" required></div>\n"
        "    <div class=\"field\"><label>联系电话</label><input type=\"text\" name=\"phone\"></div>\n"
        "    <div class=\"field\"><label>&nbsp;</label><button type=\"submit\">添加</button></div>\n"
        "  </div>\n"
        "</form>\n",
        mgr_css, userid, username, userid, username);

    frame(buf, "教师账号管理", username, userid, "teacher_mgr", c);
    return buf;
}

/* ================================================================ */
/*               子页面3：管理员账号管理                              */
/* ================================================================ */
char* page_admin_admin_mgr(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>管理员账号管理</h2></div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>管理员账号</th><th>姓名</th><th>操作</th></tr>\n"
        "  <tr><td>admin</td><td>%s</td><td><span style=\"color:#999;\">超级管理员</span></td></tr>\n"
        "  <tr><td>admin02</td><td>赵副管理</td>\n"
        "    <td><button class=\"btn-sm primary\">修改</button><button class=\"btn-sm danger\">删除</button><button class=\"btn-sm\">重置密码</button></td></tr>\n"
        "</table>\n"
        "<div class=\"section-title\">新增次级管理员</div>\n"
        "<form method=\"POST\" action=\"/admin/admin_mgr/add\">\n"
        "  <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "  <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "  <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "  <div class=\"form-inline\">\n"
        "    <div class=\"field\"><label>管理员账号</label><input type=\"text\" name=\"aid\" required></div>\n"
        "    <div class=\"field\"><label>姓名</label><input type=\"text\" name=\"name\" required></div>\n"
        "    <div class=\"field\"><label>&nbsp;</label><button type=\"submit\">添加</button></div>\n"
        "  </div>\n"
        "</form>\n",
        mgr_css, username, userid, username);

    frame(buf, "管理员账号管理", username, userid, "admin_mgr", c);
    return buf;
}

/* ================================================================ */
/*               子页面4：选课时间设置                                */
/* ================================================================ */
char* page_admin_select_config(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>选课时间设置</h2></div>\n"
        "<div class=\"card\" style=\"max-width:500px;\">\n"
        "  <p class=\"info-text\">设置选课开放时间段，系统将根据此时间自动判断选课状态。</p>\n"
        "  <form method=\"POST\" action=\"/admin/select_config/save\" style=\"margin-top:14px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <div class=\"form-row\" style=\"margin-bottom:12px;\">\n"
        "      <label>选课开启时间</label>\n"
        "      <input type=\"datetime-local\" name=\"select_start\" value=\"2026-02-24T08:00\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;\">\n"
        "    </div>\n"
        "    <div class=\"form-row\" style=\"margin-bottom:12px;\">\n"
        "      <label>选课关闭时间</label>\n"
        "      <input type=\"datetime-local\" name=\"select_end\" value=\"2026-03-07T23:59\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;\">\n"
        "    </div>\n"
        "    <button type=\"submit\" style=\"padding:8px 24px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">保存设置</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "选课时间设置", username, userid, "select_config", c);
    return buf;
}

/* ================================================================ */
/*               子页面5：强制开关控制                                */
/* ================================================================ */
char* page_admin_select_force(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>强制开关控制</h2></div>\n"
        "<div class=\"card\" style=\"max-width:600px;\">\n"
        "  <p class=\"info-text\">当前选课状态：<strong style=\"color:#27ae60;\">选课开启中</strong>（自动模式）</p>\n"
        "  <p class=\"info-text\" style=\"margin-top:8px;\">可通过以下按钮强制覆盖自动时间配置：</p>\n"
        "  <div style=\"margin-top:14px;display:flex;gap:10px;\">\n"
        "    <form method=\"POST\" action=\"/admin/select_force/set\" style=\"display:inline;\">\n"
        "      <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "      <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "      <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "      <input type=\"hidden\" name=\"action\" value=\"1\">\n"
        "      <button type=\"submit\" style=\"padding:10px 24px;background:#27ae60;color:#fff;border:none;border-radius:3px;cursor:pointer;\">强制开启选课</button>\n"
        "    </form>\n"
        "    <form method=\"POST\" action=\"/admin/select_force/set\" style=\"display:inline;\">\n"
        "      <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "      <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "      <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "      <input type=\"hidden\" name=\"action\" value=\"2\">\n"
        "      <button type=\"submit\" style=\"padding:10px 24px;background:#c0392b;color:#fff;border:none;border-radius:3px;cursor:pointer;\">强制关闭选课</button>\n"
        "    </form>\n"
        "    <form method=\"POST\" action=\"/admin/select_force/set\" style=\"display:inline;\">\n"
        "      <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "      <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "      <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "      <input type=\"hidden\" name=\"action\" value=\"0\">\n"
        "      <button type=\"submit\" style=\"padding:10px 24px;background:#7f8c8d;color:#fff;border:none;border-radius:3px;cursor:pointer;\">恢复自动模式</button>\n"
        "    </form>\n"
        "  </div>\n"
        "</div>\n",
        mgr_css, userid, username, userid, username, userid, username);

    frame(buf, "强制开关控制", username, userid, "select_force", c);
    return buf;
}

/* ================================================================ */
/*               子页面6：选课状态监控                                */
/* ================================================================ */
char* page_admin_select_monitor(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>选课状态监控</h2></div>\n"
        "<div style=\"margin-bottom:16px;\">\n"
        "  <div class=\"stat-box\"><div class=\"num\">48</div><div class=\"lbl\">课程总数</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">35</div><div class=\"lbl\">选课进行中</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">85.2%%</div><div class=\"lbl\">平均选课率</div></div>\n"
        "</div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>课程号</th><th>课程名称</th><th>授课教师</th><th>已选人数</th><th>人数上限</th><th>完成率</th><th>状态</th></tr>\n"
        "  <tr><td>CS101</td><td>数据结构与算法</td><td>陈教授</td><td>58</td><td>60</td><td>96.7%%</td><td style=\"color:#e67e22;\">即将满员</td></tr>\n"
        "  <tr><td>CS201</td><td>操作系统</td><td>刘副教授</td><td>42</td><td>50</td><td>84.0%%</td><td style=\"color:#27ae60;\">正常</td></tr>\n"
        "  <tr><td>MATH101</td><td>高等数学</td><td>周教授</td><td>75</td><td>80</td><td>93.8%%</td><td style=\"color:#27ae60;\">正常</td></tr>\n"
        "  <tr><td>PHY101</td><td>大学物理</td><td>吴副教授</td><td>20</td><td>40</td><td>50.0%%</td><td style=\"color:#999;\">偏低</td></tr>\n"
        "</table>\n",
        mgr_css);

    frame(buf, "选课状态监控", username, userid, "select_monitor", c);
    return buf;
}

/* ================================================================ */
/*               子页面7：查看所有课程                                */
/* ================================================================ */
char* page_admin_course_list(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>查看所有课程</h2></div>\n"
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/admin/course_list\" style=\"display:flex;gap:8px;flex-wrap:wrap;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <select name=\"type\"><option value=\"\">全部类型</option><option>必修</option><option>选修</option></select>\n"
        "    <select name=\"teacher\"><option value=\"\">全部教师</option><option>陈教授</option><option>刘副教授</option></select>\n"
        "    <input type=\"text\" name=\"keyword\" placeholder=\"课程号或课程名\">\n"
        "    <button type=\"submit\">筛选</button>\n"
        "  </form>\n"
        "</div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>课程号</th><th>课程名称</th><th>类型</th><th>学分</th><th>授课教师</th><th>适用专业</th><th>已选/上限</th><th>状态</th></tr>\n"
        "  <tr><td>CS101</td><td>数据结构与算法</td><td>必修</td><td>4.0</td><td>陈教授</td><td>计算机科学与技术,软件工程</td><td>58/60</td><td>选课中</td></tr>\n"
        "  <tr><td>CS201</td><td>操作系统</td><td>必修</td><td>3.0</td><td>刘副教授</td><td>计算机科学与技术</td><td>42/50</td><td>选课中</td></tr>\n"
        "  <tr><td>MATH101</td><td>高等数学</td><td>必修</td><td>5.0</td><td>周教授</td><td>全部专业</td><td>75/80</td><td>选课中</td></tr>\n"
        "  <tr><td>ENG101</td><td>大学英语</td><td>必修</td><td>3.0</td><td>孙教授</td><td>全部专业</td><td>120/150</td><td>选课中</td></tr>\n"
        "</table>\n",
        mgr_css, userid, username);

    frame(buf, "查看所有课程", username, userid, "course_list", c);
    return buf;
}

/* ================================================================ */
/*               子页面8：修改课程信息                                */
/* ================================================================ */
char* page_admin_course_edit(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>修改课程信息</h2></div>\n"
        "<div class=\"card\" style=\"max-width:700px;\">\n"
        "  <p class=\"info-text\">请输入要修改的课程号，查询后可编辑课程信息。</p>\n"
        "  <form method=\"GET\" action=\"/admin/course_edit\" style=\"margin:12px 0;display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <input type=\"text\" name=\"cid\" placeholder=\"输入课程号\" style=\"padding:7px;border:1px solid #ccd;border-radius:3px;width:160px;\">\n"
        "    <button type=\"submit\" style=\"padding:7px 16px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">查询</button>\n"
        "  </form>\n"
        "  <div class=\"section-title\">课程信息编辑</div>\n"
        "  <form method=\"POST\" action=\"/admin/course_edit/save\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <div class=\"form-inline\" style=\"flex-direction:column;gap:8px;\">\n"
        "      <div class=\"field\"><label>课程号</label><input type=\"text\" value=\"CS101\" style=\"width:200px;\" readonly></div>\n"
        "      <div class=\"field\"><label>课程名称</label><input type=\"text\" value=\"数据结构与算法\" style=\"width:300px;\"></div>\n"
        "      <div class=\"field\"><label>课程类型</label><select><option>必修</option><option>选修</option></select></div>\n"
        "      <div class=\"field\"><label>学分</label><input type=\"text\" value=\"4.0\" style=\"width:80px;\"></div>\n"
        "      <div class=\"field\"><label>人数上限</label><input type=\"text\" value=\"60\" style=\"width:80px;\"></div>\n"
        "    </div>\n"
        "    <button type=\"submit\" style=\"margin-top:12px;padding:8px 24px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">保存修改</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username, userid, username);

    frame(buf, "修改课程信息", username, userid, "course_edit", c);
    return buf;
}

/* ================================================================ */
/*               子页面9：删除课程                                    */
/* ================================================================ */
char* page_admin_course_delete(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>删除课程</h2></div>\n"
        "<div class=\"card\" style=\"max-width:600px;\">\n"
        "  <p class=\"info-text\">输入课程号查询课程，确认无误后方可删除。已有学生选课的课程需先清退学生。</p>\n"
        "  <form method=\"GET\" action=\"/admin/course_delete\" style=\"margin:12px 0;display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <input type=\"text\" name=\"cid\" placeholder=\"输入课程号\" style=\"padding:7px;border:1px solid #ccd;border-radius:3px;width:160px;\">\n"
        "    <button type=\"submit\" style=\"padding:7px 16px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">查询</button>\n"
        "  </form>\n"
        "  <div style=\"background:#fef0f0;padding:12px;border:1px solid #f5c6cb;border-radius:3px;margin-top:12px;\">\n"
        "    <p class=\"info-text\" style=\"color:#c0392b;\"><strong>注意：</strong>删除操作不可恢复！请确认选课人数为0后再执行删除。</p>\n"
        "    <button style=\"margin-top:8px;padding:8px 24px;background:#c0392b;color:#fff;border:none;border-radius:3px;cursor:pointer;\">确认删除</button>\n"
        "  </div>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "删除课程", username, userid, "course_delete", c);
    return buf;
}

/* ================================================================ */
/*               子页面10：学期日期设置                               */
/* ================================================================ */
char* page_admin_sys_semester(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>学期日期设置</h2></div>\n"
        "<div class=\"card\" style=\"max-width:500px;\">\n"
        "  <p class=\"info-text\">设置学期开学日期（第一周周一），系统将自动推算全学期20个教学周的起止日期。</p>\n"
        "  <form method=\"POST\" action=\"/admin/sys_semester/save\" style=\"margin-top:14px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <div class=\"form-row\" style=\"margin-bottom:12px;\">\n"
        "      <label>开学日期（第一周周一）</label>\n"
        "      <input type=\"date\" name=\"semester_start\" value=\"2026-02-23\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;font-size:14px;\">\n"
        "    </div>\n"
        "    <p class=\"info-text\">当前设置：2026年2月23日开学，共20个教学周，预计第20周结束日期为2026年7月12日。</p>\n"
        "    <button type=\"submit\" style=\"margin-top:10px;padding:8px 24px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">保存设置</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "学期日期设置", username, userid, "sys_semester", c);
    return buf;
}

/* ================================================================ */
/*               子页面11：专业列表维护                               */
/* ================================================================ */
char* page_admin_sys_majors(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>专业列表维护</h2></div>\n"
        "<div class=\"card\" style=\"max-width:600px;\">\n"
        "  <table class=\"data-table\">\n"
        "    <tr><th>序号</th><th>专业名称</th><th>操作</th></tr>\n"
        "    <tr><td>1</td><td>计算机科学与技术</td><td><button class=\"btn-sm danger\">删除</button></td></tr>\n"
        "    <tr><td>2</td><td>软件工程</td><td><button class=\"btn-sm danger\">删除</button></td></tr>\n"
        "    <tr><td>3</td><td>数学与应用数学</td><td><button class=\"btn-sm danger\">删除</button></td></tr>\n"
        "    <tr><td>4</td><td>信息与计算科学</td><td><button class=\"btn-sm danger\">删除</button></td></tr>\n"
        "    <tr><td>5</td><td>物理学</td><td><button class=\"btn-sm danger\">删除</button></td></tr>\n"
        "  </table>\n"
        "  <div class=\"section-title\">添加专业</div>\n"
        "  <form method=\"POST\" action=\"/admin/sys_majors/add\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"username\" value=\"%s\">\n"
        "    <input type=\"hidden\" name=\"role\" value=\"admin\">\n"
        "    <div style=\"display:flex;gap:8px;align-items:flex-end;\">\n"
        "      <div class=\"field\"><label>专业名称</label><input type=\"text\" name=\"major\" style=\"width:220px;\" required></div>\n"
        "      <button type=\"submit\" style=\"padding:7px 16px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">添加</button>\n"
        "    </div>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "专业列表维护", username, userid, "sys_majors", c);
    return buf;
}
