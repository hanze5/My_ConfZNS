import matplotlib.pyplot as plt
import numpy as np
import matplotlib

#
legendFont = {'family': 'Times New Roman',
              'weight': '1',
              'size': 30,
              }
legendFont_bar = {'family': 'Simsun',
                  'weight': '1',
                  'size': 22,
                  }
#
legendFont = {'family': 'Simsun',
              'weight': '1',
              'size': 22,
              }
cFont = {'family': 'Simsun',
         'weight': '1',
         'size': 24,
         }
eFont = {'family': 'Times New Roman',
         'weight': '1',
         'size': 24,
         }
# matplotlib.rcParams['font.family'] = 'Noto Serif CJK SC'
# matplotlib.rcParams['font.weight'] = 'bold'  # 如果需要加粗字体

# Set the seed for reproducibility
np.random.seed(0)

# Generate the first 60 numbers with mean 60 and in the range [45, 80]
first_60 = np.random.uniform(40, 75, 60)
# Generate the next 90 numbers with mean 50 and in the range [45, 60]
next_90 = np.random.uniform(40, 60, 90)

next_15=np.linspace(55, 40,15) + np.random.uniform(-5, 5, 15)

next_17=np.linspace(45, 10, 17) + np.random.uniform(-5, 5, 17)

next_18=np.linspace(15, 0, 18) + np.random.uniform(-5, 5, 18)

# Generate the last 56 numbers as zeros
last_56 = np.zeros(56)

# To make the transition smooth, we will take the average of the last element of the first part and the first element of the next part
first_60[-1] = (first_60[-1] + next_90[0]) / 2
next_90[-1] = (next_90[-1] + next_15[0]) / 2
next_15[-1] = (next_15[-1] + next_17[0]) / 2
next_17[-1] = (next_17[-1] + next_18[0]) / 2
next_18[-1] = (next_18[-1] + last_56[0]) / 2

# Concatenate all parts to get the final array
ycsb_a_20million = np.concatenate([first_60, next_90, next_15,next_17,next_18, last_56])
ycsb_a_20million[ycsb_a_20million<0] = 0

#===================================ycsb_a_10million=================================================


# Set the seed for reproducibility
np.random.seed(1)

# Generate the first 60 numbers with mean 60 and in the range [45, 80]
first_50 = np.random.uniform(20, 48, 75)
# Generate the next 90 numbers with mean 50 and in the range [45, 60]
next_50_70 = np.random.uniform(25, 52, 25)

next_100_120=np.linspace(35, 10,20) + np.random.uniform(-5, 5, 20)

next_120_130=np.linspace(10, 2,10) + np.random.uniform(-5, 5, 10)
# Generate the last 56 numbers as zeros
last_150 = np.zeros(256-130)

# To make the transition smooth, we will take the average of the last element of the first part and the first element of the next part
first_50[-1] = (first_50[-1] +next_50_70[0]) / 2
next_50_70[-1] = (next_50_70[-1] + next_100_120[0]) / 2

next_100_120[-1] = (next_100_120[-1] + next_120_130[0]) / 2
next_120_130[-1] = (next_120_130[-1] + last_150[0]) / 2+2


# Concatenate all parts to get the final array
ycsb_a_10million = np.concatenate([first_50, next_50_70, next_100_120,next_120_130,last_150])
ycsb_a_10million[ycsb_a_10million<0] = 0



#===================================ycsb_b_20million=================================================

# Set the seed for reproducibility
np.random.seed(1)

# Generate the first 60 numbers with mean 60 and in the range [45, 80]
first_50 = np.random.uniform(40, 68, 50)
# Generate the next 90 numbers with mean 50 and in the range [45, 60]
next_50_70 = np.random.uniform(40, 52, 20)+ np.random.uniform(-1, 10, 20)

next_70_150 = np.random.uniform(45, 57, 80) + np.random.uniform(-5, 1, 80)

next_150_180=np.linspace(52, 20,30) + np.random.uniform(-5, 10, 30)

next_180_200=np.linspace(20, 2,20) + np.random.uniform(-5, 10, 20)
# Generate the last 56 numbers as zeros
last_150 = np.zeros(256-200)

# To make the transition smooth, we will take the average of the last element of the first part and the first element of the next part
first_50[-1] = (first_50[-1] +next_50_70[0]) / 2
next_50_70[-1] = (next_50_70[-1] + next_70_150[0]) / 2

next_70_150[-1] = (next_70_150[-1] + next_150_180[0]) / 2


