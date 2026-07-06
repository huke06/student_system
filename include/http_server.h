/*
 * http_server.h - HTTP服务器模块声明
 * 负责Socket通信、请求解析、路由分发、响应发送
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <winsock2.h>
#include "common.h"

/*
 * 启动HTTP服务器
 * 初始化Socket，绑定端口，进入请求处理主循环
 * 返回值：0成功，-1失败
 */
int http_server_start(void);

/*
 * 解析原始HTTP请求报文
 * raw: 原始请求字符串
 * req: 输出参数，存储解析结果
 */
void parse_http_request(const char* raw, struct HttpRequest* req);

/*
 * URL解码（%XX转为原字符，+转空格）
 * str: 原地解码
 */
void url_decode_str(char* str);

/*
 * 从URL查询参数中获取值
 * query: 查询字符串 "key1=val1&key2=val2"
 * key: 参数名
 * value: 输出参数，参数值
 */
void get_query_param(const char* query, const char* key, char* value);

/*
 * 从POST请求体中获取参数值
 * body: 请求体字符串
 * key: 参数名
 * value: 输出参数，参数值
 */
void get_post_param_value(const char* body, const char* key, char* value);

/*
 * 发送HTTP响应
 * client: 客户端socket
 * status_code: 状态码(200/302/404)
 * content_type: 内容类型
 * body: 响应体HTML
 */
void send_http_response(SOCKET client, int status_code, const char* content_type,
                        const char* body);

/*
 * 发送302重定向
 * client: 客户端socket
 * location: 目标URL
 */
void send_redirect(SOCKET client, const char* location);

/*
 * 路由分发处理
 * 根据请求路径分发给对应的处理函数
 */
void route_dispatch(SOCKET client, struct HttpRequest* req);

#endif /* HTTP_SERVER_H */
