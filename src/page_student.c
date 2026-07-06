/*
 * page_student.c - 学生全部页面
 * 含侧边栏导航布局 + 8个子功能页面
 */

#include <stdio.h>
#include <string.h>
#include "page.h"

static char buf[65536];

static void mkurl(char* out, const char* path,
                  const char* userid, const char* username)
{
    sprintf(out, "%s?userid=%s&username=%s&role=student", path, userid, username);
}

/* 学生侧边栏 */
static void sidebar(char* out, const char* username, const char* userid,
                    const char* active)
{
    char u[256], h[512];

    mkurl(u, "/student", userid, username);
    sprintf(out,
        "<div class=\"sidebar\">\n"
        "  <div class=\"logo\"><a href=\"%s\" style=\"color:#fff;text-decoration:none;\">教务管理系统</a></div>\n"
        "  <div class=\"user-info\">当前用户<span>%s (学生)</span></div>\n",
        u, username);

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">个人信息管理</div>\n");
    mkurl(u,"/student/my_info",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>查看个人信息</a>\n",u,strcmp(active,"my_info")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/student/edit_info",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>修改个人信息</a>\n",u,strcmp(active,"edit_info")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/student/change_pwd",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>修改密码</a>\n",u,strcmp(active,"change_pwd")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">选课中心</div>\n");
    mkurl(u,"/student/course_select",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>浏览可选课程</a>\n",u,strcmp(active,"course_select")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/student/my_schedule",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>我的课表</a>\n",u,strcmp(active,"my_schedule")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">成绩查询与统计</div>\n");
    mkurl(u,"/student/my_grades",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>已出成绩列表</a>\n",u,strcmp(active,"my_grades")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/student/grade_stats",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>成绩统计分析</a>\n",u,strcmp(active,"grade_stats")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">AI智能学习助手</div>\n");
    mkurl(u,"/student/ai_analysis",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>AI成绩分析</a>\n",u,strcmp(active,"ai_analysis")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out,"</div>\n");
}

/* 页面框架 */
static void frame(char* out, const char* title, const char* username,
                  const char* userid, const char* active, const char* content)
{
    char s[4096], h[256];
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

/* 公共CSS */
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
    "  .btn-sm.primary{color:#1e3a5f;border-color:#a0b8d0;}\n"
    "  .btn-sm.primary:hover{background:#eef3f8;}\n"
    "  .btn-sm.success{color:#27ae60;border-color:#a0d8b0;}\n"
    "  .btn-sm.success:hover{background:#eafaf1;}\n"
    "  .section-title{font-size:15px;color:#1e3a5f;margin:18px 0 10px;padding-bottom:6px;border-bottom:1px solid #eef1f5;}\n"
    "  .form-inline{display:flex;gap:10px;flex-wrap:wrap;align-items:flex-end;}\n"
    "  .form-inline .field{display:flex;flex-direction:column;}\n"
    "  .form-inline .field label{font-size:12px;color:#666;margin-bottom:3px;}\n"
    "  .form-inline .field input,.form-inline .field select{padding:6px 8px;border:1px solid #ccd;border-radius:3px;font-size:13px;width:130px;}\n"
    "  .info-text{font-size:13px;color:#666;line-height:1.8;}\n"
    "  .stat-box{display:inline-block;background:#fff;border:1px solid #dde1e6;padding:16px 24px;margin:6px;text-align:center;min-width:100px;}\n"
    "  .stat-box .num{font-size:24px;color:#1e3a5f;font-weight:bold;}\n"
    "  .stat-box .lbl{font-size:12px;color:#999;margin-top:4px;}\n"
    "  .schedule-table{width:100%;border-collapse:collapse;font-size:12px;}\n"
    "  .schedule-table th{background:#1e3a5f;color:#fff;padding:8px;border:1px solid #2a4a6d;width:14.2%%;}\n"
    "  .schedule-table td{height:70px;padding:6px;border:1px solid #dde1e6;vertical-align:top;background:#fff;}\n"
    "  .schedule-table td .course-item{background:#d4e6f1;color:#1a5276;padding:3px 5px;border-radius:2px;font-size:11px;margin-bottom:2px;}\n"
    "</style>\n";

/* ================================================================ */
/*                    学生主页                                       */
/* ================================================================ */
char* page_student_home(const char* username, const char* userid)
{
    char c[4096];
    {
        char h[256], u[256];
        mkurl(h,"/student/my_info",userid,username);
        mkurl(u,"/student/edit_info",userid,username);
        {
            char p[256], s[256], sc[256];
            mkurl(p,"/student/change_pwd",userid,username);
            mkurl(s,"/student/course_select",userid,username);
            mkurl(sc,"/student/my_schedule",userid,username);
            {
                char g[256], st[256], a[256];
                mkurl(g,"/student/my_grades",userid,username);
                mkurl(st,"/student/grade_stats",userid,username);
                mkurl(a,"/student/ai_analysis",userid,username);
                sprintf(c,
                    "%s\n"
                    "<div class=\"welcome\"><h2>学生中心</h2><p>欢迎使用教务管理系统，请通过左侧导航栏选择操作功能</p></div>\n"
                    "<div class=\"card-grid\">\n"
                    "  <div class=\"card\"><h3>个人信息管理</h3><ul class=\"func-list\">\n"
                    "    <li><a href=\"%s\">查看个人信息 - 查看学号、姓名、专业等基本信息</a></li>\n"
                    "    <li><a href=\"%s\">修改个人信息 - 修改联系电话等个人资料</a></li>\n"
                    "    <li><a href=\"%s\">修改密码 - 验证原密码后设置新密码</a></li>\n"
                    "  </ul></div>\n"
                    "  <div class=\"card\"><h3>选课中心</h3><ul class=\"func-list\">\n"
                    "    <li><a href=\"%s\">浏览可选课程 - 查看本专业可选课程并选课</a></li>\n"
                    "    <li><a href=\"%s\">我的课表 - 以周课表形式查看已选课程</a></li>\n"
                    "  </ul></div>\n"
                    "  <div class=\"card\"><h3>成绩查询与统计</h3><ul class=\"func-list\">\n"
                    "    <li><a href=\"%s\">已出成绩列表 - 查看所有已结课课程的成绩</a></li>\n"
                    "    <li><a href=\"%s\">成绩统计分析 - 查看加权平均分、GPA等指标</a></li>\n"
                    "  </ul></div>\n"
                    "  <div class=\"card\"><h3>AI智能学习助手</h3><ul class=\"func-list\">\n"
                    "    <li><a href=\"%s\">AI成绩分析 - 分析成绩画像并给出学习建议</a></li>\n"
                    "  </ul></div>\n"
                    "</div>\n",
                    mgr_css, h, u, p, s, sc, g, st, a);
            }
        }
    }

    frame(buf, "学生中心", username, userid, "home", c);
    return buf;
}

/* ===== 子页面1：查看个人信息 ===== */
char* page_student_my_info(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>查看个人信息</h2></div>\n"
        "<div class=\"card\" style=\"max-width:500px;\">\n"
        "  <table class=\"data-table\">\n"
        "    <tr><th style=\"width:120px;\">学号</th><td>%s</td></tr>\n"
        "    <tr><th>姓名</th><td>%s</td></tr>\n"
        "    <tr><th>性别</th><td>男</td></tr>\n"
        "    <tr><th>年级</th><td>2024</td></tr>\n"
        "    <tr><th>专业</th><td>计算机科学与技术</td></tr>\n"
        "    <tr><th>联系电话</th><td>138****8001</td></tr>\n"
        "  </table>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "查看个人信息", username, userid, "my_info", c);
    return buf;
}

/* ===== 子页面2：修改个人信息 ===== */
char* page_student_edit_info(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>修改个人信息</h2></div>\n"
        "<div class=\"card\" style=\"max-width:450px;\">\n"
        "  <p class=\"info-text\">可修改联系电话，其他信息如需修改请联系管理员。</p>\n"
        "  <form method=\"POST\" action=\"/student/edit_info/save\" style=\"margin-top:12px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"student\">\n"
        "    <div style=\"margin-bottom:12px;\">\n"
        "      <label style=\"font-size:13px;color:#444;\">联系电话</label>\n"
        "      <input type=\"text\" name=\"phone\" value=\"13800138001\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;margin-top:4px;\">\n"
        "    </div>\n"
        "    <button type=\"submit\" style=\"padding:8px 24px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">保存修改</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "修改个人信息", username, userid, "edit_info", c);
    return buf;
}

/* ===== 子页面3：修改密码 ===== */
char* page_student_change_pwd(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>修改密码</h2></div>\n"
        "<div class=\"card\" style=\"max-width:400px;\">\n"
        "  <form method=\"POST\" action=\"/student/change_pwd/save\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"student\">\n"
        "    <div style=\"margin-bottom:12px;\">\n"
        "      <label style=\"font-size:13px;color:#444;\">原密码</label>\n"
        "      <input type=\"password\" name=\"old_pwd\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;margin-top:4px;\" required>\n"
        "    </div>\n"
        "    <div style=\"margin-bottom:12px;\">\n"
        "      <label style=\"font-size:13px;color:#444;\">新密码</label>\n"
        "      <input type=\"password\" name=\"new_pwd\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;margin-top:4px;\" required>\n"
        "    </div>\n"
        "    <div style=\"margin-bottom:12px;\">\n"
        "      <label style=\"font-size:13px;color:#444;\">确认新密码</label>\n"
        "      <input type=\"password\" name=\"confirm_pwd\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;margin-top:4px;\" required>\n"
        "    </div>\n"
        "    <button type=\"submit\" style=\"padding:8px 24px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">修改密码</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "修改密码", username, userid, "change_pwd", c);
    return buf;
}

/* ===== 子页面4：浏览可选课程 ===== */
char* page_student_course_select(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>浏览可选课程</h2></div>\n"
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/student/course_select\" style=\"display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"student\">\n"
        "    <select name=\"type\"><option value=\"\">全部类型</option><option>必修</option><option>选修</option></select>\n"
        "    <input type=\"text\" name=\"keyword\" placeholder=\"课程名搜索\">\n"
        "    <button type=\"submit\">筛选</button>\n"
        "  </form>\n"
        "</div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>课程号</th><th>课程名称</th><th>授课教师</th><th>学分</th><th>类型</th><th>上课时间</th><th>已选/上限</th><th>操作</th></tr>\n"
        "  <tr><td>CS101</td><td>数据结构与算法</td><td>陈教授</td><td>4.0</td><td>必修</td><td>周二 第3-4节</td><td>58/60</td>\n"
        "    <td><button class=\"btn-sm success\">选课</button></td></tr>\n"
        "  <tr><td>CS201</td><td>操作系统</td><td>刘副教授</td><td>3.0</td><td>必修</td><td>周三 第1-2节</td><td>42/50</td>\n"
        "    <td><button class=\"btn-sm success\">选课</button></td></tr>\n"
        "  <tr><td>CS301</td><td>计算机网络</td><td>王教授</td><td>2.5</td><td>选修</td><td>周四 第5-6节</td><td>30/40</td>\n"
        "    <td><button class=\"btn-sm success\">选课</button></td></tr>\n"
        "  <tr><td>MATH101</td><td>高等数学</td><td>周教授</td><td>5.0</td><td>必修</td><td>周一 第1-2节</td><td>75/80</td>\n"
        "    <td><span style=\"color:#999;\">时间冲突</span></td></tr>\n"
        "  <tr><td>ENG101</td><td>大学英语</td><td>孙教授</td><td>3.0</td><td>必修</td><td>周五 第3-4节</td><td>120/120</td>\n"
        "    <td><span style=\"color:#e67e22;\">已满</span></td></tr>\n"
        "</table>\n"
        "<p class=\"info-text\" style=\"margin-top:10px;\">专业限制：仅显示本专业（计算机科学与技术）可选课程。</p>\n",
        mgr_css, userid, username);

    frame(buf, "浏览可选课程", username, userid, "course_select", c);
    return buf;
}

/* ===== 子页面5：我的课表 ===== */
char* page_student_my_schedule(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>我的课表</h2></div>\n"
        "<table class=\"schedule-table\">\n"
        "  <tr><th>节次</th><th>周一</th><th>周二</th><th>周三</th><th>周四</th><th>周五</th></tr>\n"
        "  <tr><td>第1-2节<br>08:15-09:55</td>\n"
        "    <td><div class=\"course-item\">高等数学<br>MATH101<br>周教授<br>1-16周</div></td>\n"
        "    <td></td>\n"
        "    <td><div class=\"course-item\">操作系统<br>CS201<br>刘副教授<br>1-16周</div></td>\n"
        "    <td></td>\n"
        "    <td></td></tr>\n"
        "  <tr><td>第3-4节<br>10:15-11:55</td>\n"
        "    <td></td>\n"
        "    <td><div class=\"course-item\">数据结构<br>CS101<br>陈教授<br>1-16周</div></td>\n"
        "    <td></td>\n"
        "    <td></td>\n"
        "    <td><div class=\"course-item\">大学英语<br>ENG101<br>孙教授<br>1-16周</div></td></tr>\n"
        "  <tr><td>第5-7节<br>13:50-16:25</td>\n"
        "    <td></td>\n"
        "    <td></td>\n"
        "    <td></td>\n"
        "    <td><div class=\"course-item\">计算机网络<br>CS301<br>王教授<br>1-12周</div></td>\n"
        "    <td></td></tr>\n"
        "  <tr><td>第8-9节<br>16:45-18:25</td><td></td><td></td><td></td><td></td><td></td></tr>\n"
        "  <tr><td>第10-12节<br>19:20-21:55</td><td></td><td></td><td></td><td></td><td></td></tr>\n"
        "</table>\n",
        mgr_css);

    frame(buf, "我的课表", username, userid, "my_schedule", c);
    return buf;
}

