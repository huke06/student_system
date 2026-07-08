/* logger.h - 日志系统声明，记录系统操作日志，支持写入、读取、统计分析 */

#ifndef LOGGER_H
#define LOGGER_H

#define LOG_FILE "system.log"

/* 日志类型 */
#define LOG_TYPE_LOGIN  1
#define LOG_TYPE_LOGOUT 2
#define LOG_TYPE_ADD    3
#define LOG_TYPE_MODIFY 4
#define LOG_TYPE_DELETE 5
#define LOG_TYPE_ERROR  6
#define LOG_TYPE_SYSTEM 7

/* 写入一条日志 */
void log_write(int type, const char* user, const char* action);

/* 日志类型转中文名 */
const char* log_type_name(int type);

/* 显示最近N条日志 */
void log_show_recent(int count);

/* 按类型筛选并显示日志 */
void log_show_by_type(int type);

/* 搜索包含关键词的日志 */
void log_search_user(const char* keyword);

/* 显示日志统计概览 */
void log_show_stats(void);

#endif /* LOGGER_H */
