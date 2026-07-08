/*
 * ai_helper.c - AI 辅助功能实现
 * 通过 curl 调用 DeepSeek API，JSON 经临时文件中转，避免中文转义问题
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai_helper.h"
#include "data_store.h"
#include "utils.h"

/* DeepSeek API 配置 */
#define API_KEY  "sk-15662568a6124ae486b8e8f2f6ec483e"
#define API_URL  "https://api.deepseek.com/v1/chat/completions"
#define API_MODEL "deepseek-chat"

/* 临时文件路径 */
#define REQ_FILE  "data/ai_req.json"
#define RESP_FILE "data/ai_resp.json"

/* ================================================================ */
/*                      内部工具函数                                  */
/* ================================================================ */

/*
 * 对字符串中的特殊 JSON 字符做简单转义
 * 将 \ " 和换行符转义，防止 JSON 格式错误
 * src: 源字符串
 * dst: 输出缓冲区（需足够大，建议 src 长度*2+1）
 */
static void _json_escape(const char* src, char* dst)
{
    int i, j;
    i=0; j=0;
    while(src[i]!='\0') {
        switch(src[i]) {
            case '\\': dst[j++]='\\'; dst[j++]='\\'; break;
            case '"':  dst[j++]='\\'; dst[j++]='"';  break;
            case '\n': dst[j++]='\\'; dst[j++]='n';  break;
            case '\r': dst[j++]='\\'; dst[j++]='r';  break;
            case '\t': dst[j++]='\\'; dst[j++]='t';  break;
            default:   dst[j++]=src[i]; break;
        }
        i++;
    }
    dst[j]='\0';
}

/*
 * 发送请求到 DeepSeek API 并获取回复
 * system_prompt: 系统角色设定
 * user_message:  用户输入
 * reply:         输出回复文本
 * max_len:       输出缓冲区长度
 */
