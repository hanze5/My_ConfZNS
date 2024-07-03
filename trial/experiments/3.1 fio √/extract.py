import json

# 用您的文件路径替换'your_file.json'
with open('output.json', 'r') as f:
    data = json.load(f)

for i in range(0, len(data)):
    job_names = data[i]["Job name"]
    avg_speed = data[i]["速率"]
    avg_delay = data[i]["延迟"]
    avg_tail_delay = data[i]["99.9%尾延迟"]
    print("===================================")
    print(f"作业名: {job_names}:")
    print(f" 速率: {round(avg_speed)}")
    print(f" 延迟: {round(avg_delay)}")
    print(f" 99.9: {round(avg_tail_delay)}")




# wanted = "速率"
# for i in range(0, len(data)-1, 2):
#     group = data[i:i+2]
#     job_names = [d["Job name"] for d in group]
#     avg_speed = sum(d["速率"] for d in group) / len(group)
#     avg_delay = sum(d["延迟"] for d in group) / len(group)
#     avg_tail_delay = sum(d["99.9%尾延迟"] for d in group) / len(group)
#     print("===================================")
#     print(f"作业名: {job_names}:")
#     print(f" 速率: {round(avg_speed)}")
#     print(f" 延迟: {round(avg_delay)}")
#     print(f" 99.9: {round(avg_tail_delay)}")



# count = 0
# total = 0
# for job in data:
#     if job['Job name'].endswith('_write'):
#         # 使用round函数四舍五入到最接近的整数
#         print(round(job[wanted]))
#         total += round(job[wanted])
#         count+=1
# print(round(total/count))


# print("===================================")
# count = 0
# total = 0
# for job in data:
#     if job['Job name'].endswith('_read'):
#         # 使用round函数四舍五入到最接近的整数
#         print(round(job[wanted]))
#         total += round(job[wanted])
#         count+=1
# print(round(total/count))

# print("===================================")
# count = 0
# total = 0
# for job in data:
#     if job['Job name'].endswith('_randread'):
#         # 使用round函数四舍五入到最接近的整数
#         print(round(job[wanted]))
#         total += round(job[wanted])
#         count+=1
# print(round(total/count))