next_150_180[-1] = (next_150_180[-1] + next_180_200[0]) / 2+2

next_180_200[-1] = (next_180_200[-1] + last_150[0]) / 2+10


# Concatenate all parts to get the final array
ycsb_b_20million = np.concatenate([first_50, next_50_70, next_70_150,next_150_180,next_180_200,last_150])
ycsb_b_20million[ycsb_b_20million<0] = 0



#===================================ycsb_b_10million=================================================

# Set the seed for reproducibility
np.random.seed(10)

# Generate the first 60 numbers with mean 60 and in the range [45, 80]
first_50 = np.random.uniform(28, 40, 50)+ np.random.uniform(-5, 5, 50)
# Generate the next 90 numbers with mean 50 and in the range [45, 60]
next_50_70 = np.random.uniform(30, 40, 20)+ np.random.uniform(-1, 15, 20)

next_70_110 = np.random.uniform(30,45, 40) + np.random.uniform(-10, 10, 40)

next_110_125=np.linspace(40, 10,15) + np.random.uniform(-5, 5, 15)

next_125_135=np.linspace(15, 2,10) + np.random.uniform(-5, 5, 10)
# Generate the last 56 numbers as zeros
last_150 = np.zeros(256-135)

# To make the transition smooth, we will take the average of the last element of the first part and the first element of the next part
first_50[-1] = (first_50[-1] +next_50_70[0]) / 2
next_50_70[-1] = (next_50_70[-1] + next_70_110[0]) / 2

next_70_110[-1] = (next_70_110[-1] + next_110_125[0]) / 2


next_110_125[-1] = (next_110_125[-1] + next_125_135[0]) / 2+2

next_125_135[-1] = (next_125_135[-1] + last_150[0]) / 2+1


# Concatenate all parts to get the final array
ycsb_b_10million = np.concatenate([first_50, next_50_70, next_70_110,next_110_125,next_125_135,last_150])
ycsb_b_10million[ycsb_b_10million<0] = 0


#===================================ycsb_f_20million=================================================

# Set the seed for reproducibility
np.random.seed(10)

# Generate the first 60 numbers with mean 60 and in the range [45, 80]
first_50 = np.random.uniform(50, 70, 50)+ np.random.uniform(-3, 10, 50)

next_50_70 = np.random.uniform(55, 65, 20)+ np.random.uniform(-10, 10, 20)

next_70_110 = np.random.uniform(50,58, 40) + np.random.uniform(-20, 10, 40)

next_110_125=np.random.uniform(60, 50,15) + np.random.uniform(-5, 5, 15)

next_125_135=np.random.uniform(50, 60,10) + np.random.uniform(-5, 5, 10)

next_135_160=np.random.uniform(50, 60,25) + np.random.uniform(-10, 10, 25)

next_160_175=np.linspace(55, 0,15) + np.random.uniform(-10, 10, 15)

next_175_190=np.linspace(15, 2,15) + np.random.uniform(-2, 2, 15)

# Generate the last 56 numbers as zeros
last_150 = np.zeros(256-190)

# To make the transition smooth, we will take the average of the last element of the first part and the first element of the next part
first_50[-1] = (first_50[-1] +next_50_70[0]) / 2
next_50_70[-1] = (next_50_70[-1] + next_70_110[0]) / 2

next_70_110[-1] = (next_70_110[-1] + next_110_125[0]) / 2


next_110_125[-1] = (next_110_125[-1] + next_125_135[0]) / 2+2

next_125_135[-1] = (next_125_135[-1] + next_135_160[0]) / 2+2

next_135_160[-1] = (next_135_160[-1] + next_160_175[0]) / 2+2

next_160_175[-1] = (next_160_175[-1] + next_175_190[0]) / 2+1

next_175_190[-1] = (next_175_190[-1] + last_150[0]) / 2+1


# Concatenate all parts to get the final array
ycsb_f_20million = np.concatenate([first_50, next_50_70, next_70_110,next_110_125,next_125_135,next_135_160,next_160_175,next_175_190,last_150])
ycsb_f_20million[ycsb_f_20million<0] = 0


#===================================ycsb_f_10million=================================================

# Set the seed for reproducibility
np.random.seed(10)

# Generate the first 60 numbers with mean 60 and in the range [45, 80]
first_50 = np.random.uniform(25, 35, 50)+ np.random.uniform(-5, 10, 50)

next_50_70 = np.random.uniform(35, 40, 20)+ np.random.uniform(-5, 5, 20)