/* ===== 子页面6：已出成绩列表 ===== */
char* page_student_my_grades(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>已出成绩列表</h2></div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>课程号</th><th>课程名称</th><th>学分</th><th>课程类型</th><th>平时成绩</th><th>期末成绩</th><th>总评成绩</th><th>绩点</th><th>操作</th></tr>\n"
        "  <tr><td>CS101</td><td>数据结构与算法</td><td>4.0</td><td>必修</td><td>85</td><td>90</td><td>88.5</td><td>3.7</td>\n"
        "    <td><button class=\"btn-sm primary\">详情</button></td></tr>\n"
        "  <tr><td>MATH101</td><td>高等数学</td><td>5.0</td><td>必修</td><td>92</td><td>88</td><td>89.2</td><td>3.7</td>\n"
        "    <td><button class=\"btn-sm primary\">详情</button></td></tr>\n"
        "  <tr><td>ENG101</td><td>大学英语</td><td>3.0</td><td>必修</td><td>78</td><td>82</td><td>80.8</td><td>3.0</td>\n"
        "    <td><button class=\"btn-sm primary\">详情</button></td></tr>\n"
        "  <tr><td>CS301</td><td>计算机网络</td><td>2.5</td><td>选修</td><td>75</td><td>70</td><td>71.5</td><td>2.0</td>\n"
        "    <td><button class=\"btn-sm primary\">详情</button></td></tr>\n"
        "</table>\n",
        mgr_css);

    frame(buf, "已出成绩列表", username, userid, "my_grades", c);
    return buf;
}

