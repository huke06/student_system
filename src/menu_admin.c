/*管理员控制台菜单，含账号管理、课程管理、系统配置、日志分析 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <io.h>
#include "menu.h"
#include "logger.h"
#include "data_store.h"
#include "utils.h"

/*学生账号管理*/
static void sub_student_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("学生账号管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 新增学生信息\n");
        printf("  2. 修改学生信息\n");
        printf("  3. 删除学生信息\n");
        printf("  4. 浏览与查询学生信息\n");
        printf("  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==4) {
            struct Student students[MAX_STUDENTS];
            int count, i;
            char keyword[32];

            printf("\n  输入学号或姓名关键词搜索(直接回车显示全部): ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")]='\0';
            str_trim(keyword);

            ds_student_load_all(students, &count);
            printf("\n  === 学生列表（共 %d 条） ===\n\n", count);
            printf("  %-12s %-8s %-4s %-6s %-20s %-13s\n",
                "学号","姓名","性别","年级","专业","电话");
            printf("  --------------------------------------------------------------------------------\n");

            for(i=0; i<count; i++) {
                struct Student* p=&students[i];
                if(strlen(keyword)>0) {
                    if(strstr(p->id, keyword)==NULL &&
                       strstr(p->name, keyword)==NULL) continue;
                }
                printf("  %-12s %-8s %-4s %-6s %-20s %-13s\n",
                    p->id, p->name, p->gender, p->grade, p->major, p->phone);
            }
            printf("\n");
            pause_and_continue();
        } else if(ch==1) {
            struct Student st;
            char buf[64], pwd_plain[7], hash[33];

            printf("\n  --- 新增学生 ---\n");
            memset(&st, 0, sizeof(st));

            while(1) {
                printf("  学号(10位数字): ");
                fgets(buf, sizeof(buf), stdin);
                buf[strcspn(buf, "\n")]='\0';
                str_trim(buf);
                if(!check_student_id(buf)) { printf("  [提示] 学号必须为10位数字\n"); continue; }
                if(ds_student_find_by_id(buf, &st)) { printf("  [提示] 该学号已存在\n"); continue; }
                strcpy(st.id, buf); break;
            }

            printf("  姓名: "); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf); strcpy(st.name, buf);
            printf("  性别(男/女): "); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf); strcpy(st.gender, buf);
            printf("  年级(如2026): "); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf); strcpy(st.grade, buf);
            pick_major("专业", st.major);

            while(1) {
                printf("  电话(11位手机号): ");
                fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf);
                if(!check_phone(buf)) { printf("  [提示] 手机号格式不正确\n"); continue; }
                strcpy(st.phone, buf); break;
            }

            gen_init_pwd(st.id, pwd_plain);
            caesar_encrypt(pwd_plain, 3, hash);
            strcpy(st.password, hash);

            if(ds_student_add(&st))
                printf("\n  [成功] 学生 %s(%s) 已添加，初始密码为 %s\n", st.id, st.name, pwd_plain);
            else
                printf("\n  [失败] 添加失败\n");
            pause_and_continue();
        } else if(ch==2) {
            struct Student st;
            char id[16], buf[64];

            printf("\n  --- 修改学生信息 ---\n");
            printf("  请输入要修改的学号: ");
            fgets(id, sizeof(id), stdin); id[strcspn(id, "\n")]='\0'; str_trim(id);

            if(!ds_student_find_by_id(id, &st)) {
                printf("\n  [提示] 未找到学号为 %s 的学生\n", id);
                pause_and_continue();
            } else {
                printf("\n  当前信息（直接回车保留原值）:\n");
                printf("  -------------------------------------------------\n");

                printf("  姓名 [%s]: ", st.name); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf); if(strlen(buf)>0) strcpy(st.name, buf);
                printf("  性别 [%s]: ", st.gender); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf); if(strlen(buf)>0) strcpy(st.gender, buf);
                printf("  年级 [%s]: ", st.grade); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf); if(strlen(buf)>0) strcpy(st.grade, buf);

                {   char new_major[MAX_MAJORS_LEN+1];
                    printf("  专业 [%s] (输入0打开选择器): ", st.major);
                    fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf);
                    if(strlen(buf)>0) {
                        if(strcmp(buf,"0")==0) { if(pick_major("专业", new_major)) strcpy(st.major, new_major); }
                        else strcpy(st.major, buf);
                    }
                }

                while(1) {
                    printf("  电话 [%s]: ", st.phone); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf);
                    if(strlen(buf)==0) break;
                    if(!check_phone(buf)) { printf("  [提示] 手机号格式不正确\n"); continue; }
                    strcpy(st.phone, buf); break;
                }

                if(ds_student_update(id, &st)) printf("\n  [成功] 学生 %s 信息已更新\n", id);
                else printf("\n  [失败] 更新失败\n");
                pause_and_continue();
            }
        } else if(ch==3) {
            struct Student st;
            char id[16], confirm[8];

            printf("\n  --- 删除学生信息 ---\n");
            printf("  请输入要删除的学号: ");
            fgets(id, sizeof(id), stdin); id[strcspn(id, "\n")]='\0'; str_trim(id);

            if(!ds_student_find_by_id(id, &st)) {
                printf("\n  [提示] 未找到学号为 %s 的学生\n", id);
            } else {
                printf("\n  确认删除：学号:%s  姓名:%s  专业:%s\n", st.id, st.name, st.major);
                printf("  输入 y 确认删除: ");
                fgets(confirm, sizeof(confirm), stdin); confirm[strcspn(confirm, "\n")]='\0';
                if(strcmp(confirm,"y")==0||strcmp(confirm,"Y")==0) {
                    if(ds_student_delete(id)) printf("\n  [成功] 学生 %s 已删除\n", id);
                    else printf("\n  [失败] 删除失败\n");
                } else printf("\n  已取消\n");
            }
            pause_and_continue();
        }
    }
}