int ai_call(const char* system_prompt, const char* user_message,
            char* reply, int max_len)
{
    FILE* fp;
    char* esc_sys;
    char* esc_usr;
    char cmd[1024];
    char line[2048];
    int found;

    if(reply==NULL || max_len<=0) return 0;

    /* 初始化为空 */
    reply[0]='\0';

    /* 对系统提示词和用户消息做 JSON 转义 */
    esc_sys=(char*)malloc(strlen(system_prompt)*2+1);
    esc_usr=(char*)malloc(strlen(user_message)*2+1);
    if(esc_sys==NULL || esc_usr==NULL) {
        if(esc_sys!=NULL) free(esc_sys);
        if(esc_usr!=NULL) free(esc_usr);
        return 0;
    }
    _json_escape(system_prompt, esc_sys);
    _json_escape(user_message, esc_usr);

    /* 构建 JSON 请求并写入临时文件 */
    fp=fopen(REQ_FILE, "w");
    if(fp==NULL) {
        free(esc_sys); free(esc_usr);
        return 0;
    }

    fprintf(fp, "{");
    fprintf(fp, "\"model\":\"%s\",", API_MODEL);
    fprintf(fp, "\"messages\":[");
    fprintf(fp, "{\"role\":\"system\",\"content\":\"%s\"},", esc_sys);
    fprintf(fp, "{\"role\":\"user\",\"content\":\"%s\"}", esc_usr);
    fprintf(fp, "],");
    fprintf(fp, "\"max_tokens\":1500,");
    fprintf(fp, "\"temperature\":0.7");
    fprintf(fp, "}\n");

    fclose(fp);
    free(esc_sys);
    free(esc_usr);

    /* 调用 curl 发送请求 */
    sprintf(cmd,
        "curl -s -X POST \"%s\" "
        "-H \"Content-Type: application/json\" "
        "-H \"Authorization: Bearer %s\" "
        "-d @%s -o %s",
        API_URL, API_KEY, REQ_FILE, RESP_FILE);

    system(cmd);

    /* 读取响应文件，提取 content 字段 */
    fp=fopen(RESP_FILE, "r");
    if(fp==NULL) return 0;

    found=0;
    while(fgets(line, sizeof(line), fp)!=NULL) {
        char* p;

        /* 查找 "content":" 标记 */
        p=strstr(line, "\"content\":\"");
        if(p!=NULL) {
            int ri;
            p+=11; /* 跳过 "content":" */

            ri=0;
            while(*p!='\0' && ri<max_len-1) {
                /* 遇到 \" 表示字段结束，但可能是 \\" 即转义引号 */
                if(p[0]=='\\' && p[1]=='\"') {
                    /* 转义引号 → 还原为 " */
                    reply[ri++]='"';
                    p+=2;
                    continue;
                }
                if(p[0]=='\\' && p[1]=='n') {
                    reply[ri++]='\n';
                    p+=2;
                    continue;
                }
                if(p[0]=='\\' && p[1]=='\\') {
                    reply[ri++]='\\';
                    p+=2;
                    continue;
                }
                if(p[0]=='\"') {
                    /* 真正的字符串结束 */
                    break;
                }
                reply[ri++]=*p;
                p++;
            }
            reply[ri]='\0';

            if(strlen(reply)>0) found=1;
            /* 不break，继续读可能后面还有content（有些响应分多行） */
        }
    }
    fclose(fp);

    /* 如果通过逐行解析没找到，尝试用更简单的方式：读整个文件查找 */
    if(!found) {
        long fsize;
        char* fbuf;

        fp=fopen(RESP_FILE, "r");
        if(fp!=NULL) {
            fseek(fp, 0, SEEK_END);
            fsize=ftell(fp);
            rewind(fp);
            if(fsize>0 && fsize<100000) {
                fbuf=(char*)malloc(fsize+1);
                if(fbuf!=NULL) {
                    fread(fbuf, 1, fsize, fp);
                    fbuf[fsize]='\0';

                    /* 查找 "content":" */
                    {
                        char* p;
                        p=strstr(fbuf, "\"content\":\"");
                        if(p!=NULL) {
                            int ri;
                            p+=11;
                            ri=0;
                            while(*p!='\0' && ri<max_len-1) {
                                if(p[0]=='\\' && p[1]=='\"') {
                                    reply[ri++]='"'; p+=2; continue;
                                }
                                if(p[0]=='\\' && p[1]=='n') {
                                    reply[ri++]='\n'; p+=2; continue;
                                }
                                if(p[0]=='\\' && p[1]=='\\') {
                                    reply[ri++]='\\'; p+=2; continue;
                                }
                                if(p[0]=='\"') break;
                                reply[ri++]=*p; p++;
                            }
                            reply[ri]='\0';
                            found=1;
                        }
                    }
                    free(fbuf);
                }
            }
            fclose(fp);
        }
    }

    return found ? 1 : 0;
}

/* ================================================================ */
/*                      智能选课推荐                                  */
/* ================================================================ */

int ai_course_recommend(const char* student_id,
                        const char* interests, const char* future_plan,
                        char* reply, int max_len)
{
    struct Student st;
    struct Course courses[MAX_COURSES];
    int count, i;
    char user_msg[4096];
    char course_info[3072];
    char sys_prompt[512];
    int offset;

    /* 获取学生信息 */
    if(!ds_student_find_by_id(student_id, &st)) {
        strcpy(reply, "无法获取学生信息。");
        return 0;
    }

    /* 获取所有课程 */
    ds_course_load_all(courses, &count);
    if(count==0) {
        strcpy(reply, "当前没有课程数据。");
        return 0;
    }

    /* 构建课程信息字符串（仅包含匹配学生专业的课程） */
    course_info[0]='\0';
    offset=0;
    for(i=0; i<count; i++) {
        char buf[1024];
        const char* type_str;
        const char* status_str;

        /* 只推荐适用该学生专业的课程 */
        if(strstr(courses[i].majors, st.major)==NULL) continue;

        if(courses[i].type==COURSE_TYPE_REQUIRED) type_str="必修";
        else type_str="选修";

        if(courses[i].status==COURSE_STATUS_DRAFT) status_str="未发布";
        else if(courses[i].status==COURSE_STATUS_SELECT) status_str="选课中";
        else if(courses[i].status==COURSE_STATUS_CLOSED) status_str="已结课";
        else status_str="?";

        sprintf(buf,
            "%d. 课程号:%s 名称:%s 类型:%s 学分:%.1f 教师:%s\n"
            "   上课时段:%s 教学大纲:%s 状态:%s\n",
            i+1, courses[i].id, courses[i].name, type_str, courses[i].credit,
            courses[i].teacher_id, courses[i].schedule,
            courses[i].syllabus, status_str);

        if(offset+strlen(buf) < (int)sizeof(course_info)-10) {
            strcat(course_info, buf);
            offset+=strlen(buf);
        }
    }

    /* 构建用户消息 */
    sprintf(user_msg,
        "学生信息：\n"
        "- 专业：%s\n"
        "- 兴趣：%s\n"
        "- 未来规划：%s\n\n"
        "全部课程列表：\n%s\n\n"
        "请根据以上信息，为该学生推荐3-5门最合适的课程，"
        "推荐时请综合考虑学生的专业相关性、兴趣爱好和未来规划。"
        "对每门推荐课程简要说明推荐理由（1-2句话即可）。",
        st.major,
        (interests!=NULL && strlen(interests)>0) ? interests : "未填写",
        (future_plan!=NULL && strlen(future_plan)>0) ? future_plan : "未填写",
        course_info);

    /* 系统提示词 */
    strcpy(sys_prompt,
        "你是一个大学教务系统的智能选课助手。你的任务是根据学生的专业背景、"
        "兴趣爱好和未来规划，从课程列表中推荐最适合的课程。"
        "推荐要具体、有针对性，说明推荐理由。用中文回复。"
        "请使用纯文本格式回复，不要使用Markdown格式标记（如#、**、-、*等符号）。");

    return ai_call(sys_prompt, user_msg, reply, max_len);
}

