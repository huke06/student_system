/*
日志系统实现
文本文件存储，支持写入、读取筛选、统计分析
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "logger.h"

/*内部缓冲区*/
static char buf[4096];

/*获取当前时间字符串，格式 yyyy-mm-dd HH:MM:SS*/
static void get_now_str(char* out)
{
    time_t t;
    struct tm* tm_info;

    time(&t);
    tm_info=localtime(&t);
    sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d",
        tm_info->tm_year+1900, tm_info->tm_mon+1, tm_info->tm_mday,
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
}

/*写入一条日志到文件
 格式: [时间] [类型] 用户 | 操作描述
 */
void log_write(int type, const char* user, const char* action)
{
    FILE* fp;
    char now[32];

    get_now_str(now);

    fp=fopen(LOG_FILE, "a");
    if(fp==NULL) return;

    fprintf(fp, "[%s] [%s] %s | %s\n",
            now, log_type_name(type), user, action);
    fclose(fp);
}

/*日志类型中文名*/
const char* log_type_name(int type)
{
    switch(type) {
        case LOG_TYPE_LOGIN:  return "登录";
        case LOG_TYPE_LOGOUT: return "退出";
        case LOG_TYPE_ADD:    return "新增";
        case LOG_TYPE_MODIFY: return "修改";
        case LOG_TYPE_DELETE: return "删除";
        case LOG_TYPE_ERROR:  return "错误";
        case LOG_TYPE_SYSTEM: return "系统";
        default: return "未知";
    }
}

/*读取并显示最近N条日志*/
void log_show_recent(int count)
{
    FILE* fp;
    int total, i, start;

    fp=fopen(LOG_FILE, "r");
    if(fp==NULL) {
        printf("\n  暂无日志记录。\n");
        return;
    }

    /* 先数总行数 */
    total=0;
    while(fgets(buf, sizeof(buf), fp)!=NULL) total++;
    rewind(fp);

    start=total-count;
    if(start<0) start=0;

    printf("\n  === 最近 %d 条日志（共 %d 条） ===\n\n", count, total);
    if(total==0) {
        printf("  暂无日志记录。\n");
        fclose(fp);
        return;
    }

    i=0;
    while(fgets(buf, sizeof(buf), fp)!=NULL) {
        if(i>=start) {
            /* 去掉末尾换行 */
            buf[strcspn(buf, "\n")]='\0';
            printf("  %s\n", buf);
        }
        i++;
    }
    fclose(fp);
}

/*按类型筛选并显示日志type: 0表示显示全部*/
void log_show_by_type(int type)
{
    FILE* fp;
    char type_str[16];
    int found;

    sprintf(type_str, "[%s]", log_type_name(type));

    fp=fopen(LOG_FILE, "r");
    if(fp==NULL) {
        printf("\n  暂无日志记录。\n");
        return;
    }

    printf("\n  === 类型筛选: %s ===\n\n", log_type_name(type));

    found=0;
    while(fgets(buf, sizeof(buf), fp)!=NULL) {
        if(strstr(buf, type_str)!=NULL) {
            buf[strcspn(buf, "\n")]='\0';
            printf("  %s\n", buf);
            found++;
        }
    }

    if(found==0) {
        printf("  该类型暂无日志记录。\n");
    }
    fclose(fp);
}

/*搜索包含关键词的日志*/
void log_search_user(const char* keyword)
{
    FILE* fp;
    int found;

    fp=fopen(LOG_FILE, "r");
    if(fp==NULL) {
        printf("\n  暂无日志记录。\n");
        return;
    }

    printf("\n  === 搜索关键词: \"%s\" ===\n\n", keyword);

    found=0;
    while(fgets(buf, sizeof(buf), fp)!=NULL) {
        if(strstr(buf, keyword)!=NULL) {
            buf[strcspn(buf, "\n")]='\0';
            printf("  %s\n", buf);
            found++;
        }
    }

    if(found==0) {
        printf("  未找到包含 \"%s\" 的日志记录。\n", keyword);
    }
    printf("  共找到 %d 条匹配记录。\n", found);
    fclose(fp);
}

/* 显示日志统计概览*/
void log_show_stats(void)
{
    FILE* fp;
    int counts[8]={0};
    int total;
    int login_count, error_count;

    fp=fopen(LOG_FILE, "r");
    if(fp==NULL) {
        printf("\n  暂无日志记录。\n");
        return;
    }

    total=0;
    while(fgets(buf, sizeof(buf), fp)!=NULL) {
        int i;
        for(i=LOG_TYPE_LOGIN; i<=LOG_TYPE_SYSTEM; i++) {
            char type_tag[16];
            sprintf(type_tag, "[%s]", log_type_name(i));
            if(strstr(buf, type_tag)!=NULL) {
                counts[i]++;
            }
        }
        total++;
    }

    login_count=counts[LOG_TYPE_LOGIN]+counts[LOG_TYPE_LOGOUT];
    error_count=counts[LOG_TYPE_ERROR];

    printf("\n  ====== 日志统计概览 ======\n\n");
    printf("  日志总数: %d 条\n\n", total);
    printf("  --- 按类型统计 ---\n");
    printf("  登录/退出: %d 条\n", login_count);
    printf("  数据新增: %d 条\n", counts[LOG_TYPE_ADD]);
    printf("  数据修改: %d 条\n", counts[LOG_TYPE_MODIFY]);
    printf("  数据删除: %d 条\n", counts[LOG_TYPE_DELETE]);
    printf("  错误日志: %d 条\n", error_count);
    printf("  系统操作: %d 条\n", counts[LOG_TYPE_SYSTEM]);

    if(total>0) {
        printf("\n  --- 活跃时段 ---\n");
    }
    /*找最早和最晚日志时间*/
    rewind(fp);
    {
        char first_time[32]="";
        char last_time_buf[32]="";
        while(fgets(buf, sizeof(buf), fp)!=NULL) {
            if(buf[0]=='[' && strlen(buf)>20) {
                if(first_time[0]=='\0') {
                    strncpy(first_time, buf+1, 19);
                    first_time[19]='\0';
                }
                strncpy(last_time_buf, buf+1, 19);
                last_time_buf[19]='\0';
            }
        }
        if(first_time[0]!='\0') {
            printf("  最早记录: %s\n", first_time);
            printf("  最晚记录: %s\n", last_time_buf);
        }
    }

    printf("\n  ==========================\n");
    fclose(fp);
}
