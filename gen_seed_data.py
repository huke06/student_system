# 种子数据生成脚本 - 智慧教务管理系统
# 生成 data/ 目录下所有dat文件

import os, random

os.makedirs('data', exist_ok=True)
random.seed(2026)

# ====== 基础配置 ======
SEMESTER_START = '2026-07-08'
MAJORS = '计算机科学与技术,软件工程,美术,汉语言文学,数学与应用数学,英语,管理科学,物理学'
API_KEY = ''  # 留空，管理员自行配置
DEFAULT_PWD = '456789'  # 凯撒加密(shift=3)后的"123456"

# ====== 管理员 ======
with open('data/admins.dat', 'w', encoding='utf-8') as f:
    f.write('admin|系统管理员|456789|\n')

# ====== 教师 ======
teachers = [
    ('100001', '张伟',   '男', '计算机科学与技术'),
    ('100002', '李娜',   '女', '美术'),
    ('100003', '王强',   '男', '管理科学'),
    ('100004', '赵敏',   '女', '数学与应用数学'),
    ('100005', '陈静',   '女', '英语'),
]
with open('data/teachers.dat', 'w', encoding='utf-8') as f:
    for tid, name, gender, dept in teachers:
        f.write(f'{tid}|{name}|{gender}|{dept}|13900000000|{DEFAULT_PWD}|\n')

# ====== 课程 ======
# id, name, type(0必修1选修), credit, teacher_id, majors, schedule, max, daily_r, final_r, syllabus, status, sw, ew, sd, ed
# status: 0=草稿 1=选课中 3=已结课
# 开学日期2026-07-08是周三(实际不重要,按周次推算)

def week_date(week_num):
    """根据开学日期推算第N周周一日期"""
    from datetime import datetime, timedelta
    base = datetime(2026, 7, 8)
    # 2026-07-08 是周三，周一应该是07-06
    monday = base - timedelta(days=base.weekday())
    target = monday + timedelta(weeks=week_num - 1)
    return target.strftime('%Y-%m-%d')

