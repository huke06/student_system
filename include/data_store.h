/*数据持久化层声明，纯文本文件存储，管道符分隔，支持CRUD*/
#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "common.h"
/*初始化数据存储系统，首次运行时创建data目录和种子数据，返回1成功*/
int ds_init(void);

/*学生数据操作*/
int ds_student_load_all(struct Student* students, int* count);
int ds_student_find_by_id(const char* id, struct Student* out);
int ds_student_add(const struct Student* s);
int ds_student_update(const char* id, const struct Student* s);
int ds_student_delete(const char* id);

/*教师数据操作*/
int ds_teacher_load_all(struct Teacher* teachers, int* count);
int ds_teacher_find_by_id(const char* id, struct Teacher* out);
int ds_teacher_add(const struct Teacher* t);
int ds_teacher_update(const char* id, const struct Teacher* t);
int ds_teacher_delete(const char* id);

/*管理员数据操作*/
int ds_admin_load_all(struct Admin* admins, int* count);
int ds_admin_find_by_id(const char* id, struct Admin* out);
int ds_admin_add(const struct Admin* a);
int ds_admin_update(const char* id, const struct Admin* a);
int ds_admin_delete(const char* id);

/*课程数据操作*/
int ds_course_load_all(struct Course* courses, int* count);
int ds_course_find_by_id(const char* id, struct Course* out);
int ds_course_add(const struct Course* c);
int ds_course_update(const char* id, const struct Course* c);
int ds_course_delete(const char* id);

/*选课记录操作*/
int ds_selection_load_all(struct Selection* selections, int* count);
int ds_selection_find_by_id(int record_id, struct Selection* out);
int ds_selection_find_by_sc(const char* student_id, const char* course_id, struct Selection* out);
int ds_selection_load_by_student(const char* student_id, struct Selection* selections, int* count);
int ds_selection_add(const struct Selection* s);
int ds_selection_delete(int record_id);

/*成绩记录操作*/
int ds_score_load_all(struct Score* scores, int* count);
int ds_score_find_by_id(int record_id, struct Score* out);
int ds_score_find_by_sc(const char* student_id, const char* course_id, struct Score* out);
int ds_score_load_by_student(const char* student_id, struct Score* scores, int* count);
int ds_score_add(const struct Score* s);
int ds_score_update(int record_id, const struct Score* s);

/*系统配置操作*/
int ds_config_load(struct SystemConfig* cfg);
int ds_config_save(const struct SystemConfig* cfg);
#endif /*DATA_STORE_H*/