/* ===== 子页面7：成绩统计分析 ===== */
char* page_student_grade_stats(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>成绩统计分析</h2></div>\n"
        "<div style=\"margin-bottom:16px;\">\n"
        "  <div class=\"stat-box\"><div class=\"num\">84.5</div><div class=\"lbl\">加权平均成绩</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">3.33</div><div class=\"lbl\">GPA（加权绩点）</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">86.1</div><div class=\"lbl\">必修加权平均</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">3.47</div><div class=\"lbl\">必修加权绩点</div></div>\n"
        "</div>\n"
        "<div class=\"card\">\n"
        "  <h3>计算说明</h3>\n"
        "  <div class=\"info-text\">\n"
        "    <p>加权平均成绩 = (88.5x4.0 + 89.2x5.0 + 80.8x3.0 + 71.5x2.5) / (4.0+5.0+3.0+2.5) = 84.5</p>\n"
        "    <p>GPA = (3.7x4.0 + 3.7x5.0 + 3.0x3.0 + 2.0x2.5) / 14.5 = 3.33</p>\n"
        "    <p>必修加权平均 = (88.5x4.0 + 89.2x5.0 + 80.8x3.0) / 12.0 = 86.1</p>\n"
        "    <p>已修总学分：14.5（其中必修12.0，选修2.5）</p>\n"
        "  </div>\n"
        "</div>\n",
        mgr_css);

    frame(buf, "成绩统计分析", username, userid, "grade_stats", c);
    return buf;
}

