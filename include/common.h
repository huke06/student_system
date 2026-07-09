/*教务管理系统公共头文件，包含所有数据结构定义和系统常量 */
#ifndef COMMON_H
#define COMMON_H

/*系统常量*/
#define MAX_NAME_LEN 20           /*姓名最大长度*/
#define MAX_ID_LEN 10             /*学号/课程号最大长度 */
#define MAX_PHONE_LEN 15          /*电话号码最大长度*/
#define MAX_MAJOR_LEN 50          /*专业名称最大长度*/
#define MAX_DEPT_LEN 50           /*院系名称最大长度*/
#define MAX_PWD_LEN 32            /*密码（凯撒加密）最大长度*/
#define MAX_SCHEDULE_LEN 200      /*上课时段字符串最大长度*/
#define MAX_SYLLABUS_LEN 500      /*教学大纲最大长度*/
#define MAX_MAJORS_LEN 500        /*专业列表最大长度*/
#define MAX_LOGIN_ATTEMPTS 3      /*最大登录尝试次数*/
#define MAX_STUDENTS    1000      /*学生最大记录数*/
#define MAX_TEACHERS    200       /*教师最大记录数*/
#define MAX_ADMINS      20        /*管理员最大记录数*/
#define MAX_COURSES     500       /*课程最大记录数*/
#define MAX_SELECTIONS  5000      /*选课最大记录数*/
#define MAX_SCORES      5000      /*成绩最大记录数*/

/*数据文件路径*/
#define DATA_DIR        "data"
#define FILE_STUDENTS   "data/students.dat"
#define FILE_TEACHERS   "data/teachers.dat"
#define FILE_ADMINS     "data/admins.dat"
#define FILE_COURSES    "data/courses.dat"
#define FILE_SELECTIONS "data/selections.dat"
#define FILE_SCORES     "data/scores.dat"
#define FILE_CONFIG     "data/config.dat"

/*课程状态*/
#define COURSE_STATUS_DRAFT    0  /*未发布*/
#define COURSE_STATUS_SELECT   1  /*选课中*/
#define COURSE_STATUS_TEACHING 2  /*授课中*/
#define COURSE_STATUS_CLOSED   3  /*已结课*/

/*课程类型*/
#define COURSE_TYPE_REQUIRED 0    /*必修*/
#define COURSE_TYPE_ELECTIVE 1    /*选修*/

/*用户角色*/
#define ROLE_ADMIN   1            /*管理员*/
#define ROLE_TEACHER 2            /*教师*/
#define ROLE_STUDENT 3            /*学生*/

/*数据结构*/
/*学生信息，学号为主键，10位数字*/
struct Student {
    char id[11];                  /*学号*/
    char name[MAX_NAME_LEN+1];   /*姓名*/
    char gender[4];               /*性别*/
    char grade[10];               /*年级*/
    char major[MAX_MAJOR_LEN+1]; /*专业*/
    char phone[MAX_PHONE_LEN+1]; /*联系电话*/
    char password[MAX_PWD_LEN+1];/*密码（凯撒加密）*/
};

/*教师信息，工号为主键，6位数字*/
struct Teacher {
    char id[7];                   /*工号*/
    char name[MAX_NAME_LEN+1];   /*姓名*/
    char gender[4];               /*性别*/
    char department[MAX_DEPT_LEN+1]; /*院系*/
    char phone[MAX_PHONE_LEN+1]; /*联系电话*/
    char password[MAX_PWD_LEN+1];/*密码（凯撒加密）*/
};

/* 管理员信息 */
struct Admin {
    char id[MAX_ID_LEN+1];       /*管理员账号*/
    char name[MAX_NAME_LEN+1];   /*姓名 */
    char password[MAX_PWD_LEN+1];/*密码（凯撒加密）*/
};

/*课程信息，课程号为主键*/
struct Course {
    char id[MAX_ID_LEN+1];       /*课程号*/
    char name[51];                /*课程名称*/
    int type;                     /*0必修 1选修*/
    float credit;                 /*学分*/
    char teacher_id[7];           /*授课教师工号*/
    char majors[MAX_MAJORS_LEN+1];/*适用专业，逗号分隔*/
    char schedule[MAX_SCHEDULE_LEN+1];/*上课时段*/
    int max_students;             /*人数上限*/
    int enrolled;                 /*已选人数*/
    float daily_ratio;            /*平时成绩占比*/
    float final_ratio;            /*期末成绩占比*/
    char syllabus[MAX_SYLLABUS_LEN+1];/*教学大纲*/
    int status;                   /*课程状态*/
    int start_week;               /*起始教学周*/
    int end_week;                 /*结束教学周*/
    char start_date[11];          /*开始日期*/
    char end_date[11];            /*结束日期*/
};

/*选课记录*/
struct Selection {
    int record_id;                /*记录ID */
    char student_id[11];          /*学号*/
    char course_id[MAX_ID_LEN+1];/*课程号*/
    char select_time[20];         /*选课时间*/
    int status;                   /*状态:0已选*/
};

/*成绩记录，学号+课程号唯一*/
struct Score {
    int record_id;                /*记录ID*/
    char student_id[11];          /*学号*/
    char course_id[MAX_ID_LEN+1];/*课程号*/
    float daily_score;            /*平时成绩*/
    float final_score;            /*期末成绩*/
    float total_score;            /*总评成绩*/
    float gpa;                    /*绩点*/
    char record_time[20];         /*录入时间*/
    char teacher_id[7];           /*录入教师工号*/
};

/*系统配置*/
struct SystemConfig {
    char semester_start[11];      /*学期开学日期*/
    char select_start[20];        /*选课开启时间*/
    char select_end[20];          /*选课关闭时间*/
    int select_forced;            /*强制开关:0自动 1开启 2关闭*
    char majors[MAX_MAJORS_LEN+1];/*专业列表*/
};

/*当前登录会话信息*/
struct Session {
    char userid[32];              /*用户账号*/
    char username[64];            /*用户姓名*/
    int role;                     /*角色 ROLE_ADMIN/TEACHER/STUDENT*/
};

#endif /*COMMON_H*/
