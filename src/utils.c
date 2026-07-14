/* 工具函数实现：字符串处理、ID校验、密码生成、成绩计算、排课冲突、周次日期推算、凯撒加密与解密 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "utils.h"
#include "data_store.h"
/*字符串处理*/

/*去除字符串首尾空格和换行符
原地修改，将尾部空白字符截断，头部空格通过memmove前移*/
void str_trim(char* str)
{
    int len, start, end, i;
    if(str==NULL || strlen(str)==0) return;
    len=strlen(str);
    /*找第一个非空白字符位置*/
    start=0;
    while(start<len && (str[start]==' '||str[start]=='\t'
           ||str[start]=='\n'||str[start]=='\r')) {
        start++;
    }
    /*找最后一个非空白字符位置*/
    end=len-1;
    while(end>=start && (str[end]==' '||str[end]=='\t'
           ||str[end]=='\n'||str[end]=='\r')) {
        end--;
    }
    /*将有效字符前移*/
    if(start>0) {
        for(i=0; i<=end-start; i++) {
            str[i]=str[start+i];
        }
    }
    str[end-start+1]='\0';
}

/*ID与号码校验*/
/*校验学号：必须恰好10位数字*/
int check_student_id(const char* id)
{
    int i;

    if(id==NULL) return 0;
    if(strlen(id)!=10) return 0;

    for(i=0; i<10; i++) {
        if(id[i]<'0' || id[i]>'9') return 0;
    }
    return 1;
}

/*校验教师工号：必须恰好6位数字*/
int check_teacher_id(const char* id)
{
    int i;

    if(id==NULL) return 0;
    if(strlen(id)!=6) return 0;

    for(i=0; i<6; i++) {
        if(id[i]<'0' || id[i]>'9') return 0;
    }
    return 1;
}

/*校验中国大陆手机号：11位数字，1开头，第二位3-9*/
int check_phone(const char* phone)
{
    int i;

    if(phone==NULL) return 0;
    if(strlen(phone)!=11) return 0;
    if(phone[0]!='1') return 0;
    if(phone[1]<'3' || phone[1]>'9') return 0;

    for(i=2; i<11; i++) {
        if(phone[i]<'0' || phone[i]>'9') return 0;
    }
    return 1;
}

/*初始密码生成*/

/*根据账号生成初始密码，取后6位
 如果账号不足6位，右侧补0*/
void gen_init_pwd(const char* id, char* pwd)
{
    int len, i, start;

    len=strlen(id);

    if(len>=6) {
        /*取后6位*/
        start=len-6;
        for(i=0; i<6; i++) {
            pwd[i]=id[start+i];
        }
    } else {
        /*不足6位，先复制全部再补0*/
        for(i=0; i<len; i++) {
            pwd[i]=id[i];
        }
        for(i=len; i<6; i++) {
            pwd[i]='0';
        }
    }
    pwd[6]='\0';
}

/* 成绩计算 */

/*计算总评成绩 = 平时成绩*平时占比 + 期末成绩*期末占比
结果四舍五入保留1位小数*/
float calc_total_score(float daily_score, float final_score,
                       float daily_ratio, float final_ratio)
{
    float result;

    result=daily_score*daily_ratio + final_score*final_ratio;

    /*四舍五入保留1位小数*/
    result=(float)((int)(result*10+0.5))/10.0f;

    return result;
}

/*百分制成绩转4分制绩点*/
float score_to_gpa(float score)
{
    if(score>=90.0f) return 4.0f;
    if(score>=85.0f) return 3.7f;
    if(score>=82.0f) return 3.3f;
    if(score>=78.0f) return 3.0f;
    if(score>=75.0f) return 2.7f;
    if(score>=72.0f) return 2.3f;
    if(score>=68.0f) return 2.0f;
    if(score>=64.0f) return 1.5f;
    if(score>=60.0f) return 1.0f;
    return 0.0f;
}

/*计算加权平均成绩 = Σ(成绩*学分) / Σ学分*/
float calc_weighted_avg(const float* scores, const float* credits, int count)
{
    float sum_weighted, sum_credits;
    int i;

    if(count<=0) return 0.0f;

    sum_weighted=0.0f;
    sum_credits=0.0f;

    for(i=0; i<count; i++) {
        sum_weighted+=scores[i]*credits[i];
        sum_credits+=credits[i];
    }

    if(sum_credits==0.0f) return 0.0f;

    /*保留1位小数*/
    return (float)((int)(sum_weighted/sum_credits*10+0.5))/10.0f;
}

