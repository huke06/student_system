/*教师控制台菜单，含课程管理、选课名单、成绩录入*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "menu.h"
#include "data_store.h"
#include "utils.h"

/*课程开设与管理*/
static void sub_course_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("课程开设与管理");
        printf("  当前用户: %s (教师)\n\n", s->username);
        printf("  1. 新增课程\n  2. 修改我的课程\n  3. 查看我的课程\n  4. 发布课程\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) {
            struct Course c; char buf[128];
            printf("\n  --- 新增课程 ---\n"); memset(&c, 0, sizeof(c));
            while(1) { printf("  课程号: "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)==0){printf("  [提示] 不能为空\n");continue;}if(ds_course_find_by_id(buf,&c)){printf("  [提示] 已存在\n");continue;}strcpy(c.id,buf);break; }
            printf("  课程名称: "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);strcpy(c.name,buf);
            c.type=get_choice("  课程类型(0必修 1选修): ", 0, 1);
            printf("  学分: "); fgets(buf,sizeof(buf),stdin);c.credit=(float)atof(buf);
            strcpy(c.teacher_id, s->userid);
            pick_majors_multi("适用专业", c.majors); printf("  适用专业: %s\n", c.majors);
            printf("  上课时段(如: 周一 第1-2节): "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);strcpy(c.schedule,buf);
            c.max_students=get_choice("  人数上限: ", 1, 500);
            printf("  平时成绩占比(0~1,如0.4): "); fgets(buf,sizeof(buf),stdin);c.daily_ratio=(float)atof(buf);
            c.final_ratio=1.0f-c.daily_ratio;
            printf("  期末成绩占比(自动%.1f，回车确认或手动修改): ", c.final_ratio); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)c.final_ratio=(float)atof(buf);
            printf("  教学大纲: "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);strcpy(c.syllabus,buf);
            c.start_week=get_choice("  起始教学周: ", 1, 20);
            c.end_week=get_choice("  结束教学周: ", c.start_week, 20);
            {   struct SystemConfig cfg;
                if(ds_config_load(&cfg)) { calc_week_start_date(cfg.semester_start, c.start_week, c.start_date); calc_week_start_date(cfg.semester_start, c.end_week, c.end_date); printf("  开始日期(自动推算): %s\n", c.start_date); printf("  结束日期(自动推算): %s (回车确认或手动修改)\n", c.end_date); }
                printf("  修改开始日期(回车保留): "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c.start_date,buf);
                printf("  修改结束日期(回车保留): "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c.end_date,buf);
            }
            c.status=COURSE_STATUS_DRAFT; c.enrolled=0;
            if(ds_course_add(&c)) printf("\n  [成功] 课程 %s(%s) 已添加\n", c.id, c.name); else printf("\n  [失败]\n");
            pause_and_continue();
        } else if(ch==2) {
            struct Course my_courses[MAX_COURSES]; int total, i, my_count;
            ds_course_load_all(my_courses, &total);
            printf("\n  我的课程:\n"); my_count=0;
            for(i=0;i<total;i++){if(strcmp(my_courses[i].teacher_id,s->userid)==0){my_count++;printf("    %d. %s - %s\n",my_count,my_courses[i].id,my_courses[i].name);}}
            if(my_count==0){printf("\n  暂无课程。\n");pause_and_continue();}
            else{
                int cno=get_choice("  请选择课程编号(0取消): ",0,my_count);
                if(cno>0){
                    struct Course* c=NULL;int n=0;
                    for(i=0;i<total;i++){if(strcmp(my_courses[i].teacher_id,s->userid)==0){n++;if(n==cno){c=&my_courses[i];break;}}}
                    if(c!=NULL){
                        char buf[128];
                        printf("\n  当前信息（回车保留原值）:\n");
                        printf("  课程名称 [%s]: ",c->name);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c->name,buf);
                        printf("  上课时段 [%s]: ",c->schedule);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c->schedule,buf);
                        printf("  人数上限 [%d]: ",c->max_students);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)c->max_students=atoi(buf);
                        {char new_majors[MAX_MAJORS_LEN+1];printf("  适用专业 [%s] (输入0打开选择器): ",c->majors);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0){if(strcmp(buf,"0")==0){pick_majors_multi("适用专业",new_majors);strcpy(c->majors,new_majors);}else strcpy(c->majors,buf);}}
                        printf("  教学大纲 [%s]: ",c->syllabus);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c->syllabus,buf);
                        if(ds_course_update(c->id,c))printf("\n  [成功] 课程已更新\n");else printf("\n  [失败]\n");
                        pause_and_continue();
                    }
                }
            }
        } else if(ch==3) {
            struct Course courses[MAX_COURSES]; int count, i; const char *type_str, *status_str;
            ds_course_load_all(courses, &count);
            printf("\n  === 我的课程 ===\n\n");
            {int shown=0;
            for(i=0;i<count;i++){
                struct Course* c=&courses[i];if(strcmp(c->teacher_id,s->userid)!=0)continue;
                if(!shown){printf("  %-6s %-16s %-4s %-4s %-5s %-10s %s\n","课程号","名称","类型","学分","上限/已选","上课时段","状态");printf("  --------------------------------------------------------------------------------\n");shown=1;}
                type_str=(c->type==COURSE_TYPE_REQUIRED)?"必修":"选修";
                if(c->status==COURSE_STATUS_DRAFT)status_str="未发布";else if(c->status==COURSE_STATUS_SELECT)status_str="选课中";else if(c->status==COURSE_STATUS_CLOSED)status_str="已结课";else status_str="?";
                printf("  %-6s %-16s %-4s %-4.1f %2d/%-3d %-10s %s\n",c->id,c->name,type_str,c->credit,c->enrolled,c->max_students,c->schedule,status_str);
            }
            if(!shown)printf("  暂无课程记录。\n");}
            printf("\n"); pause_and_continue();
        } else if(ch==4) {
            struct Course my_courses[MAX_COURSES]; int total, i, my_count;
            ds_course_load_all(my_courses, &total);
            printf("\n  我的未发布课程:\n"); my_count=0;
            for(i=0;i<total;i++){if(strcmp(my_courses[i].teacher_id,s->userid)==0&&my_courses[i].status==COURSE_STATUS_DRAFT){my_count++;printf("    %d. %s - %s\n",my_count,my_courses[i].id,my_courses[i].name);}}
            if(my_count==0){printf("\n  没有可发布的课程。\n");pause_and_continue();}
            else{
                int cno=get_choice("  请选择课程编号(0取消): ",0,my_count);
                if(cno>0){
                    struct Course* c=NULL;int n=0;
                    for(i=0;i<total;i++){if(strcmp(my_courses[i].teacher_id,s->userid)==0&&my_courses[i].status==COURSE_STATUS_DRAFT){n++;if(n==cno){c=&my_courses[i];break;}}}
                    if(c!=NULL){
                        c->status=COURSE_STATUS_SELECT;
                        if(ds_course_update(c->id,c))printf("\n  [成功] %s(%s) 已发布\n",c->id,c->name);else printf("\n  [失败]\n");
                        pause_and_continue();
                    }
                }
            }
        }
    }
}

