/*AI辅助功能声明，通过DeepSeek API实现智能选课推荐和成绩分析*/

#ifndef AI_HELPER_H
#define AI_HELPER_H

/*调用DeepSeek API发送对话请求
 system_prompt:系统提示词（定义AI角色）
 user_message:用户消息
 reply:输出缓冲区，存放AI回复文本
 max_len:reply 缓冲区最大长度
 返回：1成功，0失败
 */
int ai_call(const char* system_prompt, const char* user_message,
            char* reply, int max_len);

/*
智能选课推荐
根据学生专业+兴趣+规划，推荐合适课程
student_id:学号
interests:学生填写的兴趣爱好(可为NULL或空)
future_plan:未来规划(可为NULL或空)
reply:输出推荐结果
max_len：缓冲区长度
返回：1成功，0失败
 */
int ai_course_recommend(const char* student_id,
                        const char* interests, const char* future_plan,
                        char* reply, int max_len);

/*
智能成绩分析
分析学生已出成绩，输出画像和改进建议
student_id: 学号
reply:输出分析结果
max_len:缓冲区长度
返回：1成功，0失败
 */
int ai_grade_analysis(const char* student_id, char* reply, int max_len);

#endif /* AI_HELPER_H */