/*教师账号管理*/
static void sub_teacher_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("教师账号管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 新增教师信息\n  2. 修改教师信息\n  3. 删除教师信息\n  4. 浏览与查询教师信息\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==4) {
            struct Teacher teachers[MAX_TEACHERS];
            int count, i;
            char keyword[32];
            printf("\n  输入工号或姓名关键词搜索(直接回车显示全部): ");
            fgets(keyword, sizeof(keyword), stdin); keyword[strcspn(keyword, "\n")]='\0'; str_trim(keyword);
            ds_teacher_load_all(teachers, &count);
            printf("\n  === 教师列表（共 %d 条） ===\n\n", count);
            printf("  %-8s %-8s %-4s %-20s %-13s\n", "工号","姓名","性别","院系","电话");
            printf("  ------------------------------------------------------------------------\n");
            for(i=0; i<count; i++) {
                struct Teacher* p=&teachers[i];
                if(strlen(keyword)>0 && strstr(p->id,keyword)==NULL && strstr(p->name,keyword)==NULL) continue;
                printf("  %-8s %-8s %-4s %-20s %-13s\n", p->id, p->name, p->gender, p->department, p->phone);
            }
            printf("\n"); pause_and_continue();
        } else if(ch==1) {
            struct Teacher t;
            char buf[64], pwd_plain[7], hash[33];
            printf("\n  --- 新增教师 ---\n"); memset(&t, 0, sizeof(t));
            while(1) { printf("  工号(6位数字): "); fgets(buf, sizeof(buf), stdin); buf[strcspn(buf, "\n")]='\0'; str_trim(buf); if(!check_teacher_id(buf)){printf("  [提示] 工号必须为6位数字\n");continue;} if(ds_teacher_find_by_id(buf,&t)){printf("  [提示] 该工号已存在\n");continue;} strcpy(t.id,buf);break; }
            printf("  姓名: "); fgets(buf,sizeof(buf),stdin); buf[strcspn(buf,"\n")]='\0'; str_trim(buf); strcpy(t.name,buf);
            printf("  性别(男/女): "); fgets(buf,sizeof(buf),stdin); buf[strcspn(buf,"\n")]='\0'; str_trim(buf); strcpy(t.gender,buf);
            pick_major("院系", t.department);
            while(1){printf("  电话(11位手机号): ");fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(!check_phone(buf)){printf("  [提示] 手机号格式不正确\n");continue;}strcpy(t.phone,buf);break;}
            gen_init_pwd(t.id, pwd_plain); caesar_encrypt(pwd_plain, 3, hash); strcpy(t.password, hash);
            if(ds_teacher_add(&t)) printf("\n  [成功] 教师 %s(%s) 已添加，初始密码为 %s\n", t.id, t.name, pwd_plain);
            else printf("\n  [失败] 添加失败\n");
            pause_and_continue();
        } else if(ch==2) {
            struct Teacher t; char id[16], buf[64];
            printf("\n  --- 修改教师信息 ---\n  请输入要修改的工号: ");
            fgets(id,sizeof(id),stdin); id[strcspn(id,"\n")]='\0'; str_trim(id);
            if(!ds_teacher_find_by_id(id,&t)){printf("\n  [提示] 未找到工号为 %s 的教师\n",id);pause_and_continue();}
            else{
                printf("\n  当前信息（直接回车保留原值）:\n  -------------------------------------------------\n");
                printf("  姓名 [%s]: ",t.name);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(t.name,buf);
                printf("  性别 [%s]: ",t.gender);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(t.gender,buf);
                {char new_dept[MAX_MAJORS_LEN+1];printf("  院系 [%s] (输入0打开选择器): ",t.department);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0){if(strcmp(buf,"0")==0){if(pick_major("院系",new_dept))strcpy(t.department,new_dept);}else strcpy(t.department,buf);}}
                while(1){printf("  电话 [%s]: ",t.phone);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)==0)break;if(!check_phone(buf)){printf("  [提示] 手机号格式不正确\n");continue;}strcpy(t.phone,buf);break;}
                if(ds_teacher_update(id,&t)) printf("\n  [成功] 教师 %s 信息已更新\n",id); else printf("\n  [失败] 更新失败\n");
                pause_and_continue();
            }
        } else if(ch==3) {
            struct Teacher t; char id[16], confirm[8];
            printf("\n  --- 删除教师信息 ---\n  请输入要删除的工号: ");
            fgets(id,sizeof(id),stdin);id[strcspn(id,"\n")]='\0';str_trim(id);
            if(!ds_teacher_find_by_id(id,&t)){printf("\n  [提示] 未找到工号为 %s 的教师\n",id);}
            else{
                printf("\n  确认删除：工号:%s  姓名:%s  院系:%s\n  输入 y 确认: ",t.id,t.name,t.department);
                fgets(confirm,sizeof(confirm),stdin);confirm[strcspn(confirm,"\n")]='\0';
                if(strcmp(confirm,"y")==0||strcmp(confirm,"Y")==0){if(ds_teacher_delete(id))printf("\n  [成功] 已删除\n");}
                else printf("\n  已取消\n");
            }
            pause_and_continue();
        }
    }
}

