/*
 * page.h - 页面渲染函数声明
 * 每个函数返回完整的HTML页面字符串
 * username: 用户显示名, userid: 用户账号
 */

#ifndef PAGE_H
#define PAGE_H

/* ===== 公共页面 ===== */
char* page_login(const char* error_msg);
char* page_404(void);
const char* get_layout_css(void);

/* ===== 管理员页面 ===== */
char* page_admin_home(const char* username, const char* userid);
char* page_admin_student_mgr(const char* username, const char* userid);
char* page_admin_teacher_mgr(const char* username, const char* userid);
char* page_admin_admin_mgr(const char* username, const char* userid);
char* page_admin_select_config(const char* username, const char* userid);
char* page_admin_select_force(const char* username, const char* userid);
char* page_admin_select_monitor(const char* username, const char* userid);
char* page_admin_course_list(const char* username, const char* userid);
char* page_admin_course_edit(const char* username, const char* userid);
char* page_admin_course_delete(const char* username, const char* userid);
char* page_admin_sys_semester(const char* username, const char* userid);
char* page_admin_sys_majors(const char* username, const char* userid);

/* ===== 教师页面 ===== */
char* page_teacher_home(const char* username, const char* userid);
char* page_teacher_course_add(const char* username, const char* userid);
char* page_teacher_course_edit(const char* username, const char* userid);
char* page_teacher_course_list(const char* username, const char* userid);
char* page_teacher_roster(const char* username, const char* userid);
char* page_teacher_grade_input(const char* username, const char* userid);
char* page_teacher_grade_edit(const char* username, const char* userid);
char* page_teacher_grade_stats(const char* username, const char* userid);
char* page_teacher_course_close(const char* username, const char* userid);

/* ===== 学生页面 ===== */
char* page_student_home(const char* username, const char* userid);
char* page_student_my_info(const char* username, const char* userid);
char* page_student_edit_info(const char* username, const char* userid);
char* page_student_change_pwd(const char* username, const char* userid);
char* page_student_course_select(const char* username, const char* userid);
char* page_student_my_schedule(const char* username, const char* userid);
char* page_student_my_grades(const char* username, const char* userid);
char* page_student_grade_stats(const char* username, const char* userid);
char* page_student_ai_analysis(const char* username, const char* userid);

#endif /* PAGE_H */
