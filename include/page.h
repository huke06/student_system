/*
 * page.h - 页面渲染函数声明
 * 重构后只保留核心渲染接口 + 兼容旧接口
 */

#ifndef PAGE_H
#define PAGE_H

/*
 * 通用页面渲染
 * template: HTML模板文件路径
 * username/userid/role: 用户信息
 * active: 当前页面路径（侧边栏高亮）
 * error_msg: 错误提示（仅登录页使用，其他传NULL）
 * 返回：完整HTML字符串
 */
char* page_render(const char* template, const char* username,
                  const char* userid, const char* role,
                  const char* active, const char* error_msg);

/* 兼容旧调用 */
char* page_login(const char* error_msg);
char* page_404(void);
const char* get_layout_css(void);

#endif /* PAGE_H */