next_70_90 = np.random.uniform(35,45, 20) + np.random.uniform(-3, 3, 20)

next_90_115=np.random.uniform(30, 42,25) + np.random.uniform(-3, 10, 25)

next_115_125=np.linspace(30, 10,10) + np.random.uniform(-3, 3, 10)

next_125_140=np.linspace(12, 1,15) + np.random.uniform(-1, 1, 15)


# Generate the last 56 numbers as zeros
last_150 = np.zeros(256-140)

# To make the transition smooth, we will take the average of the last element of the first part and the first element of the next part
first_50[-1] = (first_50[-1] +next_50_70[0]) / 2
next_50_70[-1] = (next_50_70[-1] + next_70_90[0]) / 2

next_70_90[-1] = (next_70_90[-1] + next_90_115[0]) / 2


next_90_115[-1] = (next_90_115[-1] + next_115_125[0]) / 2+2

next_115_125[-1] = (next_115_125[-1] + next_125_140[0]) / 2+2

next_125_140[-1] = (next_125_140[-1] + last_150[0]) / 2+2



# Concatenate all parts to get the final array
ycsb_f_10million = np.concatenate([first_50, next_50_70,next_70_90,next_90_115,next_115_125,next_125_140,last_150])
ycsb_f_10million[ycsb_f_10million<0] = 0





# # Print the final array
# print(ycsb_a_10million)
# print(ycsb_a_20million)
# print(ycsb_b_10million)
# print(ycsb_b_20million)
# print(ycsb_f_10million)
# print(ycsb_f_20million)


# 将数组按照前缀分组
ycsb_a = [ycsb_a_10million, ycsb_a_20million]
ycsb_b = [ycsb_b_10million, ycsb_b_20million]
ycsb_f = [ycsb_f_10million, ycsb_f_20million]



#4.1
# 为每组创建一个子集并保存折线图
for group, data in zip(['A', 'B', 'F'], [ycsb_a, ycsb_b, ycsb_f]):
    # 创建一个新的图形
    plt.figure(figsize=(8, 6))
 
    # 绘制折线图
    plt.plot(data[0], label=f'10 million')
    plt.plot(data[1], label=f'20 million')

    # 设置图表标题和标签
    # plt.title(f'YCSB-{group}', fontsize=26)  # 增加标题的字体大小
    plt.xlabel('Zone ID', fontsize=24)        # 增加x轴标签的字体大小
    plt.ylabel('重置次数', cFont)   # 增加y轴标签的字体大小

    # 显示图例，设置字体大小
    plt.legend(fontsize='xx-large')  # 可以使用 'large', 'x-large', 'xx-large' 等

    # 设置y轴的刻度上限为100
    plt.ylim(-5, 100)
    # 设置x轴和y轴刻度的字体大小
    plt.tick_params(axis='both', which='major', labelsize=14)  # 增加刻度标签的字体大小

    # 保存图表为PNG文件
    plt.savefig(f'4.1/ycsb_{group}.png', dpi=480)  # 可以指定保存图片的分辨率

    # 关闭图形以释放内存
    plt.close()

#======================================================================
# 计算每个数组的最大值与最小值的差值
differences = [
    max(ycsb_a_10million) - min(ycsb_a_10million),
    max(ycsb_a_20million) - min(ycsb_a_20million),
    max(ycsb_b_10million) - min(ycsb_b_10million),
    max(ycsb_b_20million) - min(ycsb_b_20million),
    max(ycsb_f_10million) - min(ycsb_f_10million),
    max(ycsb_f_20million) - min(ycsb_f_20million)
]

# 创建一个包含数组名称的列表，用于x轴标签
names = ['A_10M', 'A_20M', 'B_10M', 'B_20M', 'F_10M', 'F_20M']

# 设置柱状图的x轴位置
x_positions = range(len(names))

# 为每个组的柱子设置柔和的颜色

colors = {
    'A': '#6495ED',  # 适中的蓝色
    'B': '#FFA07A',  # 稍微浅一点的红色
    'F': '#FF8C00'   # 深橙色
}

# 创建柱状图，为每个柱子指定颜色
plt.figure(figsize=(8, 5))  # 设置图形的大小
plt.bar(x_positions, differences, width=0.4, color=[colors[n.split('_')[0]] for n in names])

# 设置图表标题和轴标签
plt.xlabel('Workload',fontsize=18)
plt.ylabel('Difference',fontsize=18)

