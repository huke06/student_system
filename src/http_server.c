/*
 * http_server.c - HTTP服务器核心
 * 路由表驱动 + 模板文件渲染
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_server.h"
#include "page.h"

/* ===== 路由表条目 ===== */
struct Route {
    const char* method;       /* "GET" / "POST" */
    const char* path;         /* URL路径 */
    const char* tmpl;         /* HTML模板文件路径，NULL表示特殊处理 */
    const char* active;       /* 侧边栏高亮路径，即path本身 */
    int need_auth;            /* 是否需要登录验证 */
    const char* allowed_role; /* 允许的角色："admin"/"teacher"/"student"/NULL */
    int is_login;             /* 是否登录处理（特殊标记） */
    int is_logout;            /* 是否退出登录 */
};

/*
 * 路由表：所有URL映射集中定义
 */
static const struct Route routes[]={
    /* method  path                       template                     active          auth  role      login logout */
    {"GET",   "/",                       "static/login.html",         "",             0,    NULL,     0,    0},
    {"POST",  "/login",                  NULL,                        "",             0,    NULL,     1,    0},
    {"GET",   "/logout",                 NULL,                        "",             0,    NULL,     0,    1},
    /* 管理员 */
    {"GET",   "/admin",                  "static/admin/home.html",           "/admin",               1, "admin", 0,0},
    {"GET",   "/admin/student_mgr",      "static/admin/student_mgr.html",   "/admin/student_mgr",   1, "admin", 0,0},
    {"GET",   "/admin/teacher_mgr",      "static/admin/teacher_mgr.html",   "/admin/teacher_mgr",   1, "admin", 0,0},
    {"GET",   "/admin/admin_mgr",        "static/admin/admin_mgr.html",     "/admin/admin_mgr",     1, "admin", 0,0},
    {"GET",   "/admin/select_config",    "static/admin/select_config.html", "/admin/select_config", 1, "admin", 0,0},
    {"GET",   "/admin/select_force",     "static/admin/select_force.html",  "/admin/select_force",  1, "admin", 0,0},
    {"GET",   "/admin/select_monitor",   "static/admin/select_monitor.html","/admin/select_monitor",1, "admin", 0,0},
    {"GET",   "/admin/course_list",      "static/admin/course_list.html",   "/admin/course_list",   1, "admin", 0,0},
    {"GET",   "/admin/course_edit",      "static/admin/course_edit.html",   "/admin/course_edit",   1, "admin", 0,0},
    {"GET",   "/admin/course_delete",    "static/admin/course_delete.html", "/admin/course_delete", 1, "admin", 0,0},
    {"GET",   "/admin/sys_semester",     "static/admin/sys_semester.html",  "/admin/sys_semester",  1, "admin", 0,0},
    {"GET",   "/admin/sys_majors",       "static/admin/sys_majors.html",    "/admin/sys_majors",    1, "admin", 0,0},
    /* 教师 */
    {"GET",   "/teacher",                "static/teacher/home.html",         "/teacher",             1, "teacher", 0,0},
    {"GET",   "/teacher/course_add",     "static/teacher/course_add.html",  "/teacher/course_add",  1, "teacher", 0,0},
    {"GET",   "/teacher/course_edit",    "static/teacher/course_edit.html", "/teacher/course_edit", 1, "teacher", 0,0},
    {"GET",   "/teacher/course_list",    "static/teacher/course_list.html", "/teacher/course_list", 1, "teacher", 0,0},
    {"GET",   "/teacher/roster",         "static/teacher/roster.html",      "/teacher/roster",      1, "teacher", 0,0},
    {"GET",   "/teacher/grade_input",    "static/teacher/grade_input.html", "/teacher/grade_input", 1, "teacher", 0,0},
    {"GET",   "/teacher/grade_edit",     "static/teacher/grade_edit.html",  "/teacher/grade_edit",  1, "teacher", 0,0},
    {"GET",   "/teacher/grade_stats",    "static/teacher/grade_stats.html", "/teacher/grade_stats", 1, "teacher", 0,0},
    {"GET",   "/teacher/course_close",   "static/teacher/course_close.html","/teacher/course_close",1, "teacher", 0,0},
    /* 学生 */
    {"GET",   "/student",                "static/student/home.html",         "/student",             1, "student", 0,0},
    {"GET",   "/student/my_info",        "static/student/my_info.html",     "/student/my_info",     1, "student", 0,0},
    {"GET",   "/student/edit_info",      "static/student/edit_info.html",   "/student/edit_info",   1, "student", 0,0},
    {"GET",   "/student/change_pwd",     "static/student/change_pwd.html",  "/student/change_pwd",  1, "student", 0,0},
    {"GET",   "/student/course_select",  "static/student/course_select.html","/student/course_select",1,"student",0,0},
    {"GET",   "/student/my_schedule",    "static/student/my_schedule.html", "/student/my_schedule", 1, "student", 0,0},
    {"GET",   "/student/my_grades",      "static/student/my_grades.html",   "/student/my_grades",   1, "student", 0,0},
    {"GET",   "/student/grade_stats",    "static/student/grade_stats.html", "/student/grade_stats", 1, "student", 0,0},
    {"GET",   "/student/ai_analysis",    "static/student/ai_analysis.html", "/student/ai_analysis", 1, "student", 0,0},
};

