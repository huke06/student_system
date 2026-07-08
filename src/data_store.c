/* data_store.c - 数据持久化层实现，纯文本文件存储，管道符分隔，增删改查全量读写模式 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "data_store.h"
#include "logger.h"

/* 内部缓冲区，避免反复在栈上分配大数组 */
static char _line[4096];

/* 内部工具函数 */

/*
 * 创建目录（跨平台）
 * path: 目录路径
 */
static void _make_dir(const char* path)
{
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

/*
 * 获取当前时间字符串，格式 yyyy-mm-dd HH:MM:SS
 */
static void _now_str(char* out)
{
    time_t t;
    struct tm* tm_info;
    time(&t);
    tm_info=localtime(&t);
    sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d",
        tm_info->tm_year+1900, tm_info->tm_mon+1, tm_info->tm_mday,
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
}

/*
 * 判断文件是否存在
 */
static int _file_exists(const char* path)
{
    FILE* fp;
    fp=fopen(path, "r");
    if(fp==NULL) return 0;
    fclose(fp);
    return 1;
}

/*
 * 获取下一个自增 record_id（用于选课和成绩表）
 * filepath: 数据文件路径
 * 返回：max(record_id)+1，若文件为空则返回1
 */
static int _next_record_id(const char* filepath)
{
    FILE* fp;
    int max_id, id;

    fp=fopen(filepath, "r");
    if(fp==NULL) return 1;

    max_id=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        /* 跳过空行和注释 */
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        sscanf(_line, "%d", &id);
        if(id>max_id) max_id=id;
    }
    fclose(fp);
    return max_id+1;
}

/* 种子数据写入 */

/*
 * 写入种子学生数据（5个示例学生）
 */
static void _seed_students(void)
{
    FILE* fp;

    fp=fopen(FILE_STUDENTS, "w");
    if(fp==NULL) return;

    /* 密码统一为 123456，凯撒加密(shift=3)后为 "456789" */
    fprintf(fp, "2024000001|张三|男|2024|计算机科学与技术|13800138001|456789|\n");
    fprintf(fp, "2024000002|李四|女|2024|软件工程|13800138002|456789|\n");
    fprintf(fp, "2024000003|王五|男|2024|数学与应用数学|13800138003|456789|\n");
    fprintf(fp, "2024000004|赵六|女|2024|英语|13800138004|456789|\n");
    fprintf(fp, "2024000005|孙七|男|2024|数据科学|13800138005|456789|\n");

    fclose(fp);
}

/*
 * 写入种子教师数据（2个示例教师）
 */
static void _seed_teachers(void)
{
    FILE* fp;

    fp=fopen(FILE_TEACHERS, "w");
    if(fp==NULL) return;

    fprintf(fp, "100001|张老师|男|计算机学院|13900139001|456789|\n");
    fprintf(fp, "100002|王老师|女|数学学院|13900139002|456789|\n");

    fclose(fp);
}

/*
 * 写入种子管理员数据
 */
static void _seed_admins(void)
{
    FILE* fp;

    fp=fopen(FILE_ADMINS, "w");
    if(fp==NULL) return;

    fprintf(fp, "admin|系统管理员|456789|\n");

    fclose(fp);
}

/*
 * 写入种子课程数据（3个示例课程）
 */
static void _seed_courses(void)
{
    FILE* fp;

    fp=fopen(FILE_COURSES, "w");
    if(fp==NULL) return;

    /* 课程号|名称|类型|学分|教师工号|适用专业|上课时段|上限|已选|平时占比|期末占比|大纲|状态|起始周|结束周|开始日期|结束日期| */
    fprintf(fp, "C001|C语言程序设计|0|4.0|100001|计算机科学与技术,软件工程,数据科学|周一 第1-2节|60|0|0.4|0.6|C语言基础语法、指针、结构体、文件操作|1|1|16|2025-02-24|2025-06-13|\n");
    fprintf(fp, "C002|高等数学|0|5.0|100002|计算机科学与技术,软件工程,数学与应用数学,数据科学|周三 第3-4节|80|0|0.3|0.7|函数极限、导数、积分、级数|1|1|16|2025-02-24|2025-06-13|\n");
    fprintf(fp, "C003|Python入门|1|2.0|100001|计算机科学与技术,软件工程,数据科学,英语|周五 第5-6节|40|0|0.5|0.5|Python基础、数据处理、可视化入门|1|1|8|2025-02-24|2025-04-18|\n");

    fclose(fp);
}