/*管理员账号管理*/
static void sub_admin_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("管理员账号管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 新增管理员\n  2. 修改管理员信息\n  3. 删除管理员\n  4. 重置管理员密码\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) {
            struct Admin a; char buf[64], hash[33];
            printf("\n  --- 新增管理员 ---\n"); memset(&a,0,sizeof(a));
            while(1){printf("  管理员账号: ");fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)==0){printf("  [提示] 账号不能为空\n");continue;}if(ds_admin_find_by_id(buf,&a)){printf("  [提示] 该账号已存在\n");continue;}strcpy(a.id,buf);break;}
            printf("  姓名: "); fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);strcpy(a.name,buf);
            printf("  密码: ");
            if(_isatty(_fileno(stdin))){int idx=0;char ch;while(1){ch=getch();if(ch=='\r'||ch=='\n'){buf[idx]='\0';printf("\n");break;}if(ch=='\b'||ch==127){if(idx>0){idx--;printf("\b \b");}}else if(idx<31){buf[idx]=ch;idx++;printf("*");}}}
            else{fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';}
            str_trim(buf);
            caesar_encrypt(buf,3,hash); strcpy(a.password,hash);
            if(ds_admin_add(&a)) printf("\n  [成功] 管理员 %s(%s) 已添加\n",a.id,a.name);
            else printf("\n  [失败] 添加失败\n");
            pause_and_continue();
        } else if(ch==2) {
            struct Admin a; char id[32], buf[64];
            printf("\n  --- 修改管理员信息 ---\n  请输入账号: ");
            fgets(id,sizeof(id),stdin);id[strcspn(id,"\n")]='\0';str_trim(id);
            if(!ds_admin_find_by_id(id,&a)){printf("\n  [提示] 未找到账号 %s\n",id);pause_and_continue();}
            else{printf("\n  姓名 [%s]: ",a.name);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(a.name,buf);
            if(ds_admin_update(id,&a))printf("\n  [成功] 已更新\n");else printf("\n  [失败] 更新失败\n");pause_and_continue();}
        } else if(ch==3) {
            struct Admin a; char id[32], confirm[8];
            printf("\n  --- 删除管理员 ---\n  请输入账号: ");
            fgets(id,sizeof(id),stdin);id[strcspn(id,"\n")]='\0';str_trim(id);
            if(!ds_admin_find_by_id(id,&a)){printf("\n  [提示] 未找到账号 %s\n",id);}
            else{printf("\n  确认删除: 账号:%s 姓名:%s\n  输入 y 确认: ",a.id,a.name);fgets(confirm,sizeof(confirm),stdin);confirm[strcspn(confirm,"\n")]='\0';
            if(strcmp(confirm,"y")==0||strcmp(confirm,"Y")==0){if(ds_admin_delete(id))printf("\n  [成功] 已删除\n");}else printf("\n  已取消\n");}
            pause_and_continue();
        } else if(ch==4) {
            struct Admin a; char id[32], new_pwd[32], hash[33];
            printf("\n  --- 重置管理员密码 ---\n  请输入账号: ");
            fgets(id,sizeof(id),stdin);id[strcspn(id,"\n")]='\0';str_trim(id);
            if(!ds_admin_find_by_id(id,&a)){printf("\n  [提示] 未找到账号 %s\n",id);pause_and_continue();}
            else{
                printf("  新密码(回车使用默认): ");
                if(_isatty(_fileno(stdin))){int idx=0;char ch;while(1){ch=getch();if(ch=='\r'||ch=='\n'){new_pwd[idx]='\0';printf("\n");break;}if(ch=='\b'||ch==127){if(idx>0){idx--;printf("\b \b");}}else if(idx<31){new_pwd[idx]=ch;idx++;printf("*");}}}
                else{fgets(new_pwd,sizeof(new_pwd),stdin);new_pwd[strcspn(new_pwd,"\n")]='\0';}
                str_trim(new_pwd); if(strlen(new_pwd)==0) strcpy(new_pwd,"123456");
                caesar_encrypt(new_pwd,3,hash); strcpy(a.password,hash);
                if(ds_admin_update(id,&a)) printf("\n  [成功] %s 的密码已重置\n",a.name);
                else printf("\n  [失败] 重置失败\n");
                pause_and_continue();
            }
        }
    }
}