#define ROUTE_COUNT (sizeof(routes)/sizeof(routes[0]))

/* ===== 请求解析 ===== */

void parse_http_request(const char* raw, struct HttpRequest* req)
{
    int i, j, len;

    req->method[0]='\0'; req->path[0]='\0';
    req->query[0]='\0'; req->body[0]='\0';
    len=strlen(raw);

    /* 解析方法 */
    i=0;
    while(i<len && raw[i]!=' ' && raw[i]!='\0') { req->method[i]=raw[i]; i++; }
    req->method[i]='\0';
    while(i<len && raw[i]==' ') i++;

    /* 解析路径 */
    j=0;
    while(i<len && raw[i]!=' ' && raw[i]!='?' && raw[i]!='\0')
        { req->path[j]=raw[i]; i++; j++; }
    req->path[j]='\0';

    /* 解析查询参数 */
    if(raw[i]=='?') {
        i++; j=0;
        while(i<len && raw[i]!=' ' && raw[i]!='\0')
            { req->query[j]=raw[i]; i++; j++; }
        req->query[j]='\0';
    }
    if(strlen(req->path)==0) strcpy(req->path, "/");

    /* POST请求体 */
    if(strcmp(req->method, "POST")==0) {
        char* p=strstr(raw, "\r\n\r\n");
        if(p!=NULL) strcpy(req->body, p+4);
    }
}

/* ===== URL/参数处理 ===== */

void url_decode_str(char* str)
{
    int i, j, code; char hex[3];
    i=0; j=0;
    while(str[i]!='\0') {
        if(str[i]=='%' && str[i+1] && str[i+2])
            { hex[0]=str[i+1]; hex[1]=str[i+2]; hex[2]='\0';
              sscanf(hex,"%x",&code); str[j]=(char)code; i+=3; }
        else if(str[i]=='+') { str[j]=' '; i++; }
        else { str[j]=str[i]; i++; }
        j++;
    }
    str[j]='\0';
}

void get_query_param(const char* query, const char* key, char* value)
{
    char* pos; char tmp[1024]; int kl, j;
    value[0]='\0'; kl=strlen(key);
    strcpy(tmp, query);
    pos=strstr(tmp, key);
    if(pos==NULL) return;
    pos+=kl; if(*pos=='=') pos++;
    j=0; while(*pos && *pos!='&') { value[j]=*pos; pos++; j++; }
    value[j]='\0';
    url_decode_str(value);
}

void get_post_param_value(const char* body, const char* key, char* value)
{
    /* 复用相同逻辑 */
    get_query_param(body, key, value);
}

/* ===== HTTP响应 ===== */

void send_http_response(SOCKET client, int status_code,
                        const char* content_type, const char* body)
{
    char hdr[1024]; char st[32]; int bl;
    bl=strlen(body);
    if(status_code==200) strcpy(st,"OK");
    else if(status_code==302) strcpy(st,"Found");
    else if(status_code==404) strcpy(st,"Not Found");
    else strcpy(st,"OK");
    sprintf(hdr,
        "HTTP/1.1 %d %s\r\nContent-Type: %s; charset=utf-8\r\n"
        "Content-Length: %d\r\nConnection: close\r\n\r\n",
        status_code, st, content_type, bl);
    send(client, hdr, strlen(hdr), 0);
    send(client, body, bl, 0);
}

void send_redirect(SOCKET client, const char* location)
{
    char buf[512];
    sprintf(buf,
        "HTTP/1.1 302 Found\r\nLocation: %s\r\n"
        "Content-Length: 0\r\nConnection: close\r\n\r\n", location);
    send(client, buf, strlen(buf), 0);
}

/* ===== 路由分发 ===== */

/*
 * 查找匹配的路由
 * 遍历路由表，匹配method+path
 */
static const struct Route* find_route(const char* method, const char* path)
{
    int i;
    for(i=0; i<(int)ROUTE_COUNT; i++) {
        if(strcmp(routes[i].method, method)!=0) continue;
        if(strcmp(routes[i].path, path)!=0) continue;
        return &routes[i];
    }
    return NULL;
}

