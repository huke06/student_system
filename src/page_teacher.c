/*
 * page_teacher.c - 教师全部页面
 * 含侧边栏导航布局 + 8个子功能页面
 */

#include <stdio.h>
#include <string.h>
#include "page.h"

static char buf[65536];

static void mkurl(char* out, const char* path,
                  const char* userid, const char* username)
{
    sprintf(out, "%s?userid=%s&username=%s&role=teacher", path, userid, username);
}

/* 教师侧边栏 */
static void sidebar(char* out, const char* username, const char* userid,
                    const char* active)
{
    char u[256], h[512];

    mkurl(u, "/teacher", userid, username);
    sprintf(out,
        "<div class=\"sidebar\">\n"
        "  <div class=\"logo\"><a href=\"%s\" style=\"color:#fff;text-decoration:none;\">教务管理系统</a></div>\n"
        "  <div class=\"user-info\">当前用户<span>%s (教师)</span></div>\n",
        u, username);

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">课程开设与管理</div>\n");
    mkurl(u,"/teacher/course_add",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>新增课程</a>\n",u,strcmp(active,"course_add")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/teacher/course_edit",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>修改课程信息</a>\n",u,strcmp(active,"course_edit")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/teacher/course_list",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>查看我的课程</a>\n",u,strcmp(active,"course_list")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">选课名单管理</div>\n");
    mkurl(u,"/teacher/roster",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>查看选课名单</a>\n",u,strcmp(active,"roster")==0?" class=\"active\"":"");
    strcat(out,h);
    strcat(out,"</div>\n");

    strcat(out, "<div class=\"nav-group\"><div class=\"group-title\">成绩录入与管理</div>\n");
    mkurl(u,"/teacher/grade_input",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>成绩录入</a>\n",u,strcmp(active,"grade_input")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/teacher/grade_edit",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>成绩修改</a>\n",u,strcmp(active,"grade_edit")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/teacher/grade_stats",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>成绩统计</a>\n",u,strcmp(active,"grade_stats")==0?" class=\"active\"":"");
    strcat(out,h);
    mkurl(u,"/teacher/course_close",userid,username);
    sprintf(h,"  <a href=\"%s\"%s>课程结课</a>\n",u,strcmp(active,"course_close")==0?" class=\"active\"":"");
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
    "  .btn-sm.danger{color:#c0392b;border-color:#e0b4b4;}\n"
    "  .btn-sm.primary{color:#1e3a5f;border-color:#a0b8d0;}\n"
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
/*                    教师主页                                       */
/* ================================================================ */
char* page_teacher_home(const char* username, const char* userid)
{
    char c[4096], h[256];
    mkurl(h,"/teacher/course_add",userid,username);
    {
        char u[256], t[256];
        mkurl(u,"/teacher/course_edit",userid,username);
        mkurl(t,"/teacher/course_list",userid,username);
        sprintf(c,
            "%s\n"
            "<div class=\"welcome\"><h2>教师工作台</h2><p>欢迎使用教务管理系统，请通过左侧导航栏选择操作功能</p></div>\n"
            "<div class=\"card-grid\">\n"
            "  <div class=\"card\"><h3>课程开设与管理</h3><ul class=\"func-list\">\n"
            "    <li><a href=\"%s\">新增课程 - 填写课程信息开设新课程</a></li>\n"
            "    <li><a href=\"%s\">修改课程信息 - 修改未发布课程的信息</a></li>\n"
            "    <li><a href=\"%s\">查看我的课程 - 查看本人开设的所有课程</a></li>\n"
            "  </ul></div>\n",
            mgr_css, h, u, t);
    }

    {
        char r[256], g[256], e[256], c2[256];
        mkurl(r,"/teacher/roster",userid,username);
        mkurl(g,"/teacher/grade_input",userid,username);
        mkurl(e,"/teacher/grade_edit",userid,username);
        mkurl(c2,"/teacher/grade_stats",userid,username);
        {
            char cl[256];
            mkurl(cl,"/teacher/course_close",userid,username);
            sprintf(c+strlen(c),
            "  <div class=\"card\"><h3>选课名单管理</h3><ul class=\"func-list\">\n"
            "    <li><a href=\"%s\">查看选课名单 - 查看已选学生并支持导出</a></li>\n"
            "  </ul></div>\n"
            "  <div class=\"card\"><h3>成绩录入与管理</h3><ul class=\"func-list\">\n"
            "    <li><a href=\"%s\">成绩录入 - 结课后逐条录入学生成绩</a></li>\n"
            "    <li><a href=\"%s\">成绩修改 - 修改已录入的学生成绩</a></li>\n"
            "    <li><a href=\"%s\">成绩统计 - 查看成绩分布与统计数据</a></li>\n"
            "    <li><a href=\"%s\">课程结课 - 执行结课操作进入成绩阶段</a></li>\n"
            "  </ul></div>\n"
            "</div>\n",
            r, g, e, c2, cl);
        }
    }

    frame(buf, "教师工作台", username, userid, "home", c);
    return buf;
}

/* ===== 子页面1：新增课程 ===== */
char* page_teacher_course_add(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>新增课程</h2></div>\n"
        "<div class=\"card\" style=\"max-width:750px;\">\n"
        "  <form method=\"POST\" action=\"/teacher/course_add/save\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <div class=\"form-inline\">\n"
        "      <div class=\"field\"><label>课程号</label><input type=\"text\" name=\"cid\" required></div>\n"
        "      <div class=\"field\"><label>课程名称</label><input type=\"text\" name=\"cname\" style=\"width:200px;\" required></div>\n"
        "      <div class=\"field\"><label>课程类型</label><select name=\"type\"><option>必修</option><option>选修</option></select></div>\n"
        "      <div class=\"field\"><label>学分</label><input type=\"text\" name=\"credit\" style=\"width:60px;\" required></div>\n"
        "    </div>\n"
        "    <div class=\"section-title\">适用专业与上课时间</div>\n"
        "    <div class=\"form-inline\">\n"
        "      <div class=\"field\"><label>适用专业</label><input type=\"text\" name=\"majors\" style=\"width:300px;\" placeholder=\"多个专业用逗号分隔\"></div>\n"
        "      <div class=\"field\"><label>上课时段</label><input type=\"text\" name=\"schedule\" style=\"width:250px;\" placeholder=\"如：周一 第1-2节\"></div>\n"
        "      <div class=\"field\"><label>人数上限</label><input type=\"text\" name=\"max_students\" style=\"width:70px;\" required></div>\n"
        "    </div>\n"
        "    <div class=\"section-title\">成绩占比与周次</div>\n"
        "    <div class=\"form-inline\">\n"
        "      <div class=\"field\"><label>平时成绩占比</label><input type=\"text\" name=\"daily_ratio\" style=\"width:70px;\" placeholder=\"如0.3\"></div>\n"
        "      <div class=\"field\"><label>期末成绩占比</label><input type=\"text\" name=\"final_ratio\" style=\"width:70px;\" placeholder=\"如0.7\"></div>\n"
        "      <div class=\"field\"><label>起始周次</label><input type=\"text\" name=\"start_week\" style=\"width:60px;\"></div>\n"
        "      <div class=\"field\"><label>结束周次</label><input type=\"text\" name=\"end_week\" style=\"width:60px;\"></div>\n"
        "    </div>\n"
        "    <div class=\"section-title\">教学大纲</div>\n"
        "    <textarea name=\"syllabus\" rows=\"4\" style=\"width:100%%;padding:8px;border:1px solid #ccd;border-radius:3px;font-size:13px;\"></textarea>\n"
        "    <button type=\"submit\" style=\"margin-top:14px;padding:10px 30px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">提交新增</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "新增课程", username, userid, "course_add", c);
    return buf;
}