/*课程全局管理*/
static void sub_course_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("课程全局管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 查看所有课程列表\n  2. 按条件筛选课程\n  3. 修改课程信息\n  4. 删除课程\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) {
            struct Course courses[MAX_COURSES]; int count, i; const char *type_str, *status_str;
            ds_course_load_all(courses, &count);
            printf("\n  === 课程列表（共 %d 门） ===\n\n", count);
            printf("  %-6s %-16s %-4s %-4s %-6s %-5s %-10s %s\n", "课程号","名称","类型","学分","教师","上限/已选","上课时段","状态");
            printf("  --------------------------------------------------------------------------------\n");
            for(i=0; i<count; i++) {
                struct Course* c=&courses[i];
                type_str=(c->type==COURSE_TYPE_REQUIRED)?"必修":"选修";
                if(c->status==COURSE_STATUS_DRAFT) status_str="未发布"; else if(c->status==COURSE_STATUS_SELECT) status_str="选课中"; else if(c->status==COURSE_STATUS_CLOSED) status_str="已结课"; else status_str="?";
                {struct Teacher t;char tn[22]="?";if(ds_teacher_find_by_id(c->teacher_id,&t))strcpy(tn,t.name);printf("  %-6s %-16s %-4s %-4.1f %-6s %2d/%-3d %-10s %s\n", c->id, c->name, type_str, c->credit, tn, c->enrolled, c->max_students, c->schedule, status_str);}
            }
            printf("\n"); pause_and_continue();
        } else if(ch==2) {
            int filter_type, filter_status; char filter_teacher[16];
            struct Course courses[MAX_COURSES]; int count, i, shown; const char *type_str, *status_str;
            ds_course_load_all(courses, &count);
            printf("\n  --- 筛选条件（0=不限） ---\n");
            filter_type=get_choice("  课程类型(0不限 1必修 2选修): ", 0, 2);
            filter_status=get_choice("  课程状态(0不限 1未发布 2选课中 3已结课): ", 0, 3);
            printf("  教师工号(0不限): "); fgets(filter_teacher,sizeof(filter_teacher),stdin); filter_teacher[strcspn(filter_teacher,"\n")]='\0'; str_trim(filter_teacher);
            printf("\n  === 筛选结果 ===\n\n"); shown=0;
            for(i=0; i<count; i++) {
                struct Course* c=&courses[i];
                if(filter_type==1 && c->type!=COURSE_TYPE_REQUIRED) continue;
                if(filter_type==2 && c->type!=COURSE_TYPE_ELECTIVE) continue;
                if(filter_status==1 && c->status!=COURSE_STATUS_DRAFT) continue;
                if(filter_status==2 && c->status!=COURSE_STATUS_SELECT) continue;
                if(filter_status==3 && c->status!=COURSE_STATUS_CLOSED) continue;
                if(strlen(filter_teacher)>0 && strcmp(filter_teacher,"0")!=0 && strcmp(c->teacher_id,filter_teacher)!=0) continue;
                if(!shown) { printf("  %-6s %-16s %-4s %-4s %-6s %-5s %-10s %s\n", "课程号","名称","类型","学分","教师","上限/已选","上课时段","状态"); printf("  --------------------------------------------------------------------------------\n"); shown=1; }
                type_str=(c->type==COURSE_TYPE_REQUIRED)?"必修":"选修";
                if(c->status==COURSE_STATUS_DRAFT) status_str="未发布"; else if(c->status==COURSE_STATUS_SELECT) status_str="选课中"; else if(c->status==COURSE_STATUS_CLOSED) status_str="已结课"; else status_str="?";
                {struct Teacher t;char tn[22]="?";if(ds_teacher_find_by_id(c->teacher_id,&t))strcpy(tn,t.name);printf("  %-6s %-16s %-4s %-4.1f %-6s %2d/%-3d %-10s %s\n", c->id, c->name, type_str, c->credit, tn, c->enrolled, c->max_students, c->schedule, status_str);}
            }
            if(!shown) printf("  无匹配课程。\n");
            printf("\n"); pause_and_continue();
        } else if(ch==3) {
            struct Course c; char id[16], buf[128];
            printf("\n  --- 修改课程信息 ---\n  请输入课程号: ");
            fgets(id,sizeof(id),stdin); id[strcspn(id,"\n")]='\0'; str_trim(id);
            if(!ds_course_find_by_id(id,&c)) { printf("\n  [提示] 未找到课程 %s\n",id); pause_and_continue(); }
            else {
                printf("\n  当前信息（直接回车保留原值）:\n  -------------------------------------------------\n");
                printf("  课程名称 [%s]: ",c.name);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c.name,buf);
                printf("  课程类型(0必修 1选修) [%d]: ",c.type);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)c.type=atoi(buf);
                printf("  学分 [%.1f]: ",c.credit);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)c.credit=(float)atof(buf);
                printf("  教师工号 [%s]: ",c.teacher_id);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c.teacher_id,buf);
                {char new_majors[MAX_MAJORS_LEN+1];printf("  适用专业 [%s] (输入0打开选择器): ",c.majors);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0){if(strcmp(buf,"0")==0){pick_majors_multi("适用专业",new_majors);strcpy(c.majors,new_majors);}else strcpy(c.majors,buf);}}
                printf("  上课时段 [%s]: ",c.schedule);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)strcpy(c.schedule,buf);
                printf("  人数上限 [%d]: ",c.max_students);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)c.max_students=atoi(buf);
                printf("  起始周 [%d]: ",c.start_week);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)c.start_week=atoi(buf);
                printf("  结束周 [%d]: ",c.end_week);fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);if(strlen(buf)>0)c.end_week=atoi(buf);
                if(ds_course_update(id,&c)) printf("\n  [成功] 课程 %s 已更新\n",id); else printf("\n  [失败] 更新失败\n");
                pause_and_continue();
            }
        } else if(ch==4) {
            struct Course c; char id[16], confirm[8];
            printf("\n  --- 删除课程 ---\n  请输入课程号: ");
            fgets(id,sizeof(id),stdin);id[strcspn(id,"\n")]='\0';str_trim(id);
            if(!ds_course_find_by_id(id,&c)){printf("\n  [提示] 未找到课程 %s\n",id);}
            else{{struct Teacher t;char tn[22]="?";if(ds_teacher_find_by_id(c.teacher_id,&t))strcpy(tn,t.name);printf("\n  确认删除：课程号:%s  名称:%s  教师:%s\n  输入 y 确认: ",c.id,c.name,tn);}fgets(confirm,sizeof(confirm),stdin);confirm[strcspn(confirm,"\n")]='\0';
            if(strcmp(confirm,"y")==0||strcmp(confirm,"Y")==0){if(ds_course_delete(id))printf("\n  [成功] 已删除\n");}else printf("\n  已取消\n");}
            pause_and_continue();
        }
    }
}