/*
 * 写入种子系统配置
 */
static void _seed_config(void)
{
    FILE* fp;

    fp=fopen(FILE_CONFIG, "w");
    if(fp==NULL) return;

    /* 开学日期|选课开始|选课结束|强制开关|专业列表| */
    fprintf(fp, "2025-02-24|2025-02-24 08:00|2025-03-07 18:00|0|计算机科学与技术,软件工程,数学与应用数学,英语,数据科学|\n");

    fclose(fp);
}

/* 学生数据 CRUD */

/*
 * 从一行文本解析学生记录
 * line: 管道符分隔的一行数据
 * s: 输出结构体
 * 返回：1成功，0失败
 */
static int _parse_student(const char* line, struct Student* s)
{
    /* 学号|姓名|性别|年级|专业|电话|密码| */
    return sscanf(line,
        "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|",
        s->id, s->name, s->gender, s->grade, s->major,
        s->phone, s->password)==7;
}

int ds_student_load_all(struct Student* students, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_STUDENTS, "r");
    if(fp==NULL) {
        *count=0;
        return 0;
    }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_STUDENTS) {
        /* 跳过空行和注释 */
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_student(_line, &students[n])) {
            n++;
        }
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_student_find_by_id(const char* id, struct Student* out)
{
    FILE* fp;
    struct Student s;

    fp=fopen(FILE_STUDENTS, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_student(_line, &s)) {
            if(strcmp(s.id, id)==0) {
                *out=s;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_student_add(const struct Student* s)
{
    FILE* fp;

    /* 先检查是否已存在 */
    {
        struct Student tmp;
        if(ds_student_find_by_id(s->id, &tmp)) {
            return 0; /* 学号重复 */
        }
    }

    fp=fopen(FILE_STUDENTS, "a");
    if(fp==NULL) return 0;

    fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|\n",
        s->id, s->name, s->gender, s->grade, s->major,
        s->phone, s->password);
    fclose(fp);

    /* 记录日志 */
    {
        char info[256];
        sprintf(info, "新增学生: %s(%s)", s->id, s->name);
        log_write(LOG_TYPE_ADD, "系统", info);
    }

    return 1;
}

int ds_student_update(const char* id, const struct Student* new_s)
{
    struct Student students[MAX_STUDENTS];
    int count, i, found;
    FILE* fp;

    if(!ds_student_load_all(students, &count)) return 0;

    found=0;
    for(i=0; i<count; i++) {
        if(strcmp(students[i].id, id)==0) {
            students[i]=*new_s;
            found=1;
            break;
        }
    }
    if(!found) return 0;

    /* 全量写回 */
    fp=fopen(FILE_STUDENTS, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|\n",
            students[i].id, students[i].name, students[i].gender,
            students[i].grade, students[i].major,
            students[i].phone, students[i].password);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "修改学生信息: %s(%s)", id, new_s->name);
        log_write(LOG_TYPE_MODIFY, "系统", info);
    }

    return 1;
}

int ds_student_delete(const char* id)
{
    struct Student students[MAX_STUDENTS];
    int count, i;
    FILE* fp;

    if(!ds_student_load_all(students, &count)) return 0;

    fp=fopen(FILE_STUDENTS, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        if(strcmp(students[i].id, id)==0) continue; /* 跳过要删除的 */
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|\n",
            students[i].id, students[i].name, students[i].gender,
            students[i].grade, students[i].major,
            students[i].phone, students[i].password);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "删除学生: %s", id);
        log_write(LOG_TYPE_DELETE, "系统", info);
    }

    return 1;
}