/*计算加权平均绩点 = Σ(绩点*学分) / Σ学分*/
float calc_weighted_gpa(const float* gpas, const float* credits, int count)
{
    float sum_weighted, sum_credits;
    int i;

    if(count<=0) return 0.0f;

    sum_weighted=0.0f;
    sum_credits=0.0f;

    for(i=0; i<count; i++) {
        sum_weighted+=gpas[i]*credits[i];
        sum_credits+=credits[i];
    }

    if(sum_credits==0.0f) return 0.0f;

    /*保留2位小数*/
    return (float)((int)(sum_weighted/sum_credits*100+0.5))/100.0f;
}

/* 选课冲突判定 */

/*
星期名称到数字的映射
支持 "周一"~"周日" 和 "星期一"~"星期日"
 */
static int weekday_to_num(const char* wd)
{
    if(strstr(wd,"一")!=NULL) return 1;
    if(strstr(wd,"二")!=NULL) return 2;
    if(strstr(wd,"三")!=NULL) return 3;
    if(strstr(wd,"四")!=NULL) return 4;
    if(strstr(wd,"五")!=NULL) return 5;
    if(strstr(wd,"六")!=NULL) return 6;
    if(strstr(wd,"日")!=NULL||strstr(wd,"天")!=NULL) return 7;
    return 0;
}

/*
解析上课时段字符串
支持格式：
"周一 第1-2节" -> weekday=1, start=1, end=2
"周三 第5-7节" -> weekday=3, start=5, end=7
"第1-2节"-> weekday=1, start=1, end=2
 */
int parse_schedule(const char* schedule, int* weekday,
                   int* start_period, int* end_period)
{
    char buf[100];
    char* p_di;     /*指向"第"字的位置*/
    char wd_str[10];
    int wd_len, i;

    if(schedule==NULL) return 0;

    strcpy(buf, schedule);
    str_trim(buf);
    if(strlen(buf)==0) return 0;

    /*1.找到"第"字的位置*/
    p_di=strstr(buf, "\xe7\xac\xac"); /*UTF-8编码的"第"*/
    if(p_di==NULL) {
        /*尝试GBK编码*/
        p_di=strstr(buf, "\xb5\xda");
    }
    if(p_di==NULL) return 0;

    /*2."第"之前的部分就是星期*/
    wd_len=(int)(p_di-buf);
    if(wd_len>9) wd_len=9;
    for(i=0; i<wd_len; i++) wd_str[i]=buf[i];
    wd_str[wd_len]='\0';
    *weekday=weekday_to_num(wd_str);
    if(*weekday==0) return 0;

    /*3. 跳过"第"，解析起始节次*/
    p_di+=3; /*"第"占3字节(UTF-8)*/
    *start_period=0;
    while(*p_di>='0' && *p_di<='9') {
        *start_period=(*start_period)*10+(*p_di-'0');
        p_di++;
    }
    if(*start_period==0) return 0;

    /*4. 跳过'-'*/
    if(*p_di=='-') p_di++;

    /*5.解析结束节次*/
    *end_period=0;
    while(*p_di>='0' && *p_di<='9') {
        *end_period=(*end_period)*10+(*p_di-'0');
        p_di++;
    }
    if(*end_period==0) {
        *end_period=*start_period;
    }

    return 1;
}

/*
判断两门课程是否存在时间冲突
冲突条件（需同时满足）：
① 星期相同
② 节次有重叠（start1<=end2 且 start2<=end1）
③ 周次有重叠（w1_start<=w2_end 且 w2_start<=w1_end）
*/
int check_time_conflict(const char* sched1, int w1_start, int w1_end,
                        const char* sched2, int w2_start, int w2_end)
{
    int wd1, sp1, ep1;
    int wd2, sp2, ep2;

    /*解析两个上课时段*/
    if(!parse_schedule(sched1, &wd1, &sp1, &ep1)) return 0;
    if(!parse_schedule(sched2, &wd2, &sp2, &ep2)) return 0;

    /*条件1：星期相同*/
    if(wd1!=wd2) return 0;

    /*条件2：节次重叠*/
    if(sp1>ep2 || sp2>ep1) return 0;

    /*条件3：周次重叠*/
    if(w1_start>w2_end || w2_start>w1_end) return 0;

    /*三个条件都满足，存在冲突*/
    return 1;
}

/*周次与日期对应*/