/*系统基础配置*/
static void sub_sys_config(struct Session* s)
{
    int ch;
    while(1) {
        print_header("系统基础配置");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 设置学期开学日期\n  2. 查看推算教学周\n  3. 维护专业列表\n  4. 设置AI API Key\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) {
            struct SystemConfig cfg; char buf[16];
            ds_config_load(&cfg);
            printf("\n  当前开学日期: %s\n  新开学日期(yyyy-mm-dd): ", cfg.semester_start);
            fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);
            if(strlen(buf)>0) strcpy(cfg.semester_start,buf);
            if(ds_config_save(&cfg)) printf("\n  [成功] 开学日期已更新\n");
            pause_and_continue();
        } else if(ch==2) {
            struct SystemConfig cfg; int w;
            ds_config_load(&cfg);
            printf("\n  开学日期: %s\n\n  === 教学周日历 ===\n\n  %-6s %s\n  --------------------\n", cfg.semester_start, "周次", "周一日期");
            for(w=1; w<=20; w++) { char date_buf[16]; calc_week_start_date(cfg.semester_start, w, date_buf); printf("  第%-4d周 %s\n", w, date_buf); }
            printf("\n"); pause_and_continue();
        } else if(ch==3) {
            int mch;
            while(1) {
                struct SystemConfig cfg; char buf[MAX_MAJORS_LEN+1], new_major[64];
                ds_config_load(&cfg);
                print_header("维护专业列表");
                printf("  当前专业列表:\n");
                {char list[MAX_MAJORS_LEN+1];char* token;int idx;strcpy(list,cfg.majors);idx=1;token=strtok(list,",");while(token!=NULL){while(*token==' ')token++;printf("    %d. %s\n",idx++,token);token=strtok(NULL,",");}if(idx==1)printf("    (空)\n");}
                printf("\n  1. 新增专业\n  2. 删除专业\n  3. 清空并重新输入\n  0. 返回上级\n\n");
                mch=get_choice("  请选择 [0-3]: ", 0, 3);
                if(mch==0) break;
                ds_config_load(&cfg);
                if(mch==1) {
                    printf("\n  新专业名称: "); fgets(new_major,sizeof(new_major),stdin);new_major[strcspn(new_major,"\n")]='\0';str_trim(new_major);
                    if(strlen(new_major)>0){if(strlen(cfg.majors)>0){strcat(cfg.majors,",");strcat(cfg.majors,new_major);}else strcpy(cfg.majors,new_major);ds_config_save(&cfg);printf("  [成功] 已添加: %s\n",new_major);}
                } else if(mch==2) {
                    int del_num, idx, cnt; char list[MAX_MAJORS_LEN+1]; char* items[50];
                    strcpy(list,cfg.majors); cnt=0;
                    {char* token;token=strtok(list,",");while(token!=NULL&&cnt<50){while(*token==' ')token++;items[cnt]=token;cnt++;token=strtok(NULL,",");}}
                    if(cnt==0){printf("\n  [提示] 列表已空\n");}
                    else{del_num=get_choice("  删除第几个专业? ",1,cnt);cfg.majors[0]='\0';
                    for(idx=0;idx<cnt;idx++){if(idx==del_num-1)continue;if(strlen(cfg.majors)>0)strcat(cfg.majors,",");strcat(cfg.majors,items[idx]);}
                    ds_config_save(&cfg);printf("  [成功] 已删除: %s\n",items[del_num-1]);}
                } else if(mch==3) {
                    printf("\n  新专业列表(逗号分隔): ");fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';str_trim(buf);
                    if(strlen(buf)>0){strcpy(cfg.majors,buf);ds_config_save(&cfg);printf("  [成功] 已更新\n");}
                }
                if(mch!=0) pause_and_continue();
            }
        } else if(ch==4) {
            struct SystemConfig cfg; char buf[128];
            ds_config_load(&cfg);
            {int klen=strlen(cfg.api_key);if(klen>6){printf("\n  当前API Key: %.2s****%s\n",cfg.api_key,cfg.api_key+klen-4);}else if(klen>0){printf("\n  当前API Key: ****\n");}else{printf("\n  当前API Key: (未设置)\n");}}
            printf("  新API Key(回车保留): ");
            if(_isatty(_fileno(stdin))){int idx=0;char c;while(1){c=getch();if(c=='\r'||c=='\n'){buf[idx]='\0';printf("\n");break;}if(c=='\b'||c==127){if(idx>0){idx--;printf("\b \b");}}else if(idx<127){buf[idx]=c;idx++;printf("*");}}}
            else{fgets(buf,sizeof(buf),stdin);buf[strcspn(buf,"\n")]='\0';}
            str_trim(buf);
            if(strlen(buf)>0){strcpy(cfg.api_key,buf);if(ds_config_save(&cfg))printf("\n  [成功] API Key已更新\n");else printf("\n  [失败]\n");}
            else printf("\n  未修改。\n");
            pause_and_continue();
        }
    }
}