/* ===== 子页面2：修改课程信息 ===== */
char* page_teacher_course_edit(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>修改课程信息</h2></div>\n"
        "<div class=\"card\" style=\"max-width:600px;\">\n"
        "  <p class=\"info-text\">选择课程后可修改信息。选课开启后仅可修改教学大纲等非核心信息。</p>\n"
        "  <form method=\"GET\" action=\"/teacher/course_edit\" style=\"margin:12px 0;display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <select name=\"cid\" style=\"padding:7px;border:1px solid #ccd;border-radius:3px;\">\n"
        "      <option value=\"\">-- 选择课程 --</option>\n"
        "      <option>CS101 数据结构与算法</option>\n"
        "      <option>CS201 操作系统</option>\n"
        "    </select>\n"
        "    <button type=\"submit\" style=\"padding:7px 16px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">查询</button>\n"
        "  </form>\n"
        "  <div class=\"section-title\">课程信息</div>\n"
        "  <form method=\"POST\" action=\"/teacher/course_edit/save\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <div class=\"form-inline\" style=\"flex-direction:column;gap:8px;\">\n"
        "      <div class=\"field\"><label>课程号</label><input type=\"text\" value=\"CS101\" readonly style=\"width:180px;\"></div>\n"
        "      <div class=\"field\"><label>课程名称</label><input type=\"text\" value=\"数据结构与算法\" style=\"width:280px;\"></div>\n"
        "      <div class=\"field\"><label>教学大纲</label><textarea rows=\"3\" style=\"width:400px;padding:8px;border:1px solid #ccd;border-radius:3px;\"></textarea></div>\n"
        "    </div>\n"
        "    <button type=\"submit\" style=\"margin-top:12px;padding:8px 24px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">保存修改</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username, userid, username);

    frame(buf, "修改课程信息", username, userid, "course_edit", c);
    return buf;
}

