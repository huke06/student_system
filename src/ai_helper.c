/*AI辅助功能实现，通过curl调用DeepSeek API，JSON经临时文件中转*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai_helper.h"
#include "data_store.h"
#include "utils.h"

/*DeepSeek API 配置*/
#define API_URL  "https://api.deepseek.com/v1/chat/completions"
#define API_MODEL "deepseek-chat"

/*临时文件路径*/
#define REQ_FILE  "data/ai_req.json"
#define RESP_FILE "data/ai_resp.json"

/*内部工具函数*/

/*
对字符串中的特殊 JSON 字符做简单转义
将 \ " 和换行符转义，防止 JSON 格式错误
src: 源字符串
dst: 输出缓冲区（需足够大，建议 src 长度*2+1）
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
发送请求到 DeepSeek API 并获取回复
system_prompt:系统角色设定
user_message:用户输入
reply:输出回复文本
max_len:输出缓冲区长度
*/
int ai_call(const char* system_prompt, const char* user_message,
            char* reply, int max_len)
{
    FILE* fp;
    char* esc_sys;
    char* esc_usr;
    char cmd[2048];
    char line[8192];
    char api_key[64];
    struct SystemConfig cfg;
    int found;

    if(reply==NULL || max_len<=0) return 0;

    /*从配置读取API Key*/
    api_key[0]='\0';
    if(ds_config_load(&cfg)) strcpy(api_key, cfg.api_key);
    if(strlen(api_key)==0) {
        strcpy(reply, "API Key未配置，请联系管理员在系统配置中设置。");
        return 0;
    }

    /*初始化为空*/
    reply[0]='\0';

    /*对系统提示词和用户消息做JSON转义*/
    esc_sys=(char*)malloc(strlen(system_prompt)*2+1);
    esc_usr=(char*)malloc(strlen(user_message)*2+1);
    if(esc_sys==NULL || esc_usr==NULL) {
        if(esc_sys!=NULL) free(esc_sys);
        if(esc_usr!=NULL) free(esc_usr);
        return 0;
    }
    _json_escape(system_prompt, esc_sys);
    _json_escape(user_message, esc_usr);

    /*构建 JSON 请求并写入临时文件*/
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
    fprintf(fp, "\"max_tokens\":8192,");
    fprintf(fp, "\"temperature\":0.7");
    fprintf(fp, "}\n");

    fclose(fp);
    free(esc_sys);
    free(esc_usr);

    /*确保JSON文件已写入磁盘*/
    fflush(NULL);

    /*调用curl发送请求，stderr输出到ai_err.txt便于诊断*/
    sprintf(cmd,
        "curl -s --connect-timeout 15 --max-time 60 -X POST \"%s\" "
        "-H \"Content-Type: application/json\" "
        "-H \"Authorization: Bearer %s\" "
        "-d @%s -o %s 2>data/ai_err.txt",
        API_URL, api_key, REQ_FILE, RESP_FILE);

    system(cmd);

    /*读取响应文件,提取content字段*/
    fp=fopen(RESP_FILE, "r");
    if(fp==NULL) {
        /*尝试读取curl stderr诊断信息*/
        {FILE* efp; efp=fopen("data/ai_err.txt","r");
        if(efp!=NULL){long esize;fseek(efp,0,SEEK_END);esize=ftell(efp);rewind(efp);
        if(esize>0&&esize<max_len-1){fread(reply,1,esize,efp);reply[esize]='\0';}
        else{strcpy(reply,"API响应异常，请稍后重试。");}
        fclose(efp);}
        else{strcpy(reply,"API响应异常，请稍后重试。");}}
        return 0;
    }

    found=0;
    while(fgets(line, sizeof(line), fp)!=NULL) {
        char* p;
        p=strstr(line, "\"content\":\"");
        if(p!=NULL) {
            int ri;
            p+=11;
            ri=0;
            while(*p!='\0' && ri<max_len-1) {
                if(p[0]=='\\' && p[1]=='\"') { reply[ri++]='"'; p+=2; continue; }
                if(p[0]=='\\' && p[1]=='n') { reply[ri++]='\n'; p+=2; continue; }
                if(p[0]=='\\' && p[1]=='\\') { reply[ri++]='\\'; p+=2; continue; }
                if(p[0]=='\"') break;
                reply[ri++]=*p; p++;
            }
            reply[ri]='\0';
            if(strlen(reply)>0) found=1;
        }
    }
    fclose(fp);

    /*回退方案：读整个响应文件查找content*/
    if(!found) {
        long fsize; char* fbuf;
        fp=fopen(RESP_FILE, "r");
        if(fp!=NULL) {
            fseek(fp, 0, SEEK_END); fsize=ftell(fp); rewind(fp);
            if(fsize>0 && fsize<500000) {
                fbuf=(char*)malloc(fsize+1);
                if(fbuf!=NULL) {
                    fread(fbuf, 1, fsize, fp); fbuf[fsize]='\0';
                    {char* p; p=strstr(fbuf, "\"content\":\"");
                    if(p!=NULL){int ri; p+=11; ri=0;
                    while(*p!='\0' && ri<max_len-1) {
                        if(p[0]=='\\' && p[1]=='\"') { reply[ri++]='"'; p+=2; continue; }
                        if(p[0]=='\\' && p[1]=='n') { reply[ri++]='\n'; p+=2; continue; }
                        if(p[0]=='\\' && p[1]=='\\') { reply[ri++]='\\'; p+=2; continue; }
                        if(p[0]=='\"') break;
                        reply[ri++]=*p; p++;
                    }
                    reply[ri]='\0'; found=1;}}
                    free(fbuf);
                }
            }
            fclose(fp);
        }
    }

    /*提取API错误信息*/
    if(!found) {
        fp=fopen(RESP_FILE, "r");
        if(fp!=NULL) {
            while(fgets(line, sizeof(line), fp)!=NULL) {
                char* p; p=strstr(line, "\"message\":\"");
                if(p!=NULL) {int ri; p+=11; ri=0;
                while(*p!='\0' && ri<max_len-1) {
                    if(p[0]=='\\' && p[1]=='\"') { reply[ri++]='"'; p+=2; continue; }
                    if(p[0]=='\\' && p[1]=='n') { reply[ri++]='\n'; p+=2; continue; }
                    if(p[0]=='\"') break;
                    reply[ri++]=*p; p++;
                }
                reply[ri]='\0';
                if(strlen(reply)>0) { fclose(fp); return 0; }}
            }
            fclose(fp);
        }
        /*最后兜底：显示curl的stderr*/
        {FILE* efp; efp=fopen("data/ai_err.txt","r");
        if(efp!=NULL){long esize;fseek(efp,0,SEEK_END);esize=ftell(efp);rewind(efp);
        if(esize>0&&esize<max_len-1){fread(reply,1,esize,efp);reply[esize]='\0';found=0;}
        fclose(efp);}}
    }

    return found ? 1 : 0;
}