/* 教师数据 CRUD */

static int _parse_teacher(const char* line, struct Teacher* t)
{
    /* 工号|姓名|性别|院系|电话|密码| */
    return sscanf(line,
        "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|",
        t->id, t->name, t->gender, t->department,
        t->phone, t->password)==6;
}

int ds_teacher_load_all(struct Teacher* teachers, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_TEACHERS, "r");
    if(fp==NULL) { *count=0; return 0; }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_TEACHERS) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_teacher(_line, &teachers[n])) n++;
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_teacher_find_by_id(const char* id, struct Teacher* out)
{
    FILE* fp;
    struct Teacher t;

    fp=fopen(FILE_TEACHERS, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_teacher(_line, &t)) {
            if(strcmp(t.id, id)==0) {
                *out=t;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_teacher_add(const struct Teacher* t)
{
    FILE* fp;
    {
        struct Teacher tmp;
        if(ds_teacher_find_by_id(t->id, &tmp)) return 0;
    }

    fp=fopen(FILE_TEACHERS, "a");
    if(fp==NULL) return 0;

    fprintf(fp, "%s|%s|%s|%s|%s|%s|\n",
        t->id, t->name, t->gender, t->department,
        t->phone, t->password);
    fclose(fp);

    {
        char info[256];
        sprintf(info, "新增教师: %s(%s)", t->id, t->name);
        log_write(LOG_TYPE_ADD, "系统", info);
    }
    return 1;
}

int ds_teacher_update(const char* id, const struct Teacher* new_t)
{
    struct Teacher teachers[MAX_TEACHERS];
    int count, i, found;
    FILE* fp;

    if(!ds_teacher_load_all(teachers, &count)) return 0;

    found=0;
    for(i=0; i<count; i++) {
        if(strcmp(teachers[i].id, id)==0) {
            teachers[i]=*new_t;
            found=1;
            break;
        }
    }
    if(!found) return 0;

    fp=fopen(FILE_TEACHERS, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|\n",
            teachers[i].id, teachers[i].name, teachers[i].gender,
            teachers[i].department, teachers[i].phone, teachers[i].password);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "修改教师信息: %s(%s)", id, new_t->name);
        log_write(LOG_TYPE_MODIFY, "系统", info);
    }
    return 1;
}

int ds_teacher_delete(const char* id)
{
    struct Teacher teachers[MAX_TEACHERS];
    int count, i;
    FILE* fp;

    if(!ds_teacher_load_all(teachers, &count)) return 0;

    fp=fopen(FILE_TEACHERS, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        if(strcmp(teachers[i].id, id)==0) continue;
        fprintf(fp, "%s|%s|%s|%s|%s|%s|\n",
            teachers[i].id, teachers[i].name, teachers[i].gender,
            teachers[i].department, teachers[i].phone, teachers[i].password);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "删除教师: %s", id);
        log_write(LOG_TYPE_DELETE, "系统", info);
    }
    return 1;
}

/* 管理员数据 CRUD */

static int _parse_admin(const char* line, struct Admin* a)
{
    /* 账号|姓名|密码| */
    return sscanf(line, "%[^|]|%[^|]|%[^|]|", a->id, a->name, a->password)==3;
}