/* ================================================================ */
/*                      智能成绩分析                                  */
/* ================================================================ */

int ai_grade_analysis(const char* student_id, char* reply, int max_len)
{
    struct Student st;
    struct Score scores[MAX_SCORES];
    int count, i;
    char user_msg[4096];
    char grade_info[3072];
    char sys_prompt[512];
    int offset;

    /* 获取学生信息 */
    if(!ds_student_find_by_id(student_id, &st)) {
        strcpy(reply, "无法获取学生信息。");
        return 0;
    }

    /* 获取成绩 */
    ds_score_load_by_student(student_id, scores, &count);
    if(count==0) {
        sprintf(reply,
            "学生 %s(%s) 目前没有已出成绩记录，无法进行AI分析。"
            "请等待教师录入成绩后再使用此功能。",
            st.id, st.name);
        return 0;
    }

    /* 构建成绩信息 */
    grade_info[0]='\0';
    offset=0;
    for(i=0; i<count; i++) {
        struct Course c;
        char buf[256];
        char cname[32];

        strcpy(cname, scores[i].course_id);
        if(ds_course_find_by_id(scores[i].course_id, &c)) {
            strcpy(cname, c.name);
        }

        sprintf(buf,
            "%d. 课程:%s(%s) 平时:%.1f 期末:%.1f 总评:%.1f 绩点:%.2f\n",
            i+1, scores[i].course_id, cname,
            scores[i].daily_score, scores[i].final_score,
            scores[i].total_score, scores[i].gpa);

        if(offset+strlen(buf) < (int)sizeof(grade_info)-10) {
            strcat(grade_info, buf);
            offset+=strlen(buf);
        }
    }

    /* 构建用户消息 */
    sprintf(user_msg,
        "学生信息：\n"
        "- 姓名：%s\n"
        "- 学号：%s\n"
        "- 专业：%s\n\n"
        "成绩数据：\n%s\n\n"
        "请对该学生的成绩进行全面分析。",
        st.name, st.id, st.major, grade_info);

    /* 系统提示词 */
    strcpy(sys_prompt,
        "你是一个学习分析专家。请根据学生成绩数据进行深入分析，"
        "输出以下四个方面的内容：\n"
        "1. 成绩画像：整体学习表现概述\n"
        "2. 优势学科：哪些课程表现突出\n"
        "3. 薄弱环节：哪些课程需要加强\n"
        "4. 学习建议：针对性的改进方案\n"
        "用中文回复，语气积极鼓励。"
        "请使用纯文本格式回复，不要使用Markdown格式标记（如#、**、-、*等符号）。");

    return ai_call(sys_prompt, user_msg, reply, max_len);
}