/*智能选课推荐*/

int ai_course_recommend(const char* student_id,
                        const char* interests, const char* future_plan,
                        char* reply, int max_len)
{
    struct Student st;
    struct Course courses[MAX_COURSES];
    int count, i;
    char user_msg[16384];
    char course_info[12288];
    char sys_prompt[512];
    int offset, sent;

    /*获取学生信息*/
    if(!ds_student_find_by_id(student_id, &st)) {
        strcpy(reply, "无法获取学生信息。");
        return 0;
    }

    /*获取所有课程*/
    ds_course_load_all(courses, &count);
    if(count==0) {
        strcpy(reply, "当前没有课程数据。");
        return 0;
    }

    /*构建课程信息（仅选课中+匹配专业，最多15门）*/
    course_info[0]='\0';
    offset=0; sent=0;
    for(i=0; i<count && sent<15; i++) {
        char buf[512];
        struct Teacher t; char tname[22];
        const char* type_str;

        if(courses[i].status!=COURSE_STATUS_SELECT) continue;
        if(strstr(courses[i].majors, st.major)==NULL) continue;

        if(ds_teacher_find_by_id(courses[i].teacher_id,&t)) strcpy(tname,t.name);
        else strcpy(tname, courses[i].teacher_id);

        type_str=(courses[i].type==COURSE_TYPE_REQUIRED)?"必修":"选修";

        sprintf(buf, "%d. [%s] %s 学分:%.1f 教师:%s 时段:%s\n",
            sent+1, type_str, courses[i].name, courses[i].credit,
            tname, courses[i].schedule);

        if(offset+(int)strlen(buf) < (int)sizeof(course_info)-10) {
            strcat(course_info, buf);
            offset+=strlen(buf);
        }
        sent++;
    }

    if(sent==0) {
        strcpy(reply, "当前没有适合你专业的可选课程。");
        return 0;
    }

    /*构建用户消息*/
    snprintf(user_msg, sizeof(user_msg),
        "学生: 专业-%s 兴趣-%s 规划-%s\n\n"
        "可选课程(共%d门，仅能从下方列表中推荐):\n%s\n"
        "请从以上%d门课程中挑选最合适的推荐，每门一句话理由。"
        "严禁推荐列表之外的课程。不用Markdown。",
        st.major,
        (interests!=NULL && strlen(interests)>0) ? interests : "未填",
        (future_plan!=NULL && strlen(future_plan)>0) ? future_plan : "未填",
        sent, course_info, sent);

    /*系统提示词*/
    strcpy(sys_prompt,
        "你是大学教务选课助手。只能从用户提供的课程列表中推荐，"
        "绝不编造列表之外的课程。用中文，纯文本，不用Markdown。");

    return ai_call(sys_prompt, user_msg, reply, max_len);
}