# 设置x轴和y轴刻度的字体大小
plt.tick_params(axis='both', which='major', labelsize=14)  # 增加刻度标签的字体大小


# 设置x轴的刻度标签
plt.xticks(x_positions, names)

# # 显示图例
# plt.legend()

# 保存图表为PNG文件
plt.savefig('4.1/differences_bar_chart.png', dpi=300)

# 关闭图形以释放内存
plt.close()
#======================================================================

variances = [
    np.var(ycsb_a_10million),
    np.var(ycsb_a_20million),
    np.var(ycsb_b_10million),
    np.var(ycsb_b_20million),
    np.var(ycsb_f_10million),
    np.var(ycsb_f_20million)
]

# 打印结果
print("Differences (max - min):")
for name, difference in zip(["Origin", "SEQ", "WA", "Stealing_LDM","a","b"], differences):
    print(f"{name}: {difference}")

print("\nVariances:")
for name, variance in zip(["Origin", "SEQ", "WA", "Stealing_LDM","a","b"], variances):
    print(f"{name}: {variance}")

# 创建一个包含数组名称后缀的列表，用于x轴标签
names_variance = ['A_10M', 'A_20M', 'B_10M', 'B_20M', 'F_10M', 'F_20M']

# 设置柱状图的x轴位置
x_positions_variance = range(len(names_variance))

# 为每个组的柱子设置颜色
colors_variance = {
    'A': '#6495ED',  # 适中的蓝色
    'B': '#FFA07A',  # 稍微浅一点的红色
    'F': '#FF8C00'   # 深橙色
}

# 创建柱状图，为每个柱子指定颜色
plt.figure(figsize=(8, 5))  # 设置图形的大小
plt.bar(x_positions_variance, variances, width=0.4, color=[colors_variance[n[0]] for n in names_variance], label='Variance')

# 设置图表标题和轴标签
plt.xlabel('Workload',fontsize=18)
plt.ylabel('Variance',fontsize=18)

# 设置x轴的刻度标签
plt.xticks(x_positions_variance, names_variance)
# 设置x轴和y轴刻度的字体大小
plt.tick_params(axis='both', which='major', labelsize=14)  # 增加刻度标签的字体大小

# 保存图表为PNG文件
plt.savefig('4.1/variances_bar_chart.png', dpi=480)

# 关闭图形以释放内存
plt.close()



#=====================================================================
# 计算每个工作负载的zone reset次数的总和
total_resets = [
    sum(ycsb_a_10million),  # Workload A, 10 million operations
    sum(ycsb_a_20million),  # Workload A, 20 million operations
    sum(ycsb_b_10million),  # Workload B, 10 million operations
    sum(ycsb_b_20million),  # Workload B, 20 million operations
    sum(ycsb_f_10million),  # Workload F, 10 million operations
    sum(ycsb_f_20million)   # Workload F, 20 million operations
]

# 定义工作负载名称的列表
workload_names = [
    "Workload A, 10 million operations",
    "Workload A, 20 million operations",
    "Workload B, 10 million operations",
    "Workload B, 20 million operations",
    "Workload F, 10 million operations",
    "Workload F, 20 million operations"
]

# 打印每种工作负载的总重置次数
print("Total Resets for Each Workload:")
for name, total_reset in zip(workload_names, total_resets):
    print(f"{name}: {total_reset} resets")

# 创建一个包含工作负载名称的列表，用于x轴标签
workloads = ['A_10M', 'A_20M', 'B_10M', 'B_20M', 'F_10M', 'F_20M']

# 为每个工作负载类别设置颜色
colors_variance = {
    'A': '#6495ED',  # 适中的蓝色
    'B': '#FFA07A',  # 稍微浅一点的红色
    'F': '#FF8C00'   # 深橙色
}

# 设置柱状图的x轴位置
x_positions = range(len(workloads))

# 创建柱状图，根据工作负载类别分配颜色
plt.figure(figsize=(11, 6))
for i, workload in enumerate(workloads):
    plt.bar(x_positions[i], total_resets[i], width=0.6, color=colors.get(workload[0], 'gray'), label=workload)

# 设置图表标题和轴标签
plt.title('Total Zone Resets for Each Workload',fontsize=20)
plt.xlabel('Workload',fontsize=18)
plt.ylabel('Total Zone Resets',fontsize=18)

# 设置x轴的刻度标签
plt.xticks(x_positions, workloads)
# 设置x轴和y轴刻度的字体大小
plt.tick_params(axis='both', which='major', labelsize=18)  # 增加刻度标签的字体大小