/* ===== 子页面3：查看我的课程 ===== */
char* page_teacher_course_list(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>查看我的课程</h2></div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>课程号</th><th>课程名称</th><th>类型</th><th>学分</th><th>适用专业</th><th>已选/上限</th><th>状态</th><th>操作</th></tr>\n"
        "  <tr><td>CS101</td><td>数据结构与算法</td><td>必修</td><td>4.0</td><td>计算机科学与技术</td><td>58/60</td><td>选课中</td>\n"
        "    <td><button class=\"btn-sm primary\">查看</button></td></tr>\n"
        "  <tr><td>CS201</td><td>操作系统</td><td>必修</td><td>3.0</td><td>计算机科学与技术</td><td>42/50</td><td>选课中</td>\n"
        "    <td><button class=\"btn-sm primary\">查看</button></td></tr>\n"
        "  <tr><td>CS301</td><td>计算机网络</td><td>选修</td><td>2.0</td><td>全部专业</td><td>30/40</td><td>未发布</td>\n"
        "    <td><button class=\"btn-sm primary\">查看</button><button class=\"btn-sm\">发布</button></td></tr>\n"
        "</table>\n",
        mgr_css);

    frame(buf, "查看我的课程", username, userid, "course_list", c);
    return buf;
}

/* ===== 子页面4：查看选课名单 ===== */
char* page_teacher_roster(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>查看选课名单</h2></div>\n"
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/teacher/roster\" style=\"display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <select name=\"cid\"><option value=\"\">-- 选择课程 --</option><option>CS101 数据结构与算法</option><option>CS201 操作系统</option></select>\n"
        "    <input type=\"text\" name=\"keyword\" placeholder=\"学号或姓名搜索\">\n"
        "    <button type=\"submit\">查询</button>\n"
        "    <button type=\"button\" style=\"background:#27ae60;\">导出名单</button>\n"
        "  </form>\n"
        "</div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>序号</th><th>学号</th><th>姓名</th><th>性别</th><th>年级</th><th>专业</th><th>联系电话</th><th>选课时间</th></tr>\n"
        "  <tr><td>1</td><td>2024010001</td><td>张三</td><td>男</td><td>2024</td><td>计算机科学与技术</td><td>13800138001</td><td>2026-02-25 09:30</td></tr>\n"
        "  <tr><td>2</td><td>2024010002</td><td>李四</td><td>女</td><td>2024</td><td>软件工程</td><td>13800138002</td><td>2026-02-25 10:15</td></tr>\n"
        "  <tr><td>3</td><td>2024010005</td><td>赵六</td><td>男</td><td>2024</td><td>计算机科学与技术</td><td>13800138005</td><td>2026-02-26 14:20</td></tr>\n"
        "</table>\n",
        mgr_css, userid, username);

    frame(buf, "查看选课名单", username, userid, "roster", c);
    return buf;
}

