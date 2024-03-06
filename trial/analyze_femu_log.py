import matplotlib.pyplot as plt
import numpy as np

def is_float(value):
    try:
        float(value)
        return True
    except ValueError:
        return False

# 读取日志文件
with open('/home/nsq/dzworkspace/My_ConfZNS/FEMU-femu-v8.0.0/build-femu/log', 'r') as f:
    lines = f.readlines()

# 初始化数据
workloads = ['Workload0', 'Workload1', 'Workload2', 'Workload3']
records = [[] for _ in range(len(workloads))]
pressures = [[] for _ in range(len(workloads))]



# 解析日志文件
for line in lines:
    if '距离上次打印 添加记录数' in line:
        record_data = line.split('|')
        for i, data in enumerate(record_data):
            if 'Workload' in data:
                records[i].append(int(data.split(':')[-1].strip()))
    elif 'workload 压力情况' in line:
        pressure_data = line.split('|')
        for i, data in enumerate(pressure_data):
            if 'Workload' in data:
                pressures[i].append(float(data.split(':')[-1].strip()))

begin = 2050  # 你可以根据需要设置这个值
end = None

# 保留 begin 之后的记录
records = [record[begin:end] for record in records]
pressures = [pressure[begin:end] for pressure in pressures]
# 打印数据
print("Records:", records)
print("Pressures:", pressures)

avg_records = [np.mean(r) for r in records]
avg_pressures = [np.mean(p) for p in pressures]

# 创建记录图
plt.figure(figsize=(10, 5))
for i in range(len(workloads)):
    plt.plot(records[i], label='{}'.format(workloads[i]), linewidth=0.5)
plt.title('Record Graph')
plt.xlabel('Time')
plt.ylabel('Record Count')
plt.legend()  # 显示图例
plt.savefig('Record_Graph.png')  # 保存图像
plt.close()

# 创建压力图
plt.figure(figsize=(10, 5))
for i in range(len(workloads)):
    plt.plot(pressures[i], label='{}'.format(workloads[i]), linewidth=0.5)
plt.title('Pressure Graph')
plt.xlabel('Time')
plt.ylabel('Pressure')
plt.legend()  # 显示图例
plt.savefig('Pressure_Graph.png')  # 保存图像
plt.close()

# 创建记录的柱状图
plt.figure(figsize=(10, 5))
plt.bar(workloads, avg_records, color='b', alpha=0.7)
plt.title('Average Record Count per Workload')
plt.xlabel('Workload')
plt.ylabel('Average Record Count')
plt.savefig('Average_Record_Bar_Graph.png')  # 保存图像
plt.close()

# 创建压力的柱状图
plt.figure(figsize=(10, 5))
plt.bar(workloads, avg_pressures, color='r', alpha=0.7)
plt.title('Average Pressure per Workload')
plt.xlabel('Workload')
plt.ylabel('Average Pressure')
plt.savefig('Average_Pressure_Bar_Graph.png')  # 保存图像
plt.close()