courses_raw = [
    # 张伟 计算机 (100001) - 5门
    ('C001', 'C语言程序设计', 0, 4.0, '100001', '计算机科学与技术,软件工程', '周一 第1-2节', 60, 0.4, 0.6,
     'C语言基础语法、指针、结构体、文件操作与项目实战', 1, 1, 16),
    ('C002', '数据结构', 0, 4.0, '100001', '计算机科学与技术,软件工程', '周二 第1-2节', 50, 0.4, 0.6,
     '线性表、栈、队列、树、图、查找与排序算法', 1, 1, 16),
    ('C003', '操作系统', 0, 3.0, '100001', '计算机科学与技术,软件工程', '周三 第3-4节', 50, 0.5, 0.5,
     '进程管理、内存管理、文件系统、设备管理与Shell编程', 1, 1, 12),
    ('C004', 'Python编程', 1, 2.0, '100001', '计算机科学与技术,软件工程,数学与应用数学,物理学', '周五 第5-6节', 40, 0.5, 0.5,
     'Python基础语法、数据处理、可视化入门与爬虫实践', 1, 2, 8),
    ('C005', '计算机网络', 0, 3.5, '100001', '计算机科学与技术,软件工程', '周四 第1-2节', 45, 0.4, 0.6,
     'TCP/IP协议栈、路由交换、网络安全与应用层协议', 1, 1, 14),

    # 李娜 美术 (100002) - 5门
    ('C006', '色彩构成', 0, 3.0, '100002', '美术', '周一 第5-7节', 25, 0.5, 0.5,
     '色彩理论、配色方法、色彩心理学与设计应用实践', 1, 1, 12),
    ('C007', '素描基础', 0, 4.0, '100002', '美术', '周三 第5-7节', 20, 0.5, 0.5,
     '几何体素描、静物写生、人物速写与透视原理', 1, 1, 16),
    ('C008', '中外美术史', 1, 2.0, '100002', '美术,汉语言文学', '周二 第9-11节', 30, 0.5, 0.5,
     '从文艺复兴到现代艺术，中西方美术发展脉络与经典作品赏析', 1, 2, 10),
    ('C009', '设计概论', 0, 3.0, '100002', '美术', '周四 第5-7节', 25, 0.4, 0.6,
     '平面构成、立体构成、设计思维与创意方法论', 1, 1, 12),
    ('C010', '数字绘画', 1, 2.0, '100002', '美术,计算机科学与技术', '周五 第1-2节', 20, 0.5, 0.5,
     'Photoshop与Procreate数字绘画技法、插画创作与商业应用', 1, 2, 8),

    # 王强 管理科学 (100003) - 5门
    ('C011', '管理学原理', 0, 5.0, '100003', '管理科学,软件工程', '周一 第3-4节', 50, 0.5, 0.5,
     '计划、组织、领导、控制四大管理职能的理论与实践案例分析', 1, 1, 16),
    ('C012', '组织行为学', 0, 3.0, '100003', '管理科学', '周三 第1-2节', 45, 0.4, 0.6,
     '个体行为、群体动力、组织文化与变革管理', 1, 1, 12),
    ('C013', '人力资源管理', 1, 2.0, '100003', '管理科学,汉语言文学', '周二 第5-6节', 40, 0.5, 0.5,
     '招聘选拔、培训开发、绩效管理与薪酬设计', 1, 2, 10),
    ('C014', '市场营销', 1, 3.0, '100003', '管理科学,美术', '周四 第3-4节', 40, 0.4, 0.6,
     '市场调研、消费者行为、品牌策略与数字营销', 1, 1, 10),
    ('C015', '财务管理', 0, 4.0, '100003', '管理科学,软件工程', '周五 第3-4节', 45, 0.5, 0.5,
     '财务报表分析、资本预算、风险管控与企业估值', 1, 2, 14),

    # 赵敏 数学 (100004) - 5门
    ('C016', '高等数学', 0, 5.0, '100004', '计算机科学与技术,软件工程,数学与应用数学,物理学', '周一 第1-2节', 80, 0.3, 0.7,
     '函数极限、微积分、级数理论与常微分方程', 3, 1, 16),
    ('C017', '线性代数', 0, 4.0, '100004', '计算机科学与技术,软件工程,数学与应用数学,物理学', '周三 第1-2节', 70, 0.3, 0.7,
     '矩阵理论、向量空间、特征值与线性变换', 3, 1, 14),
    ('C018', '概率论与数理统计', 0, 3.0, '100004', '计算机科学与技术,数学与应用数学,物理学,管理科学', '周二 第3-4节', 60, 0.4, 0.6,
     '概率空间、随机变量、假设检验与回归分析', 1, 2, 12),
    ('C019', '数学建模', 1, 2.0, '100004', '数学与应用数学,计算机科学与技术,物理学', '周五 第5-6节', 35, 0.5, 0.5,
     '从实际问题到数学模型——优化、微分方程与数值计算方法', 1, 2, 8),
    ('C020', '离散数学', 0, 3.0, '100004', '计算机科学与技术,软件工程,数学与应用数学', '周四 第1-2节', 50, 0.4, 0.6,
     '命题逻辑、集合论、图论与组合数学基础', 1, 1, 12),

    # 陈静 英语 (100005) - 5门
    ('C021', '大学英语I', 0, 4.0, '100005', '计算机科学与技术,软件工程,美术,汉语言文学,数学与应用数学,英语,管理科学,物理学', '周二 第1-2节', 60, 0.4, 0.6,
     '听说读写综合训练、四级备考策略与学术英语入门', 1, 1, 16),
    ('C022', '大学英语II', 0, 4.0, '100005', '计算机科学与技术,软件工程,美术,汉语言文学,数学与应用数学,英语,管理科学,物理学', '周四 第1-2节', 60, 0.4, 0.6,
     '高级阅读技巧、学术写作、英语演讲与跨文化交际', 1, 1, 14),
    ('C023', '英语写作', 1, 2.0, '100005', '英语,汉语言文学', '周三 第5-6节', 30, 0.5, 0.5,
     '记叙文、议论文、应用文写作技巧与范文精讲', 1, 2, 10),
    ('C024', '英美文学选读', 1, 3.0, '100005', '英语,汉语言文学', '周五 第3-4节', 35, 0.4, 0.6,
     '莎士比亚戏剧、浪漫主义诗歌与现代英美小说经典选读', 1, 2, 12),
    ('C025', '商务英语', 1, 2.0, '100005', '英语,管理科学', '周一 第5-6节', 30, 0.5, 0.5,
     '商务邮件写作、会议谈判英语与外贸实务场景模拟', 1, 2, 8),
]