/* ===== 子页面5：成绩录入 ===== */
char* page_teacher_grade_input(const char* username, const char* userid)
{
    char c[8192];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>成绩录入</h2></div>\n"
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/teacher/grade_input\" style=\"display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <select name=\"cid\"><option value=\"\">-- 选择已结课课程 --</option><option>CS101 数据结构与算法</option></select>\n"
        "    <button type=\"submit\">查询</button>\n"
        "  </form>\n"
        "</div>\n"
        "<p class=\"info-text\">当前课程：数据结构与算法 (CS101) | 平时占比：30%% | 期末占比：70%%</p>\n"
        "<form method=\"POST\" action=\"/teacher/grade_input/save\">\n"
        "  <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "  <table class=\"data-table\">\n"
        "    <tr><th>学号</th><th>姓名</th><th>平时成绩(0-100)</th><th>期末成绩(0-100)</th></tr>\n"
        "    <tr><td>2024010001</td><td>张三</td><td><input type=\"text\" name=\"daily_2024010001\" style=\"width:80px;padding:4px;\" value=\"85\"></td>\n"
        "      <td><input type=\"text\" name=\"final_2024010001\" style=\"width:80px;padding:4px;\" value=\"90\"></td></tr>\n"
        "    <tr><td>2024010002</td><td>李四</td><td><input type=\"text\" name=\"daily_2024010002\" style=\"width:80px;padding:4px;\" value=\"78\"></td>\n"
        "      <td><input type=\"text\" name=\"final_2024010002\" style=\"width:80px;padding:4px;\" value=\"82\"></td></tr>\n"
        "    <tr><td>2024010005</td><td>赵六</td><td><input type=\"text\" name=\"daily_2024010005\" style=\"width:80px;padding:4px;\" value=\"92\"></td>\n"
        "      <td><input type=\"text\" name=\"final_2024010005\" style=\"width:80px;padding:4px;\" value=\"88\"></td></tr>\n"
        "  </table>\n"
        "  <button type=\"submit\" style=\"margin-top:14px;padding:10px 30px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">提交成绩</button>\n"
        "</form>\n",
        mgr_css, userid, username, userid, username);

    frame(buf, "成绩录入", username, userid, "grade_input", c);
    return buf;
}

/* ===== 子页面6：成绩修改 ===== */
char* page_teacher_grade_edit(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>成绩修改</h2></div>\n"
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/teacher/grade_edit\" style=\"display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <select name=\"cid\"><option value=\"\">-- 选择课程 --</option><option>CS101 数据结构与算法</option></select>\n"
        "    <button type=\"submit\">查询</button>\n"
        "  </form>\n"
        "</div>\n"
        "<form method=\"POST\" action=\"/teacher/grade_edit/save\">\n"
        "  <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "  <table class=\"data-table\">\n"
        "    <tr><th>学号</th><th>姓名</th><th>平时成绩</th><th>期末成绩</th><th>总评成绩</th><th>绩点</th></tr>\n"
        "    <tr><td>2024010001</td><td>张三</td>\n"
        "      <td><input type=\"text\" name=\"d1\" style=\"width:60px;padding:4px;\" value=\"85\"></td>\n"
        "      <td><input type=\"text\" name=\"f1\" style=\"width:60px;padding:4px;\" value=\"90\"></td>\n"
        "      <td>88.5</td><td>3.7</td></tr>\n"
        "    <tr><td>2024010002</td><td>李四</td>\n"
        "      <td><input type=\"text\" name=\"d2\" style=\"width:60px;padding:4px;\" value=\"78\"></td>\n"
        "      <td><input type=\"text\" name=\"f2\" style=\"width:60px;padding:4px;\" value=\"82\"></td>\n"
        "      <td>80.8</td><td>3.0</td></tr>\n"
        "  </table>\n"
        "  <button type=\"submit\" style=\"margin-top:14px;padding:10px 30px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;cursor:pointer;\">保存修改</button>\n"
        "</form>\n",
        mgr_css, userid, username, userid, username);

    frame(buf, "成绩修改", username, userid, "grade_edit", c);
    return buf;
}

