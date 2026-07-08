/* menu_student.c - 学生控制台菜单，含个人信息、选课中心、成绩查询、AI助手 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <io.h>
#include "menu.h"
#include "utils.h"
#include "data_store.h"
#include "ai_helper.h"

/* 个人信息管理 */
static void sub_my_info(struct Session* s)
{
    int ch;
    while(1) {
        print_header("个人信息管理");
        printf("  当前用户: %s (学生)\n\n", s->username);
        printf("  1. 查看个人基本信息\n  2. 修改联系电话\n  3. 修改登录密码\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        if(ch==1) {
            struct Student st;
            if(ds_student_find_by_id(s->userid, &st)) {
                printf("\n  === 个人基本信息 ===\n\n");
                printf("  学号:%s  姓名:%s  性别:%s\n  年级:%s  专业:%s  电话:%s\n\n",
                    st.id, st.name, st.gender, st.grade, st.major, st.phone);
            } else printf("\n  [提示] 无法加载个人信息\n");
            pause_and_continue();
        } else if(ch==2) {
            struct Student st; char buf[32];
            if(!ds_student_find_by_id(s->userid, &st)) { printf("\n  [提示] 无法加载\n"); pause_and_continue(); }
            else {
                printf("\n  当前电话: %s\n", st.phone);
                while(1) { printf("  新电话(11位手机号): "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(!check_phone(buf)){printf("  [提示] 格式不正确\n");continue;}strcpy(st.phone,buf);break; }
                if(ds_student_update(s->userid,&st)) printf("\n  [成功] 电话已更新\n"); else printf("\n  [失败]\n");
                pause_and_continue();
            }
        } else if(ch==3) {
            struct Student st; char old_pwd[32], new_pwd[32], confirm[32], hash[33];
            if(!ds_student_find_by_id(s->userid, &st)) { printf("\n  [提示] 无法加载\n"); pause_and_continue(); }
            else {
                printf("\n  --- 修改密码 ---\n  请输入旧密码: ");
                if(_isatty(_fileno(stdin))){int idx=0;char ch;while(1){ch=getch();if(ch=='\r'||ch=='\n'){old_pwd[idx]='\0';printf("\n");break;}if(ch=='\b'||ch==127){if(idx>0){idx--;printf("\b \b");}}else if(idx<31){old_pwd[idx]=ch;idx++;printf("*");}}}
                else{fgets(old_pwd,sizeof(old_pwd),stdin);old_pwd[strcspn(old_pwd,"\n")]='\0';}
                str_trim(old_pwd);
                caesar_encrypt(old_pwd,3,hash);
                if(strcmp(hash,st.password)!=0){printf("\n  [错误] 旧密码不正确\n");pause_and_continue();}
                else{
                    printf("  请输入新密码: ");
                    if(_isatty(_fileno(stdin))){int idx=0;char ch;while(1){ch=getch();if(ch=='\r'||ch=='\n'){new_pwd[idx]='\0';printf("\n");break;}if(ch=='\b'||ch==127){if(idx>0){idx--;printf("\b \b");}}else if(idx<31){new_pwd[idx]=ch;idx++;printf("*");}}}
                    else{fgets(new_pwd,sizeof(new_pwd),stdin);new_pwd[strcspn(new_pwd,"\n")]='\0';}
                    str_trim(new_pwd);
                    printf("  请再次输入新密码: ");
                    if(_isatty(_fileno(stdin))){int idx=0;char ch;while(1){ch=getch();if(ch=='\r'||ch=='\n'){confirm[idx]='\0';printf("\n");break;}if(ch=='\b'||ch==127){if(idx>0){idx--;printf("\b \b");}}else if(idx<31){confirm[idx]=ch;idx++;printf("*");}}}
                    else{fgets(confirm,sizeof(confirm),stdin);confirm[strcspn(confirm,"\n")]='\0';}
                    str_trim(confirm);
                    if(strcmp(new_pwd,confirm)!=0){printf("\n  [错误] 两次密码不一致\n");pause_and_continue();}
                    else if(strlen(new_pwd)==0){printf("\n  [错误] 密码不能为空\n");pause_and_continue();}
                    else{caesar_encrypt(new_pwd,3,hash);strcpy(st.password,hash);if(ds_student_update(s->userid,&st))printf("\n  [成功] 密码已修改\n");else printf("\n  [失败]\n");pause_and_continue();}
                }
            }
        }
    }
}

/* 选课中心 */
static void sub_course_select(struct Session* s)
{
    int ch;
    while(1) {
        print_header("选课中心");
        printf("  当前用户: %s (学生)\n\n", s->username);
        printf("  1. 浏览可选课程列表\n  2. 查看课程详情\n  3. 执行选课操作\n  4. 查看我的课表\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) {
            struct Course courses[MAX_COURSES]; int count, i, sort_by; const char *type_str; struct Student st;
            ds_course_load_all(courses, &count);
            ds_student_find_by_id(s->userid, &st);
            {
                struct Course* avail; int n;
                n=0; for(i=0;i<count;i++){if(courses[i].status==COURSE_STATUS_SELECT&&strstr(courses[i].majors,st.major)!=NULL){if(n!=i)courses[n]=courses[i];n++;}}
                avail=courses;
                if(n==0){printf("\n  当前没有可选课程。\n");pause_and_continue();}
                else{
                    sort_by=get_choice("\n  排序方式(1按学分 2按已选人数 3不排序): ",1,3);
                    if(sort_by==1){int j,k;for(i=0;i<n-1;i++){k=i;for(j=i+1;j<n;j++)if(avail[j].credit>avail[k].credit)k=j;if(k!=i){struct Course tmp=avail[i];avail[i]=avail[k];avail[k]=tmp;}}}
                    else if(sort_by==2){int j,k;for(i=0;i<n-1;i++){k=i;for(j=i+1;j<n;j++)if(avail[j].enrolled>avail[k].enrolled)k=j;if(k!=i){struct Course tmp=avail[i];avail[i]=avail[k];avail[k]=tmp;}}}
                    printf("\n  === 可选课程（共 %d 门） ===\n\n",n);
                    printf("  %-6s %-16s %-4s %-4s %-6s %-5s %s\n","课程号","名称","类型","学分","教师","已选/上限","上课时段");
                    printf("  ------------------------------------------------------------------------\n");
                    for(i=0;i<n;i++){type_str=(avail[i].type==COURSE_TYPE_REQUIRED)?"必修":"选修";printf("  %-6s %-16s %-4s %-4.1f %-6s %2d/%-3d %s\n",avail[i].id,avail[i].name,type_str,avail[i].credit,avail[i].teacher_id,avail[i].enrolled,avail[i].max_students,avail[i].schedule);}
                    printf("\n"); pause_and_continue();
                }
            }
        } else if(ch==2) {
            struct Course c; char id[16]; const char *type_str, *status_str;
            printf("\n  请输入课程号: "); fgets(id,sizeof(id),stdin);id[strcspn(id,"\n")]='\0';str_trim(id);
            if(!ds_course_find_by_id(id,&c)){printf("\n  [提示] 未找到课程\n");}
            else{
                type_str=(c.type==COURSE_TYPE_REQUIRED)?"必修":"选修";
                if(c.status==COURSE_STATUS_DRAFT)status_str="未发布";else if(c.status==COURSE_STATUS_SELECT)status_str="选课中";else if(c.status==COURSE_STATUS_CLOSED)status_str="已结课";else status_str="?";
                printf("\n  === 课程详情 ===\n\n");
                printf("  课程号:%s  名称:%s  类型:%s  学分:%.1f\n  教师:%s  适用专业:%s\n  上课时段:%s  人数:%d/%d\n  平时占比:%.1f  期末占比:%.1f\n  教学周:第%d~%d周  日期:%s~%s\n  状态:%s  大纲:%s\n",c.id,c.name,type_str,c.credit,c.teacher_id,c.majors,c.schedule,c.enrolled,c.max_students,c.daily_ratio,c.final_ratio,c.start_week,c.end_week,c.start_date,c.end_date,status_str,c.syllabus);
            }
            printf("\n"); pause_and_continue();
        } else if(ch==3) {
            static struct Course avail[MAX_COURSES]; int total,n,i; char input[256]; int success_cnt,fail_cnt; struct Student st;
            ds_course_load_all(avail,&total); ds_student_find_by_id(s->userid,&st);
            n=0; for(i=0;i<total;i++){if(avail[i].status==COURSE_STATUS_SELECT&&strstr(avail[i].majors,st.major)!=NULL){if(n!=i)avail[n]=avail[i];n++;}}
            if(n==0){printf("\n  当前没有可选课程。\n");pause_and_continue();}
            else{
                const char* type_str;
                printf("\n  === 可选课程（输入编号批量选课，逗号或空格分隔） ===\n\n");
                printf("  %-4s %-6s %-16s %-4s %-4s %-5s %s\n","编号","课程号","名称","类型","学分","已选/上限","上课时段");
                printf("  ----------------------------------------------------------------------\n");
                for(i=0;i<n;i++){type_str=(avail[i].type==COURSE_TYPE_REQUIRED)?"必修":"选修";printf("  %-4d %-6s %-16s %-4s %-4.1f %2d/%-3d %s\n",i+1,avail[i].id,avail[i].name,type_str,avail[i].credit,avail[i].enrolled,avail[i].max_students,avail[i].schedule);}
                printf("\n  请输入要选的课程编号(0取消): "); fgets(input,sizeof(input),stdin);input[strcspn(input,"\n")]='\0';str_trim(input);
                if(strlen(input)>0&&strcmp(input,"0")!=0){
                    success_cnt=0;fail_cnt=0;printf("\n  --- 选课结果 ---\n");
                    {char* token;token=strtok(input,", ， \t");while(token!=NULL){int idx=atoi(token);if(idx>=1&&idx<=n){
                        struct Course* c=&avail[idx-1];int conflict=0,ok=1;
                        {struct Selection tmp;if(ds_selection_find_by_sc(s->userid,c->id,&tmp)){printf("  [跳过] %s - 已选过\n",c->name);fail_cnt++;ok=0;}}
                        if(ok&&c->enrolled>=c->max_students){printf("  [跳过] %s - 已满\n",c->name);fail_cnt++;ok=0;}
                        if(ok){
                            struct Selection my_sel[MAX_SELECTIONS];int sel_count,si;ds_selection_load_by_student(s->userid,my_sel,&sel_count);
                            for(si=0;si<sel_count&&!conflict;si++){struct Course other;if(ds_course_find_by_id(my_sel[si].course_id,&other)){if(check_time_conflict(c->schedule,c->start_week,c->end_week,other.schedule,other.start_week,other.end_week)){conflict=1;printf("  [跳过] %s - 与%s冲突\n",c->name,other.name);fail_cnt++;ok=0;}}}
                        }
                        if(ok){
                            struct Selection sel;char now[64];time_t t;struct tm* tm_info;int max_id;FILE* fp;
                            max_id=0;fp=fopen(FILE_SELECTIONS,"r");if(fp!=NULL){char lbuf[256];while(fgets(lbuf,sizeof(lbuf),fp)!=NULL){int rid_tmp;if(sscanf(lbuf,"%d",&rid_tmp)==1&&rid_tmp>max_id)max_id=rid_tmp;}fclose(fp);}
                            time(&t);tm_info=localtime(&t);sprintf(now,"%04d-%02d-%02d %02d:%02d:%02d",tm_info->tm_year+1900,tm_info->tm_mon+1,tm_info->tm_mday,tm_info->tm_hour,tm_info->tm_min,tm_info->tm_sec);
                            sel.record_id=max_id+1;strcpy(sel.student_id,s->userid);strcpy(sel.course_id,c->id);strcpy(sel.select_time,now);sel.status=0;
                            if(ds_selection_add(&sel)){c->enrolled++;ds_course_update(c->id,c);printf("  [成功] %s - %s\n",c->id,c->name);success_cnt++;}
                            else{printf("  [失败] %s\n",c->name);fail_cnt++;}
                        }
                    }
                    token=strtok(NULL,", ， \t");}}
                    printf("\n  选课完成: 成功 %d 门, 跳过 %d 门\n",success_cnt,fail_cnt);
                }
                pause_and_continue();
            }
        } else if(ch==4) {
            struct Selection my_sel[MAX_SELECTIONS]; int sel_count, i;
            ds_selection_load_by_student(s->userid, my_sel, &sel_count);
            if(sel_count==0){printf("\n  你还没有选任何课程。\n");pause_and_continue();}
            else{
                printf("\n  === 我的课表（共 %d 门） ===\n\n",sel_count);
                printf("  %-6s %-16s %-4s %s %-12s %-8s %s\n","课程号","名称","学分","上课时段","教师","状态","周次");
                printf("  ------------------------------------------------------------------\n");
                for(i=0;i<sel_count;i++){struct Course c;if(ds_course_find_by_id(my_sel[i].course_id,&c)){char week_info[20];const char* st_str;if(c.status==COURSE_STATUS_CLOSED)st_str="[已结课]";else if(c.status==COURSE_STATUS_SELECT)st_str="选课中";else st_str="?";sprintf(week_info,"第%d~%d周",c.start_week,c.end_week);printf("  %-6s %-16s %-4.1f %s %-12s %-8s %s\n",c.id,c.name,c.credit,c.schedule,c.teacher_id,st_str,week_info);}}
                printf("\n"); pause_and_continue();
            }
        }
    }
}

/* 成绩查询与统计 */
static void sub_grade_query(struct Session* s)
{
    int ch;
    while(1) {
        print_header("成绩查询与统计");
        printf("  当前用户: %s (学生)\n\n", s->username);
        printf("  1. 查看已出成绩列表\n  2. 查看成绩明细\n  3. 成绩统计分析\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        if(ch==1) {
            struct Score scores[MAX_SCORES]; int count, i;
            ds_score_load_by_student(s->userid,scores,&count);
            if(count==0){printf("\n  暂无已出成绩。\n");}
            else{
                printf("\n  === 已出成绩（共 %d 门） ===\n\n",count);
                printf("  %-8s %-16s %-6s %-6s %-6s %s\n","课程号","名称","平时","期末","总评","绩点");
                printf("  -----------------------------------------------------\n");
                for(i=0;i<count;i++){struct Course c;char cname[32];strcpy(cname,scores[i].course_id);if(ds_course_find_by_id(scores[i].course_id,&c))strcpy(cname,c.name);printf("  %-8s %-16s %-6.1f %-6.1f %-6.1f %.2f\n",scores[i].course_id,cname,scores[i].daily_score,scores[i].final_score,scores[i].total_score,scores[i].gpa);}
            }
            printf("\n"); pause_and_continue();
        } else if(ch==2) {
            struct Score sc; struct Course c; char course_id[16];
            printf("\n  请输入课程号: "); fgets(course_id,sizeof(course_id),stdin);course_id[strcspn(course_id,"\n")]='\0';str_trim(course_id);
            if(!ds_score_find_by_sc(s->userid,course_id,&sc)){printf("\n  [提示] 未找到成绩\n");}
            else{ds_course_find_by_id(course_id,&c);printf("\n  === 成绩明细 ===\n\n  课程:%s(%s)  学分:%.1f\n  平时:%.1f(占比%.1f)  期末:%.1f(占比%.1f)\n  总评:%.1f  绩点:%.2f  录入:%s\n",sc.course_id,c.name,c.credit,sc.daily_score,c.daily_ratio,sc.final_score,c.final_ratio,sc.total_score,sc.gpa,sc.record_time);}
            printf("\n"); pause_and_continue();
        } else if(ch==3) {
            struct Score scores[MAX_SCORES]; int count, i; float score_arr[MAX_SCORES],gpa_arr[MAX_SCORES],credit_arr[MAX_SCORES],avg_score,avg_gpa;
            ds_score_load_by_student(s->userid,scores,&count);
            if(count==0){printf("\n  暂无成绩数据。\n");}
            else{
                for(i=0;i<count;i++){struct Course c;score_arr[i]=scores[i].total_score;gpa_arr[i]=scores[i].gpa;if(ds_course_find_by_id(scores[i].course_id,&c))credit_arr[i]=c.credit;else credit_arr[i]=0.0f;}
                avg_score=calc_weighted_avg(score_arr,credit_arr,count);avg_gpa=calc_weighted_gpa(gpa_arr,credit_arr,count);
                printf("\n  ====== 成绩统计分析 ======\n\n  已出成绩:%d门  加权平均分:%.1f  加权平均绩点:%.2f\n\n  %-8s %-14s %-4s %-6s %-6s\n  ------------------------------------\n",count,avg_score,avg_gpa,"课程号","名称","学分","总评","绩点");
                for(i=0;i<count;i++){struct Course c;char cname[32];strcpy(cname,scores[i].course_id);if(ds_course_find_by_id(scores[i].course_id,&c))strcpy(cname,c.name);printf("  %-8s %-14s %-4.1f %-6.1f %-6.2f\n",scores[i].course_id,cname,credit_arr[i],scores[i].total_score,scores[i].gpa);}
            }
            printf("\n"); pause_and_continue();
        }
    }
}

/* AI智能学习助手 */
static void sub_ai_analysis(struct Session* s)
{
    char reply[4096];
    print_header("AI智能学习助手");
    printf("  当前用户: %s (学生)\n\n", s->username);
    printf("  正在调用 AI 分析您的成绩数据，请稍候...\n\n");
    if(ai_grade_analysis(s->userid, reply, sizeof(reply))) printf("  %s\n", reply);
    else { struct Score scores[MAX_SCORES]; int count; ds_score_load_by_student(s->userid,scores,&count); if(count==0) printf("  暂无成绩记录，无法分析。\n"); else printf("  [提示] AI 调用失败，请检查网络。\n"); }
    printf("\n"); pause_and_continue();
}

/* 智能选课助手 */
static void sub_ai_select(struct Session* s)
{
    char interests[256], future_plan[256], reply[4096];
    print_header("智能选课助手");
    printf("  当前用户: %s (学生)\n\n", s->username);
    printf("  根据您的专业、兴趣和未来规划智能推荐课程。\n\n");
    printf("  请输入您的兴趣爱好(可选):\n  > "); fgets(interests,sizeof(interests),stdin);interests[strcspn(interests,"\n")]='\0';str_trim(interests);
    printf("\n  请输入您的未来规划(可选):\n  > "); fgets(future_plan,sizeof(future_plan),stdin);future_plan[strcspn(future_plan,"\n")]='\0';str_trim(future_plan);
    printf("\n  正在调用 AI 分析推荐，请稍候...\n\n");
    if(ai_course_recommend(s->userid, interests, future_plan, reply, sizeof(reply))) printf("  %s\n", reply);
    else printf("  [提示] AI 调用失败，请检查网络。\n");
    printf("\n"); pause_and_continue();
}

/* 学生主菜单 */
void student_menu(struct Session* session)
{
    int ch;
    while(1) {
        print_header("学生中心");
        printf("  当前用户: %s (学生)\n\n", session->username);
        printf("  === 功能菜单 ===\n\n");
        printf("  1. 个人信息管理\n  2. 选课中心\n  3. 成绩查询与统计\n  4. AI智能学习助手\n  5. 智能选课助手\n  0. 退出登录\n\n");
        ch=get_choice("请输入选项 [0-5]: ", 0, 5);
        if(ch==0) break;
        if(ch==1) sub_my_info(session); else if(ch==2) sub_course_select(session); else if(ch==3) sub_grade_query(session); else if(ch==4) sub_ai_analysis(session); else if(ch==5) sub_ai_select(session);
    }
}