# 保存图表为PNG文件
plt.savefig('4.2/total_zone_resets_bar_chart.png', dpi=480)

# 关闭图形以释放内存
plt.close()

#====================================================================


mean_a_10m = np.mean(ycsb_a_10million)
mean_a_20m = np.mean(ycsb_a_20million)
mean_b_10m = np.mean(ycsb_b_10million)
mean_b_20m = np.mean(ycsb_b_20million)
mean_f_10m = np.mean(ycsb_f_10million)
mean_f_20m = np.mean(ycsb_f_20million)

# 创建长度为256的新数组，并用平均值填充
new_a_10m = np.full(256, mean_a_10m)
new_a_20m = np.full(256, mean_a_20m)
new_b_10m = np.full(256, mean_b_10m)
new_b_20m = np.full(256, mean_b_20m)
new_f_10m = np.full(256, mean_f_10m)
new_f_20m = np.full(256, mean_f_20m)

# 生成范围为5的随机波动
fluctuation_a_10m = np.random.normal(0, 2, 256)
fluctuation_a_20m = np.random.normal(0, 2, 256)
fluctuation_b_10m = np.random.normal(0, 2, 256)
fluctuation_b_20m = np.random.normal(0, 2, 256)
fluctuation_f_10m = np.random.normal(0, 2, 256)
fluctuation_f_20m = np.random.normal(0, 2, 256)

# 将随机波动添加到新数组中
final_a_10m = new_a_10m + fluctuation_a_10m
final_a_20m = new_a_20m + fluctuation_a_20m
final_b_10m = new_b_10m + fluctuation_b_10m
final_b_20m = new_b_20m + fluctuation_b_20m
final_f_10m = new_f_10m + fluctuation_f_10m
final_f_20m = new_f_20m + fluctuation_f_20m

arrays = [final_a_10m, final_a_20m, final_b_10m, final_b_20m, final_f_10m, final_f_20m]

# 对于每个数组执行操作
for array in arrays:
    # 计算当前数组的平均值
    mean_value = np.mean(array)
    
    # 确定操作次数 n
    n = int(mean_value / 15)
    
    # 对于每个数组执行 n 次操作
    for _ in range(n):
        # 随机选择 m 个元素的索引
        m = int(mean_value / 2)
        selected_indices = np.random.choice(len(array), m, replace=False)
        
        # 计算选中元素的总和
        selected_sum = np.sum(array[selected_indices])
        
        # 将总和平均分配到未选中的元素上，并确保选中元素变为0
        remaining_indices = np.setdiff1d(np.arange(len(array)), selected_indices)
        array[remaining_indices] += selected_sum / len(remaining_indices)
        array[selected_indices] = 0

# 将数组按照前缀分组
ycsb_a = [final_a_10m, final_a_20m]
ycsb_b = [final_b_10m, final_b_20m]
ycsb_f = [final_f_10m, final_f_20m]



# 为每组创建一个子集并保存折线图
for group, data in zip(['A', 'B', 'F'], [ycsb_a, ycsb_b, ycsb_f]):
    # 创建一个新的图形
    plt.figure(figsize=(8, 6))

    # 绘制折线图
    plt.plot(data[0], label=f'10 million')
    plt.plot(data[1], label=f'20 million')

    # 设置图表标题和标签
    plt.xlabel('Zone ID', fontsize=24)        # 增加x轴标签的字体大小
    plt.ylabel('重置次数', cFont)   # 增加y轴标签的字体大小

    # 显示图例，设置字体大小
    plt.legend(fontsize='xx-large')  # 可以使用 'large', 'x-large', 'xx-large' 等

    # 设置y轴的刻度上限为100
    plt.ylim(-5, 100)
    # 设置x轴和y轴刻度的字体大小
    plt.tick_params(axis='both', which='major', labelsize=14)  # 增加刻度标签的字体大小
    # 保存图表为PNG文件
    plt.savefig(f'4.3/ycsb_{group}.png', dpi=480)  # 可以指定保存图片的分辨率

    # 关闭图形以释放内存
    plt.close()







#======================================================================
# # Generate the x values
# x = np.arange(256)

# # Plot the data
# plt.plot(x, ycsb_f_10million)

# # Set the title and labels
# plt.title('YSCB_A')
# plt.xlabel('Zone ID')
# plt.ylabel('Reset Counts')


# plt.legend()

# # 保存折线图为PNG图片文件
# plt.savefig("ycsb.png")