/* ===== 子页面7：成绩统计 ===== */
char* page_teacher_grade_stats(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>成绩统计</h2></div>\n"
        "<div class=\"toolbar\">\n"
        "  <form method=\"GET\" action=\"/teacher/grade_stats\" style=\"display:flex;gap:8px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <select name=\"cid\"><option>CS101 数据结构与算法</option><option>CS201 操作系统</option></select>\n"
        "    <button type=\"submit\">查询</button>\n"
        "  </form>\n"
        "</div>\n"
        "<div style=\"margin-bottom:16px;\">\n"
        "  <div class=\"stat-box\"><div class=\"num\">82.5</div><div class=\"lbl\">平均分</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">95.0</div><div class=\"lbl\">最高分</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">60.0</div><div class=\"lbl\">最低分</div></div>\n"
        "  <div class=\"stat-box\"><div class=\"num\">93.3%%</div><div class=\"lbl\">及格率</div></div>\n"
        "</div>\n"
        "<table class=\"data-table\">\n"
        "  <tr><th>分数段</th><th>人数</th><th>占比</th></tr>\n"
        "  <tr><td>90-100</td><td>18</td><td>31.0%%</td></tr>\n"
        "  <tr><td>80-89</td><td>22</td><td>37.9%%</td></tr>\n"
        "  <tr><td>70-79</td><td>10</td><td>17.2%%</td></tr>\n"
        "  <tr><td>60-69</td><td>4</td><td>6.9%%</td></tr>\n"
        "  <tr><td>0-59</td><td>4</td><td>6.9%%</td></tr>\n"
        "</table>\n",
        mgr_css, userid, username);

    frame(buf, "成绩统计", username, userid, "grade_stats", c);
    return buf;
}

/* ===== 子页面8：课程结课 ===== */
char* page_teacher_course_close(const char* username, const char* userid)
{
    char c[4096];
    sprintf(c,
        "%s\n"
        "<div class=\"welcome\"><h2>课程结课</h2></div>\n"
        "<div class=\"card\" style=\"max-width:600px;\">\n"
        "  <p class=\"info-text\">选择课程执行结课操作，结课后课程进入成绩录入阶段，学生选课通道对该课程关闭。</p>\n"
        "  <form method=\"POST\" action=\"/teacher/course_close/do\" style=\"margin-top:14px;\">\n"
        "    <input type=\"hidden\" name=\"userid\" value=\"%s\"><input type=\"hidden\" name=\"username\" value=\"%s\"><input type=\"hidden\" name=\"role\" value=\"teacher\">\n"
        "    <select name=\"cid\" style=\"padding:7px;border:1px solid #ccd;border-radius:3px;margin-bottom:12px;\">\n"
        "      <option value=\"\">-- 选择授课中课程 --</option>\n"
        "      <option>CS101 数据结构与算法</option>\n"
        "      <option>CS201 操作系统</option>\n"
        "    </select>\n"
        "    <br>\n"
        "    <p class=\"info-text\" style=\"color:#c0392b;\"><strong>注意：</strong>结课操作不可撤销，请确认课程教学已完成。</p>\n"
        "    <button type=\"submit\" style=\"margin-top:10px;padding:10px 30px;background:#c0392b;color:#fff;border:none;border-radius:3px;cursor:pointer;\">确认结课</button>\n"
        "  </form>\n"
        "</div>\n",
        mgr_css, userid, username);

    frame(buf, "课程结课", username, userid, "course_close", c);
    return buf;
}
