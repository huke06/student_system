/* utils.h - 工具函数与加密模块声明：字符串处理、ID校验、成绩计算、冲突判定、周次推算、凯撒加密 */

#ifndef UTILS_H
#define UTILS_H

/* 字符串处理 */
void str_trim(char* str);

/* ID与号码校验 */
int check_student_id(const char* id);
int check_teacher_id(const char* id);
int check_phone(const char* phone);

/* 初始密码生成，根据账号生成初始密码（取账号后6位） */
void gen_init_pwd(const char* id, char* pwd);

/* 成绩计算 */
float calc_total_score(float daily_score, float final_score, float daily_ratio, float final_ratio);
float score_to_gpa(float score);
float calc_weighted_avg(const float* scores, const float* credits, int count);
float calc_weighted_gpa(const float* gpas, const float* credits, int count);

/* 选课冲突判定 */
int parse_schedule(const char* schedule, int* weekday, int* start_period, int* end_period);
int check_time_conflict(const char* sched1, int w1_start, int w1_end, const char* sched2, int w2_start, int w2_end);

/* 周次与日期对应 */
void calc_week_start_date(const char* semester_start, int week_num, char* date_out);
int date_in_week_range(const char* date, const char* semester_start, int start_week, int end_week);

/* 凯撒密码加密与解密，shift为偏移量 */
void caesar_encrypt(const char* input, int shift, char* output);
void caesar_decrypt(const char* input, int shift, char* output);

#endif /* UTILS_H */
