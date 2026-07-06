/*
 * http_server.c - HTTP服务器核心实现
 * Socket通信 / 请求解析 / URL参数处理 / 响应发送 / 路由分发
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_server.h"
#include "page.h"

/* ========== 请求解析 ========== */

/*
 * 解析原始HTTP请求报文
 * 提取方法、路径、查询参数、请求体
 */
void parse_http_request(const char* raw, struct HttpRequest* req)
{
    int i, j;
    int len;

    /* 初始化 */
    req->method[0]='\0';
    req->path[0]='\0';
    req->query[0]='\0';
    req->body[0]='\0';
    len=strlen(raw);

    /* 1. 解析请求方法 */
    i=0;
    while(i<len && raw[i]!=' ' && raw[i]!='\0') {
        req->method[i]=raw[i];
        i++;
    }
    req->method[i]='\0';

    /* 跳过空格 */
    while(i<len && raw[i]==' ') i++;

    /* 2. 解析URL路径和查询参数 */
    j=0;
    while(i<len && raw[i]!=' ' && raw[i]!='?' && raw[i]!='\0') {
        req->path[j]=raw[i];
        i++; j++;
    }
    req->path[j]='\0';

    /* 如果有?，解析查询参数 */
    if(raw[i]=='?') {
        i++; /* 跳过? */
        j=0;
        while(i<len && raw[i]!=' ' && raw[i]!='\0') {
            req->query[j]=raw[i];
            i++; j++;
        }
        req->query[j]='\0';
    }

    if(strlen(req->path)==0) {
        strcpy(req->path, "/");
    }

    /* 3. 提取POST请求体 */
    if(strcmp(req->method, "POST")==0) {
        char* body_pos;
        body_pos=strstr(raw, "\r\n\r\n");
        if(body_pos != NULL) {
            body_pos=body_pos+4;
            strcpy(req->body, body_pos);
        }
    }
}

/* ========== URL解码 ========== */

/*
 * URL解码: %XX转字符, +转空格
 * 原地解码
 */
void url_decode_str(char* str)
{
    int i, j;
    char hex[3];
    int code;

    i=0; j=0;
    while(str[i]!='\0') {
        if(str[i]=='%' && str[i+1]!='\0' && str[i+2]!='\0') {
            hex[0]=str[i+1];
            hex[1]=str[i+2];
            hex[2]='\0';
            sscanf(hex, "%x", &code);
            str[j]=(char)code;
            i=i+3;
        } else if(str[i]=='+') {
            str[j]=' ';
            i++;
        } else {
            str[j]=str[i];
            i++;
        }
        j++;
    }
    str[j]='\0';
}

/* ========== URL查询参数获取 ========== */

/*
 * 从查询字符串中获取参数值
 * query格式: "key1=val1&key2=val2"
 */
void get_query_param(const char* query, const char* key, char* value)
{
    char* pos;
    char tmp[1024];
    int key_len;
    int j;

    value[0]='\0';
    key_len=strlen(key);
    strcpy(tmp, query);

    pos=strstr(tmp, key);
    if(pos==NULL) return;

    pos=pos+key_len;
    if(*pos=='=') pos++;

    j=0;
    while(*pos!='\0' && *pos!='&') {
        value[j]=*pos;
        pos++; j++;
    }
    value[j]='\0';

    url_decode_str(value);
}

/* ========== POST参数获取 ========== */

/*
 * 从POST请求体获取参数值
 * body格式: "key1=val1&key2=val2"
 */
void get_post_param_value(const char* body, const char* key, char* value)
{
    char* pos;
    char tmp[4096];
    int key_len;
    int j;

    value[0]='\0';
    key_len=strlen(key);
    strcpy(tmp, body);

    pos=strstr(tmp, key);
    if(pos==NULL) return;

    pos=pos+key_len;
    if(*pos=='=') pos++;

    j=0;
    while(*pos!='\0' && *pos!='&') {
        value[j]=*pos;
        pos++; j++;
    }
    value[j]='\0';

    url_decode_str(value);
}