with open('data/courses.dat', 'w', encoding='utf-8') as f:
    for (cid, name, ctype, credit, tid, majors, schedule, max_s,
         dr, fr, syllabus, status, sw, ew) in courses_raw:
        sd = week_date(sw)
        ed = week_date(ew)
        f.write(f'{cid}|{name}|{ctype}|{credit}|{tid}|{majors}|{schedule}|{max_s}|0|{dr}|{fr}|{syllabus}|{status}|{sw}|{ew}|{sd}|{ed}|\n')

# ====== 学生 ======
students_raw = [
    # 计算机科学与技术 (4)
    ('2026000001', '林晓峰', '男', '2026', '计算机科学与技术'),
    ('2026000002', '陈宇航', '男', '2026', '计算机科学与技术'),
    ('2026000003', '黄思涵', '女', '2026', '计算机科学与技术'),
    ('2026000004', '周子轩', '男', '2026', '计算机科学与技术'),
    # 软件工程 (4)
    ('2026000005', '吴昊天', '男', '2026', '软件工程'),
    ('2026000006', '郑雨桐', '女', '2026', '软件工程'),
    ('2026000007', '孙浩宇', '男', '2026', '软件工程'),
    ('2026000008', '马晓婷', '女', '2026', '软件工程'),
    # 美术 (4)
    ('2026000009', '赵艺涵', '女', '2026', '美术'),
    ('2026000010', '钱一鸣', '男', '2026', '美术'),
    ('2026000011', '沈雨萱', '女', '2026', '美术'),
    ('2026000012', '韩梦琪', '女', '2026', '美术'),
    # 汉语言文学 (3)
    ('2026000013', '杨文博', '男', '2026', '汉语言文学'),
    ('2026000014', '朱雅婷', '女', '2026', '汉语言文学'),
    ('2026000015', '秦思源', '女', '2026', '汉语言文学'),
    # 数学与应用数学 (4)
    ('2026000016', '许明达', '男', '2026', '数学与应用数学'),
    ('2026000017', '何雨晴', '女', '2026', '数学与应用数学'),
    ('2026000018', '吕志远', '男', '2026', '数学与应用数学'),
    ('2026000019', '施晓晨', '女', '2026', '数学与应用数学'),
    # 英语 (4)
    ('2026000020', '张馨月', '女', '2026', '英语'),
    ('2026000021', '孔维佳', '女', '2026', '英语'),
    ('2026000022', '曹悦然', '女', '2026', '英语'),
    ('2026000023', '严子豪', '男', '2026', '英语'),
    # 管理科学 (4)
    ('2026000024', '金正熙', '男', '2026', '管理科学'),
    ('2026000025', '魏佳琳', '女', '2026', '管理科学'),
    ('2026000026', '陶致远', '男', '2026', '管理科学'),
    ('2026000027', '姜雨萌', '女', '2026', '管理科学'),
    # 物理学 (3)
    ('2026000028', '谢明哲', '男', '2026', '物理学'),
    ('2026000029', '苏雨桐', '女', '2026', '物理学'),
    ('2026000030', '潘昊天', '男', '2026', '物理学'),
]

with open('data/students.dat', 'w', encoding='utf-8') as f:
    for sid, name, gender, grade, major in students_raw:
        f.write(f'{sid}|{name}|{gender}|{grade}|{major}|13800000000|{DEFAULT_PWD}|\n')

# ====== 选课分配 ======
# 课程ID -> 适用专业列表(从courses_raw解析)
course_majors = {}
course_teacher = {}
course_status = {}
for c in courses_raw:
    cid = c[0]
    course_teacher[cid] = c[4]
    course_status[cid] = c[12]
    course_majors[cid] = [m.strip() for m in c[5].split(',')]

# 每个学生选5-8门课
student_major = {s[0]: s[4] for s in students_raw}
selections = []
sel_id = 1

for sid, name, gender, grade, major in students_raw:
    my_courses = []
    for cid, cmajors in course_majors.items():
        if major in cmajors:
            my_courses.append(cid)
    # 随机选5-8门
    n = min(len(my_courses), random.randint(5, 8))
    chosen = random.sample(my_courses, n)
    for cid in chosen:
        # 检查时间冲突(简单避免同一天同一时段)
        selections.append((sel_id, sid, cid))
        sel_id += 1