/*账号信息管理子菜单*/
static void sub_account_mgr(struct Session* s)
{
    int ch;
    while(1) {
        print_header("账号信息管理");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 学生账号管理\n  2. 教师账号管理\n  3. 管理员账号管理\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-3]: ", 0, 3);
        if(ch==0) break;
        if(ch==1) sub_student_mgr(s); else if(ch==2) sub_teacher_mgr(s); else if(ch==3) sub_admin_mgr(s);
    }
}

/*日志分析*/
static void sub_log_analysis(struct Session* s)
{
    int ch, type; char keyword[64];
    while(1) {
        print_header("日志分析");
        printf("  当前用户: %s (管理员)\n\n", s->username);
        printf("  1. 查看最近20条日志\n  2. 按类型筛选日志\n  3. 搜索用户操作记录\n  4. 日志统计概览\n  0. 返回上级\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) { log_show_recent(20); pause_and_continue(); }
        else if(ch==2) {
            printf("\n  日志类型：\n  1.登录 2.退出 3.新增 4.修改 5.删除 6.错误 7.系统\n");
            type=get_choice("  请选择类型 [1-7]: ", 1, 7); log_show_by_type(type); pause_and_continue();
        } else if(ch==3) {
            printf("\n  请输入搜索关键词: "); fgets(keyword,sizeof(keyword),stdin);keyword[strcspn(keyword,"\n")]='\0';
            if(strlen(keyword)>0) log_search_user(keyword);
            pause_and_continue();
        } else if(ch==4) { log_show_stats(); pause_and_continue(); }
    }
}

/*管理员主菜单*/
void admin_menu(struct Session* session)
{
    int ch;
    while(1) {
        print_header("管理员工作台");
        printf("  当前用户: %s (管理员)\n\n", session->username);
        printf("  === 功能菜单 ===\n\n");
        printf("  1. 账号信息管理（学生/教师/管理员）\n");
        printf("  2. 课程全局管理\n");
        printf("  3. 系统基础配置\n");
        printf("  4. 日志分析\n");
        printf("  0. 退出登录\n\n");
        ch=get_choice("请输入选项 [0-4]: ", 0, 4);
        if(ch==0) break;
        if(ch==1) sub_account_mgr(session);
        else if(ch==2) sub_course_mgr(session);
        else if(ch==3) sub_sys_config(session);
        else if(ch==4) sub_log_analysis(session);
    }
}