int ds_admin_load_all(struct Admin* admins, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_ADMINS, "r");
    if(fp==NULL) { *count=0; return 0; }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_ADMINS) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_admin(_line, &admins[n])) n++;
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_admin_find_by_id(const char* id, struct Admin* out)
{
    FILE* fp;
    struct Admin a;

    fp=fopen(FILE_ADMINS, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_admin(_line, &a)) {
            if(strcmp(a.id, id)==0) {
                *out=a;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_admin_add(const struct Admin* a)
{
    FILE* fp;
    {
        struct Admin tmp;
        if(ds_admin_find_by_id(a->id, &tmp)) return 0;
    }

    fp=fopen(FILE_ADMINS, "a");
    if(fp==NULL) return 0;

    fprintf(fp, "%s|%s|%s|\n", a->id, a->name, a->password);
    fclose(fp);

    {
        char info[256];
        sprintf(info, "新增管理员: %s(%s)", a->id, a->name);
        log_write(LOG_TYPE_ADD, "系统", info);
    }
    return 1;
}

int ds_admin_update(const char* id, const struct Admin* new_a)
{
    struct Admin admins[MAX_ADMINS];
    int count, i, found;
    FILE* fp;

    if(!ds_admin_load_all(admins, &count)) return 0;

    found=0;
    for(i=0; i<count; i++) {
        if(strcmp(admins[i].id, id)==0) {
            admins[i]=*new_a;
            found=1;
            break;
        }
    }
    if(!found) return 0;

    fp=fopen(FILE_ADMINS, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        fprintf(fp, "%s|%s|%s|\n", admins[i].id, admins[i].name, admins[i].password);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "修改管理员信息: %s", id);
        log_write(LOG_TYPE_MODIFY, "系统", info);
    }
    return 1;
}

int ds_admin_delete(const char* id)
{
    struct Admin admins[MAX_ADMINS];
    int count, i;
    FILE* fp;

    if(!ds_admin_load_all(admins, &count)) return 0;

    fp=fopen(FILE_ADMINS, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        if(strcmp(admins[i].id, id)==0) continue;
        fprintf(fp, "%s|%s|%s|\n", admins[i].id, admins[i].name, admins[i].password);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "删除管理员: %s", id);
        log_write(LOG_TYPE_DELETE, "系统", info);
    }
    return 1;
}

/* 课程数据 CRUD */

static int _parse_course(const char* line, struct Course* c)
{
    char type_buf[8];
    char credit_buf[16];
    char max_buf[8], enr_buf[8];
    char dr_buf[8], fr_buf[8];
    char status_buf[8], sw_buf[8], ew_buf[8];
    int ret;

    /* 课程号|名称|类型|学分|教师工号|适用专业|上课时段|上限|已选|平时占比|期末占比|大纲|状态|起始周|结束周|开始日期|结束日期| */
    ret=sscanf(line,
        "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|",
        c->id, c->name, type_buf, credit_buf, c->teacher_id,
        c->majors, c->schedule, max_buf, enr_buf, dr_buf, fr_buf,
        c->syllabus, status_buf, sw_buf, ew_buf,
        c->start_date, c->end_date);
    if(ret!=17) return 0;

    c->type=atoi(type_buf);
    c->credit=(float)atof(credit_buf);
    c->max_students=atoi(max_buf);
    c->enrolled=atoi(enr_buf);
    c->daily_ratio=(float)atof(dr_buf);
    c->final_ratio=(float)atof(fr_buf);
    c->status=atoi(status_buf);
    c->start_week=atoi(sw_buf);
    c->end_week=atoi(ew_buf);

    return 1;
}

int ds_course_load_all(struct Course* courses, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_COURSES, "r");
    if(fp==NULL) { *count=0; return 0; }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_COURSES) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_course(_line, &courses[n])) n++;
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_course_find_by_id(const char* id, struct Course* out)
{
    FILE* fp;
    struct Course c;

    fp=fopen(FILE_COURSES, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_course(_line, &c)) {
            if(strcmp(c.id, id)==0) {
                *out=c;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_course_add(const struct Course* c)
{
    FILE* fp;
    {
        struct Course tmp;
        if(ds_course_find_by_id(c->id, &tmp)) return 0;
    }

    fp=fopen(FILE_COURSES, "a");
    if(fp==NULL) return 0;

    fprintf(fp, "%s|%s|%d|%.1f|%s|%s|%s|%d|%d|%.1f|%.1f|%s|%d|%d|%d|%s|%s|\n",
        c->id, c->name, c->type, c->credit, c->teacher_id,
        c->majors, c->schedule, c->max_students, c->enrolled,
        c->daily_ratio, c->final_ratio, c->syllabus,
        c->status, c->start_week, c->end_week,
        c->start_date, c->end_date);
    fclose(fp);

    {
        char info[256];
        sprintf(info, "新增课程: %s(%s)", c->id, c->name);
        log_write(LOG_TYPE_ADD, "系统", info);
    }
    return 1;
}

int ds_course_update(const char* id, const struct Course* new_c)
{
    static struct Course courses[MAX_COURSES];
    int count, i, found;
    FILE* fp;

    if(!ds_course_load_all(courses, &count)) return 0;

    found=0;
    for(i=0; i<count; i++) {
        if(strcmp(courses[i].id, id)==0) {
            courses[i]=*new_c;
            found=1;
            break;
        }
    }
    if(!found) return 0;

    fp=fopen(FILE_COURSES, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        fprintf(fp, "%s|%s|%d|%.1f|%s|%s|%s|%d|%d|%.1f|%.1f|%s|%d|%d|%d|%s|%s|\n",
            courses[i].id, courses[i].name, courses[i].type, courses[i].credit,
            courses[i].teacher_id, courses[i].majors, courses[i].schedule,
            courses[i].max_students, courses[i].enrolled,
            courses[i].daily_ratio, courses[i].final_ratio, courses[i].syllabus,
            courses[i].status, courses[i].start_week, courses[i].end_week,
            courses[i].start_date, courses[i].end_date);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "修改课程信息: %s(%s)", id, new_c->name);
        log_write(LOG_TYPE_MODIFY, "系统", info);
    }
    return 1;
}

int ds_course_delete(const char* id)
{
    static struct Course courses[MAX_COURSES];
    int count, i;
    FILE* fp;

    if(!ds_course_load_all(courses, &count)) return 0;

    fp=fopen(FILE_COURSES, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        if(strcmp(courses[i].id, id)==0) continue;
        fprintf(fp, "%s|%s|%d|%.1f|%s|%s|%s|%d|%d|%.1f|%.1f|%s|%d|%d|%d|%s|%s|\n",
            courses[i].id, courses[i].name, courses[i].type, courses[i].credit,
            courses[i].teacher_id, courses[i].majors, courses[i].schedule,
            courses[i].max_students, courses[i].enrolled,
            courses[i].daily_ratio, courses[i].final_ratio, courses[i].syllabus,
            courses[i].status, courses[i].start_week, courses[i].end_week,
            courses[i].start_date, courses[i].end_date);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "删除课程: %s", id);
        log_write(LOG_TYPE_DELETE, "系统", info);
    }
    return 1;
}

/* 选课记录 CRUD */

static int _parse_selection(const char* line, struct Selection* s)
{
    char rid_buf[16], status_buf[8];
    int ret;

    /* 记录ID|学号|课程号|选课时间|状态| */
    ret=sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|",
        rid_buf, s->student_id, s->course_id, s->select_time, status_buf);
    if(ret!=5) return 0;

    s->record_id=atoi(rid_buf);
    s->status=atoi(status_buf);
    return 1;
}

