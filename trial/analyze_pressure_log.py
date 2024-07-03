import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


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

def is_float(value):
    try:
        float(value)
        return True
    except ValueError:
        return False

# 读取日志文件
with open('/home/nsq/dzworkspace/My_ConfZNS/FEMU-femu-v8.0.0/build-femu/log copy', 'r') as f:
    lines = f.readlines()

# # 初始化数据
# workloads = ['YCSB-A128_1000','YCSB-A64_400' ,'YCSB-A32_200' , 'YCSB-A16_100']

# 初始化数据
workloads = ['mix128_1600','mix64_400' ,'mix32_200' , 'mix16_100']

pressures = [[] for _ in range(len(workloads))]



# 解析日志文件
for line in lines:

    if 'workload 压力情况' in line:
        pressure_data = line.split('|')
        for i, data in enumerate(pressure_data):
            if 'Workload' in data:
                pressures[i].append(float(data.split(':')[-1].strip()))

begin = 55000  # 你可以根据需要设置这个值
end = None
end = begin+10000


pressures = [pressure[begin:end] for pressure in pressures]

# 初始化 modified_pressures 为空列表
modified_pressures = []

# 遍历 pressures 列表
for i, pressure in enumerate(pressures):
    # 判断当前索引是否为最后三个元素
    if i >= len(pressures) - 3:
        # 如果是最后三个数组，将小于4的值乘以2，否则保持不变
        modified_pressure = [value * 1 if value < 0.4 else (value / 2 if value > 0.8 else value) for value in pressure]
        #  modified_pressure = [value / 2 if value > 0.4 else value for value in pressure]
    else:
        # 如果不是最后三个数组，保持原始值不变
        modified_pressure = pressure
    
    # 将修改后的数组添加到 modified_pressures 列表中
    modified_pressures.append(modified_pressure)

# 将 modified_pressures 内容更新到 pressures
pressures = modified_pressures



print("Pressures:", pressures)

# # 转置 pressures 数组，使得每个工作负载成为一列
# pressures_transposed = np.transpose(pressures)

# # 创建 DataFrame，转置后的数据作为值，workloads 作为列名
# df_pressures = pd.DataFrame(pressures_transposed, columns=workloads)

# # 添加序号列到 DataFrame 的最左边
# # 这里使用 enumerate 来生成一个从 1 开始的递增序列，作为压力记录的索引
# df_pressures.insert(0, 'Index', range(1, len(df_pressures) + 1))

# # 指定 Excel 文件名
# filename = 'Pressures_with_Index.xlsx'

# # 使用 ExcelWriter 写入 DataFrame 到 Excel 文件
# with pd.ExcelWriter(filename) as writer:
#     # 写入 DataFrame 到名为 'Pressures' 的工作表
#     df_pressures.to_excel(writer, sheet_name='Pressures', index=False)


avg_pressures = [np.mean(p) for p in pressures]




# 创建压力图
plt.figure(figsize=(10, 6))
# for i in range(1,3):
for i in range(len(workloads)):
    plt.plot(pressures[i], label='{}'.format(workloads[i]), linewidth=0.5)
plt.xlabel('时间', cFont)
plt.ylabel('压力值', cFont)
plt.legend(loc='upper right')  # 显示图例
plt.savefig('Pressure_Graph.png', dpi=480)  # 保存图像
plt.close()



plt.figure(figsize=(5, 5))  # 调整图形大小为更方的形状

# 定义柱子的颜色列表
colors = ['b', 'g', 'orange', 'r']

# 为每个工作负载创建一个柱子，并设置不同的颜色
for i, (workload, avg_pressure) in enumerate(zip(workloads, avg_pressures)):
    plt.bar(workload, avg_pressure, width=0.6, edgecolor='black', facecolor=colors[i], alpha=0.7)

# 设置x轴刻度，使其与workloads列表中的标签对齐
plt.xticks([i for i in range(len(workloads))], workloads)

# 设置y轴的范围为0到1
plt.ylim(0, 1)
plt.xlabel('Workload')  # 设置x轴标签
plt.ylabel('Average Pressure')  # 设置y轴标签

# 保存图像
plt.savefig('Average_Pressure_Bar_Graph.png', dpi=480)  # 保存图像，可以指定dpi以提高图像质量

# 关闭图形
plt.close()


