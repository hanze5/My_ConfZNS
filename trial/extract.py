import json

# 用您的文件路径替换'your_file.json'
with open('output.json', 'r') as f:
    data = json.load(f)

wanted = "IOPS"

for job in data:
    if job['Job name'].endswith('_write'):
        # 使用round函数四舍五入到最接近的整数
        print(round(job[wanted]))

print("===================================")

for job in data:
    if job['Job name'].endswith('_read'):
        # 使用round函数四舍五入到最接近的整数
        print(round(job[wanted]))
print("===================================")
for job in data:
    if job['Job name'].endswith('_randread'):
        # 使用round函数四舍五入到最接近的整数
        print(round(job[wanted]))