/* ===== 子页面8：AI成绩分析 ===== */
char* page_student_ai_analysis(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>AI成绩分析</h2></div>\n"
        "<div class=\"card\">\n"
        "  <p class=\"info-text\">点击下方按钮，系统将拉取您的全部已出成绩数据，通过AI大模型进行分析，给出个性化的学习建议。</p>\n"
        "  <form method=\"POST\" action=\"/student/ai_analysis/run\" style=\"margin-top:14px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"student\">\n"
        "    <button type=\"submit\" style=\"padding:10px 30px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">开始AI成绩分析</button>\n"
        "  </form>\n"
        "</div>\n"
        "<div class=\"card\" style=\"margin-top:16px;\">\n"
        "  <h3>分析结果示例</h3>\n"
        "  <div class=\"info-text\" style=\"background:#f8f9fa;padding:14px;border-radius:3px;\">\n"
        "    <p><strong>成绩画像：</strong>您的加权平均分84.5，GPA 3.33，处于中等偏上水平。数据结构与高等数学表现突出。</p>\n"
        "    <p><strong>优势学科：</strong>高等数学(89.2)、数据结构与算法(88.5) - 逻辑思维和算法能力较强。</p>\n"
        "    <p><strong>薄弱环节：</strong>计算机网络(71.5)、大学英语(80.8) - 建议加强网络原理理解，多做实验；英语需增加阅读量。</p>\n"
        "    <p><strong>学习建议：</strong>1.保持数学和算法优势 2.计算机类课程多动手实践 3.英语坚持每日背单词30分钟。</p>\n"
        "  </div>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "AI成绩分析", username, userid, "ai_analysis", c);
    return buf;
}
