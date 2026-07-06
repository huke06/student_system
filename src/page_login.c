/*
 * page_login.c - 登录页面和404页面
 * 包含公共CSS样式字串
 */

#include <stdio.h>
#include <string.h>
#include "page.h"

/* 页面缓冲区 */
static char buf[65536];

/*
 * 公共基础CSS样式
 * 登录页和404页使用
 */
static const char* base_css=
    "<style>\n"
    "  *{margin:0;padding:0;box-sizing:border-box;}\n"
    "  body{font-family:'Microsoft YaHei',SimHei,sans-serif;background:#eef1f5;min-height:100vh;}\n"
    "  .footer{text-align:center;color:#999;font-size:12px;padding:20px;}\n"
    "</style>\n";

/*
 * 侧边栏布局CSS（管理员/教师/学生主页共用）
 * 布局结构：左侧固定导航栏 + 右侧内容区
 */
static const char* layout_css=
    "<style>\n"
    "  *{margin:0;padding:0;box-sizing:border-box;}\n"
    "  body{font-family:'Microsoft YaHei',SimHei,sans-serif;background:#eef1f5;min-height:100vh;display:flex;}\n"
    "  /* 左侧导航栏 */\n"
    "  .sidebar{width:220px;background:#1e3a5f;color:#c8d6e5;min-height:100vh;position:fixed;top:0;left:0;bottom:0;overflow-y:auto;}\n"
    "  .sidebar .logo{height:50px;line-height:50px;text-align:center;font-size:17px;font-weight:bold;color:#fff;background:#172d4a;border-bottom:1px solid #2a4a6d;}\n"
    "  .sidebar .user-info{padding:14px 16px;font-size:13px;color:#a0b4c8;border-bottom:1px solid #2a4a6d;}\n"
    "  .sidebar .user-info span{display:block;color:#e8e8e8;font-size:14px;margin-top:2px;}\n"
    "  .nav-group{margin-top:6px;}\n"
    "  .nav-group .group-title{padding:10px 16px 4px;font-size:12px;color:#6b8aaa;font-weight:bold;}\n"
    "  .nav-group a{display:block;padding:8px 16px 8px 24px;color:#c8d6e5;text-decoration:none;font-size:13px;transition:all 0.15s;}\n"
    "  .nav-group a:hover{background:#2a4a6d;color:#fff;border-left:3px solid #4a90d9;padding-left:21px;}\n"
    "  .nav-group a.active{background:#2a4a6d;color:#fff;border-left:3px solid #4a90d9;}\n"
    "  /* 右侧主体 */\n"
    "  .main-area{margin-left:220px;flex:1;display:flex;flex-direction:column;min-height:100vh;}\n"
    "  /* 顶部栏 */\n"
    "  .top-bar{height:50px;line-height:50px;background:#fff;padding:0 25px;border-bottom:1px solid #dde1e6;display:flex;justify-content:flex-end;align-items:center;font-size:13px;color:#555;}\n"
    "  .top-bar a{color:#1e3a5f;text-decoration:none;margin-left:15px;font-weight:bold;}\n"
    "  .top-bar a:hover{color:#c0392b;}\n"
    "  /* 内容区 */\n"
    "  .content{padding:25px;flex:1;}\n"
    "  .content .welcome{margin-bottom:22px;}\n"
    "  .content .welcome h2{font-size:20px;color:#1e3a5f;font-weight:normal;}\n"
    "  .content .welcome p{font-size:13px;color:#999;margin-top:4px;}\n"
    "  .card-grid{display:grid;grid-template-columns:repeat(2,1fr);gap:16px;}\n"
    "  .card{background:#fff;border:1px solid #e4e7ec;border-radius:3px;padding:20px 22px;}\n"
    "  .card h3{font-size:15px;color:#1e3a5f;margin-bottom:10px;padding-bottom:8px;border-bottom:1px solid #eef1f5;}\n"
    "  .card .func-list{list-style:none;}\n"
    "  .card .func-list li{margin-bottom:5px;}\n"
    "  .card .func-list a{display:block;padding:7px 10px;color:#445;text-decoration:none;font-size:13px;border-radius:2px;transition:all 0.15s;}\n"
    "  .card .func-list a:hover{background:#eef3f8;color:#1e3a5f;}\n"
    "  .footer{text-align:center;color:#aaa;font-size:11px;padding:15px;border-top:1px solid #e4e7ec;}\n"
    "</style>\n";

/* ========== 登录页面 ========== */

/*
 * 生成登录页面HTML
 * error_msg: 错误提示，NULL表示无错误
 */