/* ========== HTTP响应 ========== */

/*
 * 发送HTTP响应
 */
void send_http_response(SOCKET client, int status_code, const char* content_type,
                        const char* body)
{
    char header[2048];
    char status_text[32];
    int body_len;

    body_len=strlen(body);

    if(status_code==200) strcpy(status_text, "OK");
    else if(status_code==302) strcpy(status_text, "Found");
    else if(status_code==404) strcpy(status_text, "Not Found");
    else strcpy(status_text, "OK");

    sprintf(header,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, content_type, body_len);

    send(client, header, strlen(header), 0);
    send(client, body, body_len, 0);
}

/*
 * 发送302重定向
 */
void send_redirect(SOCKET client, const char* location)
{
    char buf[512];

    sprintf(buf,
        "HTTP/1.1 302 Found\r\n"
        "Location: %s\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n",
        location);

    send(client, buf, strlen(buf), 0);
}

/* ========== 路由分发 ========== */

/*
 * 根据URL路径分发给对应页面处理函数
 */
void route_dispatch(SOCKET client, struct HttpRequest* req)
{
    char userid[32];
    char username[64];
    char role[16];
    char* page;

    /* 从URL查询参数中获取用户身份 */
    get_query_param(req->query, "userid", userid);
    get_query_param(req->query, "username", username);
    get_query_param(req->query, "role", role);

    /* ===== POST /login - 登录处理 ===== */
    if(strcmp(req->path, "/login")==0 && strcmp(req->method, "POST")==0) {
        char post_role[16];
        char post_userid[32];
        char redirect_url[256];

        get_post_param_value(req->body, "role", post_role);
        get_post_param_value(req->body, "userid", post_userid);

        /* 根据角色确定显示名称 */
        if(strcmp(post_role, "admin")==0) {
            strcpy(username, "系统管理员");
        } else if(strcmp(post_role, "teacher")==0) {
            strcpy(username, "张老师");
        } else if(strcmp(post_role, "student")==0) {
            strcpy(username, "李同学");
        } else {
            page=page_login("请选择有效的登录身份");
            send_http_response(client, 200, "text/html", page);
            return;
        }

        /* 重定向到角色主页，携带用户参数 */
        sprintf(redirect_url, "/%s?userid=%s&username=%s&role=%s",
                post_role, post_userid, username, post_role);
        send_redirect(client, redirect_url);
        return;
    }

    /* ===== GET / 或 /login - 登录页面 ===== */
    if(strcmp(req->path, "/")==0 || strcmp(req->path, "/login")==0) {
        page=page_login(NULL);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    /* ================================================================ */
    /*                        管理员路由                                  */
    /* ================================================================ */

    if(strcmp(req->path, "/admin")==0) {
        if(strlen(role)==0 || strcmp(role, "admin")!=0) {
            page=page_login("请使用管理员账号登录");
            send_http_response(client, 200, "text/html", page);
            return;
        }
        page=page_admin_home(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/student_mgr")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_student_mgr(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/teacher_mgr")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_teacher_mgr(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/admin_mgr")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_admin_mgr(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/select_config")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_select_config(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/select_force")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_select_force(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/select_monitor")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_select_monitor(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/course_list")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_course_list(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/course_edit")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_course_edit(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/course_delete")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_course_delete(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/sys_semester")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_sys_semester(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/admin/sys_majors")==0) {
        if(strlen(role)==0||strcmp(role,"admin")!=0)
        { page=page_login("请使用管理员账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_admin_sys_majors(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    /* ================================================================ */
    /*                        教师路由                                    */
    /* ================================================================ */

    if(strcmp(req->path, "/teacher")==0) {
        if(strlen(role)==0 || strcmp(role, "teacher")!=0) {
            page=page_login("请使用教师账号登录");
            send_http_response(client, 200, "text/html", page);
            return;
        }
        page=page_teacher_home(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/course_add")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_course_add(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/course_edit")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_course_edit(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/course_list")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_course_list(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/roster")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_roster(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/grade_input")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_grade_input(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/grade_edit")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_grade_edit(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/grade_stats")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_grade_stats(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/teacher/course_close")==0) {
        if(strlen(role)==0||strcmp(role,"teacher")!=0)
        { page=page_login("请使用教师账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_teacher_course_close(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    /* ================================================================ */
    /*                        学生路由                                    */
    /* ================================================================ */

    if(strcmp(req->path, "/student")==0) {
        if(strlen(role)==0 || strcmp(role, "student")!=0) {
            page=page_login("请使用学生账号登录");
            send_http_response(client, 200, "text/html", page);
            return;
        }
        page=page_student_home(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/my_info")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_my_info(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/edit_info")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_edit_info(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/change_pwd")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_change_pwd(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/course_select")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_course_select(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/my_schedule")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_my_schedule(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/my_grades")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_my_grades(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/grade_stats")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_grade_stats(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    if(strcmp(req->path, "/student/ai_analysis")==0) {
        if(strlen(role)==0||strcmp(role,"student")!=0)
        { page=page_login("请使用学生账号登录"); send_http_response(client,200,"text/html",page); return; }
        page=page_student_ai_analysis(username, userid);
        send_http_response(client, 200, "text/html", page);
        return;
    }

    /* ===== 退出登录 ===== */
    if(strcmp(req->path, "/logout")==0) {
        send_redirect(client, "/");
        return;
    }

    /* ===== 未匹配路由 - 404 ===== */
    page=page_404();
    send_http_response(client, 404, "text/html", page);
}

/* ========== 服务器启动 ========== */

int http_server_start(void)
{
    WSADATA wsa_data;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int addr_len;
    char recv_buf[BUFFER_SIZE];
    int recv_len;

    if(WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
        printf("[ERROR] Winsock init failed\n");
        return -1;
    }

    server_sock=socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock==INVALID_SOCKET) {
        printf("[ERROR] Socket create failed\n");
        WSACleanup();
        return -1;
    }

    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(PORT);

    if(bind(server_sock, (struct sockaddr*)&server_addr,
            sizeof(server_addr))==SOCKET_ERROR) {
        printf("[ERROR] Bind to port %d failed\n", PORT);
        closesocket(server_sock);
        WSACleanup();
        return -1;
    }

    if(listen(server_sock, 10)==SOCKET_ERROR) {
        printf("[ERROR] Listen failed\n");
        closesocket(server_sock);
        WSACleanup();
        return -1;
    }

    printf("\n");
    printf("============================================\n");
    printf("  Edu Management System Server Running\n");
    printf("  URL: http://localhost:%d\n", PORT);
    printf("  Press Ctrl+C to stop\n");
    printf("============================================\n\n");

    while(1) {
        struct HttpRequest req;

        addr_len=sizeof(client_addr);
        client_sock=accept(server_sock,
                           (struct sockaddr*)&client_addr, &addr_len);
        if(client_sock==INVALID_SOCKET) {
            printf("[WARN] Accept failed, retrying...\n");
            continue;
        }

        printf("[CONN] %d.%d.%d.%d:%d\n",
            client_addr.sin_addr.S_un.S_un_b.s_b1,
            client_addr.sin_addr.S_un.S_un_b.s_b2,
            client_addr.sin_addr.S_un.S_un_b.s_b3,
            client_addr.sin_addr.S_un.S_un_b.s_b4,
            ntohs(client_addr.sin_port));

        recv_len=recv(client_sock, recv_buf, BUFFER_SIZE-1, 0);
        if(recv_len<=0) {
            closesocket(client_sock);
            continue;
        }
        recv_buf[recv_len]='\0';

        parse_http_request(recv_buf, &req);
        printf("[REQ] %s %s", req.method, req.path);
        if(strlen(req.query)>0) printf("?%s", req.query);
        printf("\n");

        route_dispatch(client_sock, &req);

        closesocket(client_sock);
        printf("[DONE] Connection closed\n");
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}