int ds_selection_load_all(struct Selection* selections, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_SELECTIONS, "r");
    if(fp==NULL) { *count=0; return 0; }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_SELECTIONS) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_selection(_line, &selections[n])) n++;
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_selection_find_by_id(int record_id, struct Selection* out)
{
    FILE* fp;
    struct Selection s;

    fp=fopen(FILE_SELECTIONS, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_selection(_line, &s)) {
            if(s.record_id==record_id) {
                *out=s;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_selection_find_by_sc(const char* student_id, const char* course_id,
                            struct Selection* out)
{
    FILE* fp;
    struct Selection s;

    fp=fopen(FILE_SELECTIONS, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_selection(_line, &s)) {
            if(strcmp(s.student_id, student_id)==0 &&
               strcmp(s.course_id, course_id)==0) {
                *out=s;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_selection_load_by_student(const char* student_id,
                                  struct Selection* selections, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_SELECTIONS, "r");
    if(fp==NULL) { *count=0; return 0; }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_SELECTIONS) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_selection(_line, &selections[n])) {
            if(strcmp(selections[n].student_id, student_id)==0) {
                n++;
            }
        }
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_selection_add(const struct Selection* s)
{
    FILE* fp;

    /* 检查是否已选过该课程 */
    {
        struct Selection tmp;
        if(ds_selection_find_by_sc(s->student_id, s->course_id, &tmp)) {
            return 0; /* 已选过 */
        }
    }

    fp=fopen(FILE_SELECTIONS, "a");
    if(fp==NULL) return 0;

    fprintf(fp, "%d|%s|%s|%s|%d|\n",
        s->record_id, s->student_id, s->course_id,
        s->select_time, s->status);
    fclose(fp);

    {
        char info[256];
        sprintf(info, "选课: 学生%s 课程%s", s->student_id, s->course_id);
        log_write(LOG_TYPE_ADD, "系统", info);
    }
    return 1;
}

int ds_selection_delete(int record_id)
{
    struct Selection selections[MAX_SELECTIONS];
    int count, i;
    FILE* fp;

    if(!ds_selection_load_all(selections, &count)) return 0;

    fp=fopen(FILE_SELECTIONS, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        if(selections[i].record_id==record_id) continue;
        fprintf(fp, "%d|%s|%s|%s|%d|\n",
            selections[i].record_id, selections[i].student_id,
            selections[i].course_id, selections[i].select_time,
            selections[i].status);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "退选: 记录ID %d", record_id);
        log_write(LOG_TYPE_DELETE, "系统", info);
    }
    return 1;
}

/* 成绩记录 CRUD */

static int _parse_score(const char* line, struct Score* s)
{
    char rid_buf[16], ds_buf[16], fs_buf[16], ts_buf[16], gpa_buf[16];
    int ret;

    /* 记录ID|学号|课程号|平时成绩|期末成绩|总评|绩点|录入时间|教师工号| */
    ret=sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|",
        rid_buf, s->student_id, s->course_id,
        ds_buf, fs_buf, ts_buf, gpa_buf,
        s->record_time, s->teacher_id);
    if(ret!=9) return 0;

    s->record_id=atoi(rid_buf);
    s->daily_score=(float)atof(ds_buf);
    s->final_score=(float)atof(fs_buf);
    s->total_score=(float)atof(ts_buf);
    s->gpa=(float)atof(gpa_buf);
    return 1;
}

int ds_score_load_all(struct Score* scores, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_SCORES, "r");
    if(fp==NULL) { *count=0; return 0; }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_SCORES) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_score(_line, &scores[n])) n++;
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_score_find_by_id(int record_id, struct Score* out)
{
    FILE* fp;
    struct Score s;

    fp=fopen(FILE_SCORES, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_score(_line, &s)) {
            if(s.record_id==record_id) {
                *out=s;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_score_find_by_sc(const char* student_id, const char* course_id,
                        struct Score* out)
{
    FILE* fp;
    struct Score s;

    fp=fopen(FILE_SCORES, "r");
    if(fp==NULL) return 0;

    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_score(_line, &s)) {
            if(strcmp(s.student_id, student_id)==0 &&
               strcmp(s.course_id, course_id)==0) {
                *out=s;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

int ds_score_load_by_student(const char* student_id,
                              struct Score* scores, int* count)
{
    FILE* fp;
    int n;

    fp=fopen(FILE_SCORES, "r");
    if(fp==NULL) { *count=0; return 0; }

    n=0;
    while(fgets(_line, sizeof(_line), fp)!=NULL && n<MAX_SCORES) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_score(_line, &scores[n])) {
            if(strcmp(scores[n].student_id, student_id)==0) {
                n++;
            }
        }
    }
    fclose(fp);
    *count=n;
    return 1;
}

int ds_score_add(const struct Score* s)
{
    FILE* fp;

    /* 检查是否已有该学生该课程的成绩 */
    {
        struct Score tmp;
        if(ds_score_find_by_sc(s->student_id, s->course_id, &tmp)) {
            return 0; /* 已有成绩记录 */
        }
    }

    fp=fopen(FILE_SCORES, "a");
    if(fp==NULL) return 0;

    fprintf(fp, "%d|%s|%s|%.1f|%.1f|%.1f|%.2f|%s|%s|\n",
        s->record_id, s->student_id, s->course_id,
        s->daily_score, s->final_score, s->total_score, s->gpa,
        s->record_time, s->teacher_id);
    fclose(fp);

    {
        char info[256];
        sprintf(info, "录入成绩: 学生%s 课程%s 总评%.1f",
            s->student_id, s->course_id, s->total_score);
        log_write(LOG_TYPE_ADD, "系统", info);
    }
    return 1;
}

int ds_score_update(int record_id, const struct Score* new_s)
{
    struct Score scores[MAX_SCORES];
    int count, i, found;
    FILE* fp;

    if(!ds_score_load_all(scores, &count)) return 0;

    found=0;
    for(i=0; i<count; i++) {
        if(scores[i].record_id==record_id) {
            scores[i]=*new_s;
            found=1;
            break;
        }
    }
    if(!found) return 0;

    fp=fopen(FILE_SCORES, "w");
    if(fp==NULL) return 0;

    for(i=0; i<count; i++) {
        fprintf(fp, "%d|%s|%s|%.1f|%.1f|%.1f|%.2f|%s|%s|\n",
            scores[i].record_id, scores[i].student_id, scores[i].course_id,
            scores[i].daily_score, scores[i].final_score, scores[i].total_score,
            scores[i].gpa, scores[i].record_time, scores[i].teacher_id);
    }
    fclose(fp);

    {
        char info[256];
        sprintf(info, "修改成绩: 记录ID %d", record_id);
        log_write(LOG_TYPE_MODIFY, "系统", info);
    }
    return 1;
}

/* 系统配置操作 */

static int _parse_config(const char* line, struct SystemConfig* cfg)
{
    char forced_buf[8];
    int ret;

    /* 开学日期|选课开始|选课结束|强制开关|专业列表| */
    ret=sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|",
        cfg->semester_start, cfg->select_start, cfg->select_end,
        forced_buf, cfg->majors);
    if(ret!=5) return 0;

    cfg->select_forced=atoi(forced_buf);
    return 1;
}

int ds_config_load(struct SystemConfig* cfg)
{
    FILE* fp;

    fp=fopen(FILE_CONFIG, "r");
    if(fp==NULL) return 0;

    /* 只读第一条有效记录 */
    while(fgets(_line, sizeof(_line), fp)!=NULL) {
        if(_line[0]=='\n' || _line[0]=='\r' || _line[0]=='#') continue;
        if(_parse_config(_line, cfg)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int ds_config_save(const struct SystemConfig* cfg)
{
    FILE* fp;

    fp=fopen(FILE_CONFIG, "w");
    if(fp==NULL) return 0;

    fprintf(fp, "%s|%s|%s|%d|%s|\n",
        cfg->semester_start, cfg->select_start, cfg->select_end,
        cfg->select_forced, cfg->majors);
    fclose(fp);
    return 1;
}

/* 系统初始化 */

/* 初始化数据存储系统，检查data目录和数据文件是否存在，首次运行自动创建并写入种子数据 */
int ds_init(void)
{
    int need_seed;

    /* 检查 data 目录 */
    need_seed=0;
    if(!_file_exists(FILE_STUDENTS)) need_seed=1;

    if(need_seed) {
        /* 创建 data 目录 */
        _make_dir(DATA_DIR);

        /* 写入种子数据 */
        _seed_students();
        _seed_teachers();
        _seed_admins();
        _seed_courses();
        _seed_config();

        /* 创建空的选课和成绩文件 */
        {
            FILE* fp;
            fp=fopen(FILE_SELECTIONS, "w");
            if(fp!=NULL) fclose(fp);
            fp=fopen(FILE_SCORES, "w");
            if(fp!=NULL) fclose(fp);
        }

        log_write(LOG_TYPE_SYSTEM, "系统", "数据存储初始化完成，已写入种子数据");
        return 1;
    }

    return 1;
}
