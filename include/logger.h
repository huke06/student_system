/*
 * logger.h - 日志系统声明
 * 记录系统操作日志，支持写入、读取、统计分析
 */

#ifndef LOGGER_H
#define LOGGER_H

/* 日志类型 */
#define LOG_TYPE_LOGIN    1   /* 登录日志 */
#define LOG_TYPE_LOGOUT   2   /* 退出日志 */
#define LOG_TYPE_ADD      3   /* 新增数据 */
#define LOG_TYPE_MODIFY   4   /* 修改数据 */
#define LOG_TYPE_DELETE   5   /* 删除数据 */
#define LOG_TYPE_ERROR    6   /* 错误日志 */
#define LOG_TYPE_SYSTEM   7   /* 系统操作 */

/* 日志文件路径 */
#define LOG_FILE "system.log"

/*
 * 写入一条日志
 * type: 日志类型(LOG_TYPE_xxx)
 * user: 操作用户描述(如"管理员admin001")
 * action: 操作描述
 */
void log_write(int type, const char* user, const char* action);

/*
 * 获取日志类型的中文名称
 */
const char* log_type_name(int type);

/*
 * 读取并显示最近N条日志
 * count: 要显示的条数
 */
void log_show_recent(int count);

/*
 * 按类型筛选并显示日志
 * type: 日志类型，0表示全部
 */
void log_show_by_type(int type);

/*
 * 搜索特定用户的日志
 * keyword: 搜索关键词(匹配用户或操作描述)
 */
void log_search_user(const char* keyword);

/*
 * 显示日志统计概览
 * 统计各类操作次数、最近活跃用户
 */
void log_show_stats(void);

#endif /* LOGGER_H */