/*智能成绩分析*/

int ai_grade_analysis(const char* student_id, char* reply, int max_len)
{
    struct Student st;
    struct Score scores[MAX_SCORES];
    int count, i;
    char user_msg[16384];
    char grade_info[12288];
    char sys_prompt[512];
    int offset, sent;

    /*获取学生信息*/
    if(!ds_student_find_by_id(student_id, &st)) {
        strcpy(reply, "无法获取学生信息。");
        return 0;
    }

    /*获取成绩*/
    ds_score_load_by_student(student_id, scores, &count);
    if(count==0) {
        snprintf(reply, max_len,
            "%s同学，你目前没有已出成绩记录，无法进行分析。",
            st.name);
        return 0;
    }

    /*构建成绩信息（紧凑格式）*/
    grade_info[0]='\0';
    offset=0; sent=0;
    for(i=0; i<count && sent<50; i++) {
        struct Course c;
        char buf[384];
        char cname[102];
        const char* type_str;

        strcpy(cname, scores[i].course_id);
        if(ds_course_find_by_id(scores[i].course_id, &c)) {
            strcpy(cname, c.name);
            type_str=(c.type==COURSE_TYPE_REQUIRED)?"必修":"选修";
        } else type_str="?";

        snprintf(buf, sizeof(buf),
            "%d. [%s]%s 平时%.1f 期末%.1f 总评%.1f 绩点%.2f\n",
            i+1, type_str, cname,
            scores[i].daily_score, scores[i].final_score,
            scores[i].total_score, scores[i].gpa);

        if(offset+(int)strlen(buf) < (int)sizeof(grade_info)-10) {
            strcat(grade_info, buf);
            offset+=strlen(buf);
        }
        sent++;
    }

    /*构建用户消息*/
    snprintf(user_msg, sizeof(user_msg),
        "学生: %s 专业: %s\n\n成绩(%d门):\n%s\n"
        "请分析:1.整体画像 2.优势 3.薄弱 4.建议。纯文本，不用Markdown。",
        st.name, st.major, count, grade_info);

    /*系统提示词*/
    strcpy(sys_prompt,
        "你是学业分析专家。根据成绩数据分析学生表现，"
        "提供画像、优势、薄弱环节和改进建议。用中文，语气积极鼓励。"
        "纯文本回复，不用Markdown符号。");

    return ai_call(sys_prompt, user_msg, reply, max_len);
}
