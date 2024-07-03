import matplotlib.pyplot as plt
import numpy as np

#
legendFont = {'family': 'Times New Roman',
              'weight': '1',
              'size': 30,
              }
legendFont_bar = {'family': 'Times New Roman',
                  'weight': '1',
                  'size': 22,
                  }
#
legendFont = {'family': 'Times New Roman',
              'weight': '1',
              'size': 22,
              }
yFont = {'family': 'Simsun',
         'weight': '1',
         'size': 24,
         }
xFont = {'family': 'Times New Roman',
         'weight': '1',
         'size': 30,
         }
# 连线标星大小
startSize = 15
tickSize = 20
markes = ['-x', '-s', '-^', '-p', '-D']

'''Beijing'''
# fig1 = plt.gcf()
# nums = [30, 60, 90, 120, 150]
# eta_time = [2209, 5467, 13756, 33985, 78792]
# gpa_time = [317, 803, 1782, 7225, 24828]
# AA_time = [63, 132, 272, 559, 1113]
# value_to_add = 300
# eta_time_updated = [x + value_to_add for x in eta_time]
# gpa_time_updated = [x + value_to_add for x in gpa_time]
# AA_time_updated = [x + value_to_add for x in AA_time]


# plt.plot(nums, eta_time, '-d',
#          label='SBT', ms=startSize, color='#66bfbf',linewidth=2)
# plt.plot(nums, gpa_time, markes[1],
#          label='SBT-PC', ms=startSize,color='#fcbe32',linewidth=2 )
# plt.plot(nums, AA_time, markes[2], label='SBN', ms=startSize,color='#ff5f2e',linewidth=2)
# plt.xlabel("|T| (s)", xFont)
# plt.ylabel("运行时间 (10^3 ms)",  yFont)
# plt.xticks(nums, ['30', '60', '120', '240', '480'])
# plt.yticks([0, 20000, 40000, 60000, 80000], ['0', '20', '40', '60', '80'])
# plt.tick_params(labelsize=tickSize)
# plt.legend(prop=legendFont, ncol=3, handlelength=0, frameon=False, loc=2)
# plt.show()
# fig1.savefig(r'imgs/PT-T-time.png', dpi=300, bbox_inches='tight')

patterns = ["/", "\\\\", "|", "-", "+", "xxx", "o", "O", ".", "*"]
# 输入统计数据
periods = ('30', '60', '120', '240', '480')
eta_IC = [2.2596, 4.5191, 9.0427, 18.1027, 36.2676]
gpa_IC = [2.2596, 4.5191, 9.0427, 18.1027, 36.2676]
AA_IC = [2.2596, 4.5191, 9.0427, 18.1027, 36.2676]

gpa_IC = [2.2596, 4.5191, 9.0427, 18.1027, 36.2676]
AA_IC = [2.2596, 4.5191, 9.0427, 18.1027, 36.2676]
fig1 = plt.gcf()
bar_width = 0.25  # 条形宽度
index1 = np.arange(len(periods))
index2 = index1 + bar_width
# index3 = index2 + bar_width
# 使用两次 bar 函数画出两组条形图
plt.bar(index1, height=eta_IC, width=bar_width, edgecolor='black',
        color='#66bfbf', label='SBT', hatch=patterns[3])
plt.bar(index2, height=gpa_IC, width=bar_width, edgecolor='black',
        color='#fcbe32', label='SBT-PC', hatch=patterns[1])
# plt.bar(index3, height=AA_IC, width=bar_width,
#         edgecolor='black', color='#ff5f2e', label='SBN',  hatch=patterns[5])
# 让横坐标轴刻度显示 periods 里的饮用水， index_male + bar_width/2 为横坐标轴刻度的位置
plt.xticks(index1 + bar_width/2, periods)
plt.yticks([0, 10, 20, 30, 40], ['0', '10', '20', '30', '40'])
plt.xlabel("|T| (s)", xFont)
plt.ylabel('|CE| (10^4)', xFont)  # 纵坐标轴标题
plt.tick_params(labelsize=tickSize)
# plt.legend(prop=legendFont_bar, handlelength=0, frameon=False, loc=2)
plt.legend(prop=legendFont_bar, frameon=False,  handlelength=1, loc=2, ncol=1)

plt.show()
fig1.savefig(r'imgs/PT-Y-CE.png', dpi=300, bbox_inches='tight')


# '''BJ'''
# fig1 = plt.gcf()
# nums = [30, 60, 90, 120, 150]
# eta_time = [948, 2112, 5858, 13492, 29276]
# gpa_time = [444, 890, 1556, 3348, 9057]
# AA_time = [318, 338, 376, 454, 416]
# plt.plot(nums, eta_time, '-d',
#          label='SBT', ms=startSize, color='#66bfbf',linewidth=2)
# plt.plot(nums, gpa_time, markes[1],
#          label='SBT-PC', ms=startSize,color='#fcbe32',linewidth=2 )
# plt.plot(nums, AA_time, markes[2], label='SBN', ms=startSize,color='#ff5f2e',linewidth=2)
# plt.xlabel("|T| (s)", xFont)
# plt.ylabel("运行时间 (10^3 ms)",  yFont)
# plt.xticks(nums, ['30', '60', '120', '240', '480'])
# plt.yticks([0, 10000, 20000, 30000, 40000], ['0', '10', '20', '30', '40'])
# plt.tick_params(labelsize=tickSize)
# plt.legend(prop=legendFont, ncol=3, handlelength=0, frameon=False, loc=2)
# plt.show()
# fig1.savefig(r'imgs/BJ-T-time.png', dpi=300, bbox_inches='tight')

patterns = ["/", "\\\\", "|", "-", "+", "xxx", "o", "O", ".", "*"]
# 输入统计数据
periods = ('30', '60', '120', '240', '480')
eta_IC = [0.2221, 0.4535, 1.0822, 2.2475, 3.6605]
gpa_IC = [0.2221, 0.4535, 1.0822, 2.2475, 3.6605]
AA_IC = [0.2221, 0.4535, 1.0822, 2.2475, 3.6605]
fig1 = plt.gcf()
bar_width = 0.25  # 条形宽度
index1 = np.arange(len(periods))
index2 = index1 + bar_width

# 使用两次 bar 函数画出两组条形图
plt.bar(index1, height=eta_IC, width=bar_width, edgecolor='black',
        color='#66bfbf', label='SBT', hatch=patterns[3])
plt.bar(index2, height=gpa_IC, width=bar_width, edgecolor='black',
        color='#fcbe32', label='SBT-PC', hatch=patterns[1])
# plt.bar(index3, height=AA_IC, width=bar_width,
#         edgecolor='black', color='#ff5f2e', label='SBN',  hatch=patterns[5])
# 让横坐标轴刻度显示 periods 里的饮用水， index_male + bar_width/2 为横坐标轴刻度的位置
plt.xticks(index1 + bar_width/2, periods)
plt.yticks([0, 1,2,3,4], ['0', '1', '2', '3', '4'])
plt.xlabel("|T| (s)", xFont)
plt.ylabel('|CE| (10^4)', xFont)  # 纵坐标轴标题
plt.tick_params(labelsize=tickSize)
# plt.legend(prop=legendFont_bar, handlelength=0, frameon=False, loc=2)
plt.legend(prop=legendFont, ncol=1, handlelength=1, frameon=False, loc=2)
plt.show()
fig1.savefig(r'imgs/BJ-y-CE.png', dpi=300, bbox_inches='tight')