/*将"yyyy-mm-dd"格式日期字符串拆分为年、月、日*/
static void parse_date(const char* date_str, int* year, int* month, int* day)
{
    sscanf(date_str, "%d-%d-%d", year, month, day);
}

/*判断是否为闰年*/
static int is_leap_year(int year)
{
    return (year%4==0 && year%100!=0) || (year%400==0);
}

/*每月天数*/
static int days_in_month(int year, int month)
{
    int days[]={0,31,28,31,30,31,30,31,31,30,31,30,31};

    if(month==2 && is_leap_year(year)) return 29;
    return days[month];
}

/*日期加上指定天数;y/m/d: 输入日期，原地修改;add_days: 增加的天数*/
static void add_days(int* year, int* month, int* day, int add_days)
{
    int d;

    *day+=add_days;

    while(*day > days_in_month(*year, *month)) {
        d=days_in_month(*year, *month);
        *day-=d;
        (*month)++;
        if(*month>12) {
            *month=1;
            (*year)++;
        }
    }
    /*处理负天数的情况*/
    while(*day < 1) {
        (*month)--;
        if(*month<1) {
            *month=12;
            (*year)--;
        }
        *day+=days_in_month(*year, *month);
    }
}

/*根据学期开学日期和第N周，推算该周周一的日期
第1周的周一就是开学日期本身
第N周的周一=开学日期 + (N-1)*7 天*/
void calc_week_start_date(const char* semester_start, int week_num,
                          char* date_out)
{
    int year, month, day;
    int offset;

    if(week_num<1) week_num=1;

    parse_date(semester_start, &year, &month, &day);

    offset=(week_num-1)*7;
    add_days(&year, &month, &day, offset);

    sprintf(date_out, "%04d-%02d-%02d", year, month, day);
}

/*判断一个日期是否在指定周次范围内
思路：
第N周的周一是开学日期+(N-1)*7天
第N周的周日是开学日期+(N-1)*7+6天
判断给定日期是否在[start_week周一, end_week周日]之间*/
int date_in_week_range(const char* date, const char* semester_start,                int start_week, int end_week)
{
    int y, m, d;
    int sy, sm, sd;
    int date_offset, start_offset, end_offset;

    parse_date(date, &y, &m, &d);
    parse_date(semester_start, &sy, &sm, &sd);

    /*计算给定日期距离开学日期的天数
    简化处理：将两个日期都转为"从某起点算起的天数"再求差*/
    {
        /*计算date距离公元元年的天数（粗略）*/
        int days1, days2;
        days1=0;
        {
            int i, yy;
            yy=y-1;
            days1=yy*365 + yy/4 - yy/100 + yy/400;
            for(i=1; i<m; i++) days1+=days_in_month(y, i);
            days1+=d;
        }

        days2=0;
        {
            int i, yy;
            yy=sy-1;
            days2=yy*365 + yy/4 - yy/100 + yy/400;
            for(i=1; i<sm; i++) days2+=days_in_month(sy, i);
            days2+=sd;
        }

        date_offset=days1-days2; /*距离开学日的偏移天数*/
    }

    /*start_week周一距离开学日的偏移天数*/
    start_offset=(start_week-1)*7;
    /*end_week周日距离开学日的偏移天数*/
    end_offset=(end_week-1)*7+6;

    return (date_offset>=start_offset && date_offset<=end_offset);
}

/*凯撒密码加密——字母A-Z/a-z循环移位，数字0-9循环移位，其他字符不变*/
void caesar_encrypt(const char* input, int shift, char* output)
{
    int i;
    char ch;
    int base, range;

    /*限定偏移量范围*/
    shift=shift%26;
    if(shift<0) shift+=26;

    i=0;
    while(input[i]!='\0') {
        ch=input[i];

        if(ch>='A' && ch<='Z') {
            /*大写字母*/
            base='A'; range=26;
            ch=(ch-base+shift)%range+base;
        } else if(ch>='a' && ch<='z') {
            /*小写字母*/
            base='a'; range=26;
            ch=(ch-base+shift)%range+base;
        } else if(ch>='0' && ch<='9') {
            /*数字*/
            base='0'; range=10;
            ch=(ch-base+shift)%range+base;
        }
        /*其他字符保持不变*/

        output[i]=ch;
        i++;
    }
    output[i]='\0';
}