/*选课名单管理*/
static void sub_roster(struct Session* s)
{
    int ch;
    while(1) {
        print_header("选课名单管理");
        printf("  当前用户: %s (教师)\n\n", s->username);
        printf("  1. 查看课程选课名单\n  2. 搜索选课学生\n  3. 导出选课名单\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        if(ch==1) {
            struct Course courses[MAX_COURSES]; int count, i; char course_id[16];
            ds_course_load_all(courses, &count);
            printf("\n  我的课程:\n");
            for(i=0;i<count;i++){if(strcmp(courses[i].teacher_id,s->userid)==0)printf("    %s - %s (已选 %d/%d)\n",courses[i].id,courses[i].name,courses[i].enrolled,courses[i].max_students);}
            printf("\n  请输入课程号: "); fgets(course_id,sizeof(course_id),stdin);course_id[strcspn(course_id,"\n")]='\0';str_trim(course_id);
            {
                struct Course c;
                if(!ds_course_find_by_id(course_id,&c)){printf("\n  [提示] 课程不存在\n");}
                else if(strcmp(c.teacher_id,s->userid)!=0){printf("\n  [提示] 这不是你的课程\n");}
                else{
                    struct Selection sel_all[MAX_SELECTIONS];int sel_count,si,shown;
                    ds_selection_load_all(sel_all,&sel_count);
                    printf("\n  === %s 选课名单（%d/%d） ===\n\n",c.name,c.enrolled,c.max_students);shown=0;
                    for(si=0;si<sel_count;si++){
                        if(strcmp(sel_all[si].course_id,course_id)==0){
                            struct Student st;
                            if(!shown){printf("  %-12s %-8s %-4s %-16s %s\n","学号","姓名","性别","专业","选课时间");printf("  -------------------------------------------------------------\n");shown=1;}
                            if(ds_student_find_by_id(sel_all[si].student_id,&st))
                                printf("  %-12s %-8s %-4s %-16s %s\n",st.id,st.name,st.gender,st.major,sel_all[si].select_time);
                            else printf("  %-12s %s\n",sel_all[si].student_id,sel_all[si].select_time);
                        }
                    }
                    if(!shown)printf("  暂无学生选课。\n");
                }
            }
            printf("\n"); pause_and_continue();
        } else if(ch==2) {
            char keyword[32]; struct Selection sel_all[MAX_SELECTIONS]; int sel_count, i;
            printf("\n  输入学号或姓名搜索: "); fgets(keyword,sizeof(keyword),stdin);keyword[strcspn(keyword,"\n")]='\0';str_trim(keyword);
            ds_selection_load_all(sel_all, &sel_count);
            printf("\n  === 搜索结果 ===\n\n");
            for(i=0;i<sel_count;i++){
                struct Student st;struct Course c;
                if(ds_student_find_by_id(sel_all[i].student_id,&st)){
                    if(strstr(st.id,keyword)!=NULL||strstr(st.name,keyword)!=NULL){
                        ds_course_find_by_id(sel_all[i].course_id,&c);
                        if(strcmp(c.teacher_id,s->userid)==0)
                            printf("  %s(%s) -> %s(%s)  %s\n",st.id,st.name,c.id,c.name,sel_all[i].select_time);
                    }
                }
            }
            printf("\n"); pause_and_continue();
        } else if(ch==3) {
            char course_id[16];
            printf("\n  --- 导出选课名单 ---\n  请输入课程号: ");
            fgets(course_id,sizeof(course_id),stdin);course_id[strcspn(course_id,"\n")]='\0';str_trim(course_id);
            {
                struct Course c;
                if(!ds_course_find_by_id(course_id,&c)){printf("\n  [提示] 课程不存在\n");}
                else if(strcmp(c.teacher_id,s->userid)!=0){printf("\n  [提示] 这不是你的课程\n");}
                else{
                    struct Selection sel_all[MAX_SELECTIONS];int sel_count,si;char path[64];FILE* fp;
                    sprintf(path,"data/exports/%s_roster.txt",course_id);
#ifdef _WIN32
                    _mkdir("data/exports");
#else
                    mkdir("data/exports",0755);
#endif
                    fp=fopen(path,"w");
                    if(fp==NULL){printf("\n  [失败] 无法创建文件\n");}
                    else{
                        fprintf(fp,"课程选课名单\n==============================\n");
                        fprintf(fp,"课程号:%s\n课程名:%s\n教师:%s(%s)\n人数:%d/%d\n",c.id,c.name,c.teacher_id,s->username,c.enrolled,c.max_students);
                        fprintf(fp,"==============================\n\n%-12s %-8s %-4s %-16s %s\n","学号","姓名","性别","专业","选课时间");
                        fprintf(fp,"-------------------------------------------------------------\n");
                        ds_selection_load_all(sel_all,&sel_count);
                        for(si=0;si<sel_count;si++){
                            if(strcmp(sel_all[si].course_id,course_id)==0){
                                struct Student st;
                                if(ds_student_find_by_id(sel_all[si].student_id,&st))
                                    fprintf(fp,"%-12s %-8s %-4s %-16s %s\n",st.id,st.name,st.gender,st.major,sel_all[si].select_time);
                            }
                        }
                        fclose(fp);
                        printf("\n  [成功] 已导出到 %s\n",path);
                    }
                }
            }
            pause_and_continue();
        }
    }
}