void route_dispatch(SOCKET client, struct HttpRequest* req)
{
    const struct Route* rt;
    char userid[32], username[64], role[16];
    char* page;

    /* 读取用户身份参数 */
    get_query_param(req->query, "userid", userid);
    get_query_param(req->query, "username", username);
    get_query_param(req->query, "role", role);

    /* 查找路由 */
    rt=find_route(req->method, req->path);

    /* ===== 登录处理 ===== */
    if(rt!=NULL && rt->is_login) {
        char post_role[16], post_userid[32], redir[256];
        get_post_param_value(req->body, "role", post_role);
        get_post_param_value(req->body, "userid", post_userid);

        if(strcmp(post_role,"admin")==0) strcpy(username,"系统管理员");
        else if(strcmp(post_role,"teacher")==0) strcpy(username,"张老师");
        else if(strcmp(post_role,"student")==0) strcpy(username,"李同学");
        else {
            page=page_render("static/login.html","","","","","请选择有效的登录身份");
            send_http_response(client,200,"text/html",page);
            return;
        }
        sprintf(redir,"/%s?userid=%s&username=%s&role=%s",
                post_role, post_userid, username, post_role);
        send_redirect(client, redir);
        return;
    }

    /* ===== 退出登录 ===== */
    if(rt!=NULL && rt->is_logout) {
        send_redirect(client, "/");
        return;
    }

    /* ===== 权限校验 ===== */
    if(rt!=NULL && rt->need_auth) {
        if(strlen(role)==0 || strcmp(role, rt->allowed_role)!=0) {
            char msg[64];
            if(strcmp(rt->allowed_role,"admin")==0)
                sprintf(msg,"请使用管理员账号登录");
            else if(strcmp(rt->allowed_role,"teacher")==0)
                sprintf(msg,"请使用教师账号登录");
            else
                sprintf(msg,"请使用学生账号登录");
            page=page_render("static/login.html","","","","",msg);
            send_http_response(client,200,"text/html",page);
            return;
        }
    }

    /* ===== 正常页面渲染 ===== */
    if(rt!=NULL && rt->tmpl!=NULL) {
        page=page_render(rt->tmpl, username, userid, role, rt->active, NULL);
        send_http_response(client,200,"text/html",page);
        return;
    }

    /* ===== 404 ===== */
    page=page_404();
    send_http_response(client,404,"text/html",page);
}

/* ===== 服务器启动 ===== */

int http_server_start(void)
{
    WSADATA wsa; SOCKET svr, cli;
    struct sockaddr_in sa, ca; int al; char rbuf[BUFFER_SIZE]; int rl;

    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){printf("[ERR]WSA init\n");return -1;}
    svr=socket(AF_INET,SOCK_STREAM,0);
    if(svr==INVALID_SOCKET){printf("[ERR]socket\n");WSACleanup();return -1;}

    sa.sin_family=AF_INET; sa.sin_addr.s_addr=INADDR_ANY; sa.sin_port=htons(PORT);
    if(bind(svr,(struct sockaddr*)&sa,sizeof(sa))==SOCKET_ERROR)
        {printf("[ERR]bind %d\n",PORT);closesocket(svr);WSACleanup();return -1;}
    if(listen(svr,10)==SOCKET_ERROR)
        {printf("[ERR]listen\n");closesocket(svr);WSACleanup();return -1;}

    printf("\n============================================\n");
    printf("  Edu System Server : http://localhost:%d\n",PORT);
    printf("  %d routes loaded\n",(int)ROUTE_COUNT);
    printf("  Ctrl+C to stop\n");
    printf("============================================\n\n");

    while(1){
        struct HttpRequest req;
        al=sizeof(ca);
        cli=accept(svr,(struct sockaddr*)&ca,&al);
        if(cli==INVALID_SOCKET){printf("[WARN]accept\n");continue;}

        printf("[CONN] %d.%d.%d.%d:%d\n",
            ca.sin_addr.S_un.S_un_b.s_b1,ca.sin_addr.S_un.S_un_b.s_b2,
            ca.sin_addr.S_un.S_un_b.s_b3,ca.sin_addr.S_un.S_un_b.s_b4,
            ntohs(ca.sin_port));

        rl=recv(cli,rbuf,BUFFER_SIZE-1,0);
        if(rl<=0){closesocket(cli);continue;}
        rbuf[rl]='\0';
        parse_http_request(rbuf,&req);
        printf("[REQ] %s %s\n",req.method,req.path);

        route_dispatch(cli,&req);
        closesocket(cli);
        printf("[DONE]\n");
    }
    closesocket(svr); WSACleanup(); return 0;
}
