import matplotlib.pyplot as plt

# 你的日志文件路径
log_file_path = "/home/nsq/dzworkspace/My_ConfZNS/FEMU-femu-v8.0.0/build-femu/log"

# 你的工作负载
workloads = ['Workload0', 'Workload1', 'Workload2', 'Workload3']

# 从日志文件中读取数据
with open(log_file_path, "r") as file:
    lines = file.readlines()

# 解析数据
data = {workload: [] for workload in workloads}
current_workload = None
for line in lines:
    if "reset" in line:
        parts = line.split(" ")
        current_workload = "Workload" + parts[3]
    elif "queue" in line:
        continue
    elif current_workload in workloads:
        values = [int(part) for part in line.split() if part.isdigit()]
        data[current_workload] = values
    


#中和实验方式带来的误差

# 打印每个工作负载最后一次重置的数据
for workload, values in data.items():
    print(f"{workload} last reset data: {values}")
 # 计算总和并打印
    total_sum = sum(values)
    print(f"{workload} total sum: {total_sum}")
    average_value = total_sum / len(values)
    print(f"{workload}       avg: {average_value}")
    # 计算最大值并打印
    max_value = max(values)
    print(f"{workload} max value: {max_value}")

# 生成折线图
for workload, values in data.items():
    plt.plot(values, label=workload)

plt.legend()

# 保存折线图为PNG图片文件
plt.savefig("workload_data.png")
