/*
 * utils.h - 工具函数与加密模块声明
 * 包含：字符串处理、ID校验、成绩计算、时间冲突判定、周次推算、MD5加密
 */

#ifndef UTILS_H
#define UTILS_H

/* ========== 字符串处理 ========== */

/*
 * 去除字符串首尾空格和换行符
 * str: 待处理的字符串（原地修改）
 */
void str_trim(char* str);

/* ========== ID与号码校验 ========== */

/*
 * 校验学号格式：必须为10位数字
 * 返回：1有效，0无效
 */
int check_student_id(const char* id);

/*
 * 校验教师工号格式：必须为6位数字
 * 返回：1有效，0无效
 */
int check_teacher_id(const char* id);

/*
 * 校验中国大陆手机号格式（11位数字，1开头）
 * 返回：1有效，0无效
 */
int check_phone(const char* phone);

/* ========== 初始密码生成 ========== */

/*
 * 根据账号生成初始密码（取账号后6位）
 * id: 学号或工号
 * pwd: 输出参数，生成的6位密码字符串
 */
void gen_init_pwd(const char* id, char* pwd);

/* ========== 成绩计算 ========== */

/*
 * 根据平时成绩和期末成绩计算总评成绩
 * daily_score: 平时成绩(0-100)
 * final_score: 期末成绩(0-100)
 * daily_ratio: 平时占比(0-1)
 * final_ratio: 期末占比(0-1)
 * 返回：总评成绩，保留1位小数
 */
float calc_total_score(float daily_score, float final_score,
                       float daily_ratio, float final_ratio);

/*
 * 将百分制总评成绩转换为4分制绩点
 * score: 总评成绩(0-100)
 * 返回：对应绩点
 */
float score_to_gpa(float score);

/*
 * 计算加权平均成绩
 * scores: 成绩数组
 * credits: 对应学分数组
 * count: 课程数量
 * 返回：加权平均成绩
 */
float calc_weighted_avg(const float* scores, const float* credits, int count);

/*
 * 计算加权平均绩点(GPA)
 * gpas: 绩点数组
 * credits: 对应学分数组
 * count: 课程数量
 * 返回：加权平均绩点
 */
float calc_weighted_gpa(const float* gpas, const float* credits, int count);

/* ========== 选课冲突判定 ========== */

/*
 * 解析上课时段字符串，提取星期和节次范围
 * schedule: 格式如 "周一 第1-2节" 或 "周一 第3-4节"
 * weekday: 输出参数，星期几(1-7)
 * start_period: 输出参数，起始节次
 * end_period: 输出参数，结束节次
 * 返回：1解析成功，0失败
 */
int parse_schedule(const char* schedule, int* weekday,
                   int* start_period, int* end_period);

/*
 * 判断两门课程是否存在时间冲突
 * 规则：上课星期+节次有重叠，且周次范围有重叠
 * sched1, sched2: 上课时段字符串
 * w1_start, w1_end: 课程1的起止周次
 * w2_start, w2_end: 课程2的起止周次
 * 返回：1冲突，0不冲突
 */
int check_time_conflict(const char* sched1, int w1_start, int w1_end,
                        const char* sched2, int w2_start, int w2_end);

/* ========== 周次与日期对应 ========== */

/*
 * 根据学期开学日期和第N周，推算该周周一的日期
 * semester_start: 学期开学日期(第一周周一)，格式"yyyy-mm-dd"
 * week_num: 第几教学周(从1开始)
 * date_out: 输出参数，该周周一的日期字符串"yyyy-mm-dd"
 */
void calc_week_start_date(const char* semester_start, int week_num,
                          char* date_out);

/*
 * 判断一个日期是否在指定周次范围内
 * date: 日期字符串"yyyy-mm-dd"
 * semester_start: 开学日期
 * start_week: 起始周次
 * end_week: 结束周次
 * 返回：1在范围内，0不在
 */
int date_in_week_range(const char* date, const char* semester_start,
                       int start_week, int end_week);

/* ========== 凯撒密码加密 ========== */

/*
 * 凯撒密码加密
 * 将字符串中每个字符按shift偏移量移位
 * 字母和数字循环移位，其他字符不变
 * input: 明文字符串
 * shift: 偏移量(1-25)
 * output: 密文字符串（需预先分配与input相同长度+1的空间）
 */
void caesar_encrypt(const char* input, int shift, char* output);

/*
 * 凯撒密码解密（偏移量取反即可）
 */
void caesar_decrypt(const char* input, int shift, char* output);

#endif /* UTILS_H */