/*凯撒密码解密，逐字符反向移位*/
void caesar_decrypt(const char* input, int shift, char* output)
{
    int i;
    char ch;
    int base, range;

    i=0;
    while(input[i]!='\0') {
        ch=input[i];

        if(ch>='A' && ch<='Z') {
            base='A'; range=26;
            shift=shift%26;
            ch=(ch-base-shift+range)%range+base;
        } else if(ch>='a' && ch<='z') {
            base='a'; range=26;
            shift=shift%26;
            ch=(ch-base-shift+range)%range+base;
        } else if(ch>='0' && ch<='9') {
            base='0'; range=10;
            shift=shift%10;
            ch=(ch-base-shift+range)%range+base;
        }

        output[i]=ch;
        i++;
    }
    output[i]='\0';
}

/*==== 控制台交互工具 ====*/

/*清屏并打印系统标题栏*/
void print_header(const char* title)
{
    system("cls");
    printf("========================================\n");
    printf("        教务管理系统\n");
    if(title!=NULL && strlen(title)>0) {
        printf("        %s\n", title);
    }
    printf("========================================\n\n");
}

/*读取整数选项，带范围校验*/
int get_choice(const char* prompt, int min, int max)
{
    int choice;
    char buf[32];
    while(1) {
        printf("%s", prompt);
        fgets(buf, sizeof(buf), stdin);
        if(sscanf(buf, "%d", &choice)==1) {
            if(choice>=min && choice<=max) {
                return choice;
            }
        }
        printf("  [提示] 请输入 %d~%d 之间的数字\n", min, max);
    }
}

/*暂停，等待按回车*/
void pause_and_continue(void)
{
    printf("\n按回车键继续...");
    getchar();
}

/*从系统配置加载专业列表，展示并让用户单选，返回选中编号(0=手输)*/
int pick_major(const char* label, char* majors_out)
{
    struct SystemConfig cfg;
    char list[MAX_MAJORS_LEN+1];
    char* token;
    char* items[50];
    int count, i, choice;

    ds_config_load(&cfg);
    strcpy(list, cfg.majors);
    count=0;
    token=strtok(list, ",");
    while(token!=NULL && count<50) {
        while(*token==' ') token++;
        items[count]=token;
        count++;
        token=strtok(NULL, ",");
    }

    if(count==0) {
        printf("\n  [提示] 专业列表为空，请先在系统配置中设置专业列表\n");
        majors_out[0]='\0';
        return 0;
    }
    printf("\n  可选%s:\n", label);
    for(i=0; i<count; i++) printf("    %d. %s\n", i+1, items[i]);
    printf("    0. 手动输入\n");
    choice=get_choice("  请选择: ", 0, count);
    if(choice==0) {
        printf("  请输入%s: ", label);
        fgets(majors_out, MAX_MAJORS_LEN, stdin);
        majors_out[strcspn(majors_out, "\n")]='\0';
        str_trim(majors_out);
    } else {
        strcpy(majors_out, items[choice-1]);
    }
    return choice;
}

/*专业多选器（用于课程适用专业），输出逗号分隔列表*/
int pick_majors_multi(const char* label, char* out)
{
    struct SystemConfig cfg;
    char list[MAX_MAJORS_LEN+1];
    char* items[50];
    int count, i, selected;
    char input[256];
    int picks[50];

    ds_config_load(&cfg);
    strcpy(list, cfg.majors);
    count=0;
    {
        char* token;
        token=strtok(list, ",");
        while(token!=NULL && count<50) {
            while(*token==' ') token++;
            items[count]=token;
            count++;
            token=strtok(NULL, ",");
        }
    }

    if(count==0) {
        printf("\n  [提示] 专业列表为空\n");
        out[0]='\0';
        return 0;
    }
    printf("\n  可选%s（多选，编号逗号分隔，0=全选）:\n", label);
    for(i=0; i<count; i++) printf("    %d. %s\n", i+1, items[i]);
    printf("  请选择: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")]='\0';
    str_trim(input);

    if(strlen(input)==0 || strcmp(input,"0")==0) {
        strcpy(out, cfg.majors);
        return count;
    }

    memset(picks, 0, sizeof(picks));
    {
        char* token;
        token=strtok(input, ", ， \t");
        while(token!=NULL) {
            int idx=atoi(token);
            if(idx>=1 && idx<=count) picks[idx-1]=1;
            token=strtok(NULL, ", ， \t");
        }
    }

    out[0]='\0';
    selected=0;
    for(i=0; i<count; i++) {
        if(picks[i]) {
            if(selected>0) strcat(out, ",");
            strcat(out, items[i]);
            selected++;
        }
    }
    if(selected==0) { strcpy(out, cfg.majors); return count; }
    return selected;
}