/*成绩录入与管理*/
static void sub_grade_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("成绩录入与管理");
        printf("  当前用户: %s (教师)\n\n", s->username);
        printf("  1. 课程结课操作\n  2. 录入成绩\n  3. 修改成绩\n  4. 查看成绩与统计\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) {
            struct Course courses[MAX_COURSES]; int count, i; char course_id[16];
            ds_course_load_all(courses, &count);
            printf("\n  我的选课中课程:\n");
            for(i=0;i<count;i++){if(strcmp(courses[i].teacher_id,s->userid)==0&&courses[i].status==COURSE_STATUS_SELECT)printf("    %s - %s\n",courses[i].id,courses[i].name);}
            printf("\n  请输入要结课的课程号: "); fgets(course_id,sizeof(course_id),stdin);course_id[strcspn(course_id,"\n")]='\0';str_trim(course_id);
            {
                struct Course c;
                if(!ds_course_find_by_id(course_id,&c)){printf("\n  [提示] 课程不存在\n");}
                else if(strcmp(c.teacher_id,s->userid)!=0){printf("\n  [提示] 这不是你的课程\n");}
                else if(c.status!=COURSE_STATUS_SELECT){printf("\n  [提示] 只有选课中的课程才能结课\n");}
                else{c.status=COURSE_STATUS_CLOSED;if(ds_course_update(course_id,&c))printf("\n  [成功] %s 已结课\n",course_id);}
            }
            pause_and_continue();
        } else if(ch==2||ch==3||ch==4) {
            struct Course my_courses[MAX_COURSES]; int total, i, my_count; struct Course* picked;
            int is_enter=(ch==2), is_modify=(ch==3);
            ds_course_load_all(my_courses, &total);
            if(is_enter){printf("\n  我的已结课课程(仅结课后可录入):\n");}else{printf("\n  我的课程:\n");}my_count=0;
            for(i=0;i<total;i++){if(strcmp(my_courses[i].teacher_id,s->userid)==0&&(!is_enter||my_courses[i].status==COURSE_STATUS_CLOSED)){my_count++;printf("    %d. %s - %s (已选%d人)\n",my_count,my_courses[i].id,my_courses[i].name,my_courses[i].enrolled);}}
            if(my_count==0){printf("\n  暂无课程。\n");pause_and_continue();}
            else{
                int cno=get_choice("  请选择课程编号(0取消): ",0,my_count);
                if(cno==0){}
                else{
                    picked=NULL;my_count=0;
                    for(i=0;i<total;i++){if(strcmp(my_courses[i].teacher_id,s->userid)==0&&(!is_enter||my_courses[i].status==COURSE_STATUS_CLOSED)){my_count++;if(my_count==cno){picked=&my_courses[i];break;}}}
                    if(picked==NULL){printf("\n  [错误]\n");pause_and_continue();}
                    else if(ch==4){
                        struct Score all_scores[MAX_SCORES]; int sc_total, si; int score_count=0;
                        struct Score view[MAX_SCORES]; char sname_buf[MAX_SCORES][32];
                        int sort_by;
                        ds_score_load_all(all_scores,&sc_total);
                        /*收集本课程成绩*/
                        for(si=0;si<sc_total;si++){
                            if(strcmp(all_scores[si].course_id,picked->id)==0){
                                view[score_count]=all_scores[si];
                                {struct Student st;if(ds_student_find_by_id(all_scores[si].student_id,&st))strcpy(sname_buf[score_count],st.name);else strcpy(sname_buf[score_count],all_scores[si].student_id);}
                                score_count++;
                            }
                        }
                        if(score_count==0){printf("\n  === %s 成绩列表 ===\n\n  暂无成绩数据。\n",picked->name);}
                        else{
                            sort_by=get_choice("\n  排序方式(1按学号 2按总评升序 3按总评降序): ",1,3);
                            if(sort_by==2||sort_by==3){
                                int j,k;for(si=0;si<score_count-1;si++){k=si;for(j=si+1;j<score_count;j++){
                                    if((sort_by==2&&view[j].total_score<view[k].total_score)||(sort_by==3&&view[j].total_score>view[k].total_score))k=j;
                                }if(k!=si){struct Score tmp=view[si];view[si]=view[k];view[k]=tmp;{char tmpn[32];strcpy(tmpn,sname_buf[si]);strcpy(sname_buf[si],sname_buf[k]);strcpy(sname_buf[k],tmpn);}}}
                            }
                            printf("\n  === %s 成绩列表 ===\n\n",picked->name);
                            printf("  %-12s %-8s %-6s %-6s %-6s %s\n","学号","姓名","平时","期末","总评","绩点");
                            printf("  -----------------------------------------------------\n");
                            for(si=0;si<score_count;si++)printf("  %-12s %-8s %-6.1f %-6.1f %-6.1f %.2f\n",view[si].student_id,sname_buf[si],view[si].daily_score,view[si].final_score,view[si].total_score,view[si].gpa);
                            {float sum=0,max_s=0,min_s=100;int pass=0,exc=0,good=0,mid=0,fail=0;
                            for(si=0;si<score_count;si++){
                                float s=view[si].total_score;sum+=s;
                                if(s>max_s)max_s=s;if(s<min_s)min_s=s;
                                if(s>=90)exc++;else if(s>=80)good++;else if(s>=70)mid++;else if(s>=60)pass++;else fail++;
                            }
                            printf("\n  --- 统计 ---\n  人数:%d  最高:%.1f  最低:%.1f  平均:%.1f  及格率:%.1f%%\n",score_count,max_s,min_s,sum/score_count,100.0f*(exc+good+mid+pass)/score_count);}
                        }
                        printf("\n"); pause_and_continue();
                    } else {
                        struct Selection sel_all[MAX_SELECTIONS]; int sel_count, si;
                        struct { char sid[11]; char sname[32]; int has_grade; float total; } roster[200];
                        int ros_count;
                        ds_selection_load_all(sel_all,&sel_count);
                        ros_count=0;
                        for(si=0;si<sel_count;si++){
                            if(strcmp(sel_all[si].course_id,picked->id)==0){
                                struct Student st;struct Score exist;
                                strcpy(roster[ros_count].sid,sel_all[si].student_id);
                                if(ds_student_find_by_id(sel_all[si].student_id,&st))strcpy(roster[ros_count].sname,st.name);
                                else strcpy(roster[ros_count].sname,"?");
                                if(ds_score_find_by_sc(sel_all[si].student_id,picked->id,&exist)){roster[ros_count].has_grade=1;roster[ros_count].total=exist.total_score;}
                                else{roster[ros_count].has_grade=0;roster[ros_count].total=0;}
                                ros_count++;
                            }
                        }
                        if(ros_count==0){printf("\n  该课程暂无学生选课。\n");pause_and_continue();}
                        else{
                            while(1){
                                printf("\n  === %s 学生名单 ===\n\n",picked->name);
                                printf("  %-4s %-12s %-8s %s\n","编号","学号","姓名","成绩状态");
                                printf("  ------------------------------------\n");
                                for(si=0;si<ros_count;si++)printf("  %-4d %-12s %-8s %s\n",si+1,roster[si].sid,roster[si].sname,roster[si].has_grade?"已有成绩":"未录入");
                                {int sno;char buf[32];
                                if(is_enter)printf("\n  输入要录入成绩的学生编号(0返回): ");
                                else printf("\n  输入要修改成绩的学生编号(0返回): ");
                                sno=get_choice("",0,ros_count);
                                if(sno==0)break;
                                sno--;
                                if(is_enter&&roster[sno].has_grade){printf("\n  [提示] 该学生已有成绩(总评:%.1f)\n",roster[sno].total);}
                                else if(is_modify&&!roster[sno].has_grade){printf("\n  [提示] 该学生还未录入成绩\n");}
                                else{
                                    struct Score sc;char now[64];time_t t;struct tm* tm_info;int max_id;FILE* fp;
                                    if(is_modify){ds_score_find_by_sc(roster[sno].sid,picked->id,&sc);printf("\n  当前: 平时%.1f 期末%.1f 总评%.1f\n",sc.daily_score,sc.final_score,sc.total_score);}
                                    printf("  平时成绩(0-100): ");fgets(buf,sizeof(buf),stdin);sc.daily_score=(float)atof(buf);
                                    printf("  期末成绩(0-100): ");fgets(buf,sizeof(buf),stdin);sc.final_score=(float)atof(buf);
                                    sc.total_score=calc_total_score(sc.daily_score,sc.final_score,picked->daily_ratio,picked->final_ratio);
                                    sc.gpa=score_to_gpa(sc.total_score);
                                    if(is_enter){
                                        max_id=0;fp=fopen(FILE_SCORES,"r");if(fp!=NULL){char lbuf[256];while(fgets(lbuf,sizeof(lbuf),fp)!=NULL){int tmp_id;if(sscanf(lbuf,"%d",&tmp_id)==1&&tmp_id>max_id)max_id=tmp_id;}fclose(fp);}
                                        sc.record_id=max_id+1;strcpy(sc.student_id,roster[sno].sid);strcpy(sc.course_id,picked->id);
                                        time(&t);tm_info=localtime(&t);sprintf(now,"%04d-%02d-%02d %02d:%02d:%02d",tm_info->tm_year+1900,tm_info->tm_mon+1,tm_info->tm_mday,tm_info->tm_hour,tm_info->tm_min,tm_info->tm_sec);
                                        strcpy(sc.record_time,now);strcpy(sc.teacher_id,s->userid);
                                        if(ds_score_add(&sc)){printf("\n  [成功] %s 成绩已录入: 总评%.1f 绩点%.2f\n",roster[sno].sname,sc.total_score,sc.gpa);roster[sno].has_grade=1;roster[sno].total=sc.total_score;}
                                        else printf("\n  [失败]\n");
                                    } else {
                                        if(ds_score_update(sc.record_id,&sc)){printf("\n  [成功] %s 成绩已更新: 总评%.1f 绩点%.2f\n",roster[sno].sname,sc.total_score,sc.gpa);roster[sno].total=sc.total_score;}
                                        else printf("\n  [失败]\n");
                                    }
                                }}printf("\n");}
                            pause_and_continue();
                        }
                    }
                }
            }
        }
    }
}

/*教师主菜单*/
void teacher_menu(struct Session* session)
{
    int ch;
    while(1) {
        print_header("教师工作台");
        printf("  当前用户: %s (教师)\n\n", session->username);
        printf("  === 功能菜单 ===\n\n");
        printf("  1. 课程开设与管理\n  2. 选课名单管理\n  3. 成绩录入与管理\n  0. 退出登录\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        if(ch==1) sub_course_mgr(session); else if(ch==2) sub_roster(session); else if(ch==3) sub_grade_mgr(session);
    }
}