# 更新enrolled计数
enrolled_count = {}
for _, _, cid in selections:
    enrolled_count[cid] = enrolled_count.get(cid, 0) + 1

# 重写courses.dat并写入enrolled
with open('data/courses.dat', 'w', encoding='utf-8') as f:
    for (cid, name, ctype, credit, tid, majors, schedule, max_s,
         dr, fr, syllabus, status, sw, ew) in courses_raw:
        enr = enrolled_count.get(cid, 0)
        if enr > max_s:  # 如果选课人数超上限则状态改为已满(仍为选课中)
            enr = max_s
        sd = week_date(sw)
        ed = week_date(ew)
        f.write(f'{cid}|{name}|{ctype}|{credit}|{tid}|{majors}|{schedule}|{max_s}|{enr}|{dr}|{fr}|{syllabus}|{status}|{sw}|{ew}|{sd}|{ed}|\n')

# ====== 选课记录 ======
with open('data/selections.dat', 'w', encoding='utf-8') as f:
    import datetime
    now = datetime.datetime(2026, 7, 10, 9, 0, 0)
    for rid, sid, cid in selections:
        ts = now + datetime.timedelta(minutes=random.randint(0, 480))
        f.write(f'{rid}|{sid}|{cid}|{ts.strftime("%Y-%m-%d %H:%M:%S")}|0|\n')

# ====== 成绩录入 ======
# 张伟(100001)结课C001和C002, 赵敏(100004)结课C016和C017
# 把这几门课状态改为3, 然后给选了这些课的学生打分
closed_courses = {
    'C001': '100001', 'C002': '100001',
    'C016': '100004', 'C017': '100004',
}

# 更新courses.dat中这几门的状态
all_courses = []
with open('data/courses.dat', 'r', encoding='utf-8') as f:
    for line in f:
        line = line.strip()
        if not line: continue
        fld = line.split('|')
        if fld[0] in closed_courses:
            fld[12] = '3'  # status = 已结课
        all_courses.append('|'.join(fld))
with open('data/courses.dat', 'w', encoding='utf-8') as f:
    for c in all_courses:
        f.write(c + '|\n')

# 给选了结课课程的学生打分
score_id = 1
with open('data/scores.dat', 'w', encoding='utf-8') as f:
    import datetime
    now = datetime.datetime(2026, 7, 13, 14, 0, 0)
    for _, sid, cid in selections:
        if cid not in closed_courses:
            continue
        daily = round(random.uniform(50, 100), 1)
        final = round(random.uniform(50, 100), 1)
        # 从courses_raw找占比
        dr_val, fr_val = 0.5, 0.5
        for c in courses_raw:
            if c[0] == cid:
                dr_val, fr_val = c[8], c[9]
                break
        total = round(daily * dr_val + final * fr_val, 1)
        # 绩点换算
        if total >= 90: gpa = 4.0
        elif total >= 85: gpa = 3.7
        elif total >= 82: gpa = 3.3
        elif total >= 78: gpa = 3.0
        elif total >= 75: gpa = 2.7
        elif total >= 72: gpa = 2.3
        elif total >= 68: gpa = 2.0
        elif total >= 64: gpa = 1.5
        elif total >= 60: gpa = 1.0
        else: gpa = 0.0
        ts = now + datetime.timedelta(minutes=random.randint(0, 120))
        f.write(f'{score_id}|{sid}|{cid}|{daily}|{final}|{total}|{gpa:.2f}|{ts.strftime("%Y-%m-%d %H:%M:%S")}|{closed_courses[cid]}|\n')
        score_id += 1

# ====== 系统配置 ======
with open('data/config.dat', 'w', encoding='utf-8') as f:
    f.write(f'{SEMESTER_START}|{MAJORS}|{API_KEY}|\n')

# ====== 统计 ======
stu_count = len(students_raw)
tea_count = len(teachers)
course_count = len(courses_raw)
sel_count = len(selections)
score_count = score_id - 1
closed_count = len(closed_courses)

print(f'数据生成完毕:')
print(f'  管理员: 1')
print(f'  教师: {tea_count}')
print(f'  学生: {stu_count} ({len(set(s[4] for s in students_raw))}个专业)')
print(f'  课程: {course_count} (已结课{closed_count}门)')
print(f'  选课记录: {sel_count}')
print(f'  成绩记录: {score_count}')
print(f'  开学日期: {SEMESTER_START}')
print(f'  专业: {MAJORS}')