char* page_login(const char* error_msg)
{
    sprintf(buf,
        "<!DOCTYPE html>\n"
        "<html lang=\"zh-CN\">\n"
        "<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">\n"
        "  <title>教务管理系统 - 登录</title>\n"
        "  %s\n"
        "  <style>\n"
        "    .login-box{width:400px;margin:100px auto;background:#fff;border:1px solid #dde1e6;border-radius:4px;padding:38px 36px;}\n"
        "    .login-box h1{text-align:center;color:#1e3a5f;font-size:22px;font-weight:bold;margin-bottom:4px;}\n"
        "    .login-box .subtitle{text-align:center;color:#999;font-size:12px;margin-bottom:22px;}\n"
        "    .form-row{margin-bottom:16px;}\n"
        "    .form-row label{display:block;font-size:13px;color:#444;margin-bottom:5px;}\n"
        "    .form-row input,.form-row select{width:100%%;padding:9px 10px;border:1px solid #ccd;border-radius:3px;font-size:13px;outline:none;}\n"
        "    .form-row input:focus,.form-row select:focus{border-color:#1e3a5f;}\n"
        "    .btn-login{width:100%%;padding:10px;background:#1e3a5f;color:#fff;border:none;border-radius:3px;font-size:14px;cursor:pointer;letter-spacing:2px;}\n"
        "    .btn-login:hover{background:#2a5280;}\n"
        "    .error-tip{background:#fef0f0;color:#c0392b;padding:8px 12px;border:1px solid #f5c6cb;border-radius:3px;margin-bottom:14px;font-size:12px;text-align:center;}\n"
        "  </style>\n"
        "</head>\n"
        "<body>\n"
        "  <div class=\"login-box\">\n"
        "    <h1>教务管理系统</h1>\n"
        "    <p class=\"subtitle\">综合教务管理平台</p>\n",
        base_css);

    /* 错误提示 */
    if(error_msg!=NULL && strlen(error_msg)>0) {
        char tmp[512];
        sprintf(tmp,"    <div class=\"error-tip\">%s</div>\n", error_msg);
        strcat(buf, tmp);
    }

    /* 登录表单 */
    strcat(buf,
        "    <form method=\"POST\" action=\"/login\">\n"
        "      <div class=\"form-row\">\n"
        "        <label>登录身份</label>\n"
        "        <select name=\"role\" required>\n"
        "          <option value=\"\">-- 请选择登录身份 --</option>\n"
        "          <option value=\"admin\">管理员</option>\n"
        "          <option value=\"teacher\">教师</option>\n"
        "          <option value=\"student\">学生</option>\n"
        "        </select>\n"
        "      </div>\n"
        "      <div class=\"form-row\">\n"
        "        <label>账号</label>\n"
        "        <input type=\"text\" name=\"userid\" placeholder=\"请输入学号/工号/管理员账号\" required>\n"
        "      </div>\n"
        "      <div class=\"form-row\">\n"
        "        <label>密码</label>\n"
        "        <input type=\"password\" name=\"password\" placeholder=\"请输入密码\" required>\n"
        "      </div>\n"
        "      <button type=\"submit\" class=\"btn-login\">登 录</button>\n"
        "    </form>\n"
        "  </div>\n"
        "  <div class=\"footer\">教务管理系统 v1.0</div>\n"
        "</body>\n"
        "</html>\n");

    return buf;
}

/* ========== 404页面 ========== */

/*
 * 生成404页面HTML
 */
char* page_404(void)
{
    sprintf(buf,
        "<!DOCTYPE html>\n"
        "<html lang=\"zh-CN\">\n"
        "<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <title>404 - 教务管理系统</title>\n"
        "  %s\n"
        "  <style>\n"
        "    .e404{width:400px;margin:100px auto;text-align:center;background:#fff;border:1px solid #dde1e6;border-radius:4px;padding:40px;}\n"
        "    .e404 .code{font-size:56px;color:#c0392b;font-weight:bold;}\n"
        "    .e404 .msg{font-size:15px;color:#666;margin:12px 0;}\n"
        "    .e404 .hint{font-size:12px;color:#999;}\n"
        "    .e404 a{display:inline-block;margin-top:16px;color:#1e3a5f;text-decoration:none;font-size:13px;}\n"
        "    .e404 a:hover{text-decoration:underline;}\n"
        "  </style>\n"
        "</head>\n"
        "<body>\n"
        "  <div class=\"e404\">\n"
        "    <div class=\"code\">404</div>\n"
        "    <div class=\"msg\">页面未找到</div>\n"
        "    <div class=\"hint\">您访问的页面不存在或已被移除</div>\n"
        "    <a href=\"/\">返回登录页</a>\n"
        "  </div>\n"
        "  <div class=\"footer\">教务管理系统 v1.0</div>\n"
        "</body>\n"
        "</html>\n",
        base_css);

    return buf;
}

/* ===== 导出layout_css供其他页面文件使用 ===== */

/*
 * 返回侧边栏布局公共CSS
 * 管理员/教师/学生页面引用此样式
 */
const char* get_layout_css(void)
{
    return layout_css;
}
