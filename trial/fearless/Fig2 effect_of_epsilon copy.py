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
xFont = {'family': 'Simsun',
         'weight': '1',
         'size': 30,
         }
# 连线标星大小
startSize = 20
tickSize = 30
markes = ['-x', '-s', '-^', '-p']

'''PT-CPU-TIME'''
fig1 = plt.gcf()
nums = [10, 15, 20, 25, 30]
# eta_time = [900, 884, 855, 896, 913]
gpa_time = [1301, 1411, 1697, 1782, 1996]
AA_time = [210, 256, 272, 304, 346]
#plt.plot(nums, eta_time, markes[0], label='SBT', ms=startSize, color='Green')
plt.plot(nums, gpa_time, markes[1], label='LCRS', ms=startSize,color = '#004e66')
# plt.plot(nums, AA_time, markes[2], label='SBN', ms=startSize,color= '#ff5f2e')
plt.xlabel('相似度阈值 ' + u'\u03B8', xFont)
plt.ylabel("运行时间 (ms)",  yFont)
plt.xticks(nums, ['10', '15', '20', '25', '30'])
plt.yticks([0, 1000, 2000, 3000], ['0', '1k', '2k', '3k'])
plt.tick_params(labelsize=tickSize)
plt.legend(prop=legendFont, ncol=3, handlelength=0, frameon=False, loc=2)
plt.show()
fig1.savefig(r'imgs/PT-ep-time.png', dpi=300, bbox_inches='tight')

patterns = ["/", "\\\\", "|", "-", "+", "xxx", "o", "O", ".", "*"]
# 输入统计数据
periods = ('10', '15', '20', '25', '30')
eta_IC = [5555, 8022, 8894, 9125, 9187]
gpa_IC = [7.5660, 8.3055, 9.0427, 9.7543, 10.5433]
AA_IC = [7.5660, 8.3055, 9.0427, 9.7543, 10.5433]
fig1 = plt.gcf()
bar_width = 0.25  # 条形宽度
index1 = np.arange(len(periods))
index2 = index1 + bar_width
# index3 = index2 + bar_width
# 使用两次 bar 函数画出两组条形图
# plt.bar(index1, height=eta_IC, width=bar_width, edgecolor='black',
#         color='white', label='SBT', hatch=patterns[3])
plt.bar(index1, height=gpa_IC, width=bar_width, edgecolor='black',
        color='white', label='SBT-PC', hatch=patterns[1])
plt.bar(index2, height=AA_IC, width=bar_width,
        edgecolor='black', color='white', label='SBN',  hatch=patterns[5])
# 让横坐标轴刻度显示 periods 里的饮用水， index_male + bar_width/2 为横坐标轴刻度的位置
plt.xticks(index1 + bar_width/2, periods)
plt.xlabel('距离阈值 ' + u'\u03B5', xFont)
plt.ylabel('|CE| (10^4)', yFont)  # 纵坐标轴标题
plt.tick_params(labelsize=tickSize)
plt.yticks([0, 3, 6, 9, 12], ['0', '3', '6', '9', '12'])
# plt.legend(prop=legendFont_bar, handlelength=0, frameon=False, loc=2)
plt.legend(prop=legendFont_bar, frameon=False,  handlelength=1, loc=2, ncol=1)
plt.show()
fig1.savefig(r'imgs/PT-ep-CE.png', dpi=300, bbox_inches='tight')


'''ny'''
fig1 = plt.gcf()
nums = [2, 4, 6, 8, 10]
eta_time = [86, 84, 75, 83, 69]
gpa_time = [176, 195, 226, 237, 251]
AA_time = [75, 75, 76, 81, 84]
# plt.plot(nums, eta_time, markes[0], label='ET', ms=startSize)
plt.plot(nums, gpa_time, markes[1], label='SBT-PC', ms=startSize,color = '#004e66')
plt.plot(nums, AA_time, markes[2], label='SBN', ms=startSize,color= '#ff5f2e')
plt.xlabel('距离阈值 ' + u'\u03B5', xFont)
plt.ylabel("运行时间 (ms)",  yFont)
plt.xticks(nums, ['20', '25', '30', '35', '40'])
plt.yticks([0, 100, 200, 300, 400], ['0', '100', '200', '300', '400'])
plt.tick_params(labelsize=tickSize)
plt.legend(prop=legendFont, ncol=3, handlelength=0, frameon=False, loc=2)
plt.show()
fig1.savefig(r'imgs/BJ-ep-time.png', dpi=300, bbox_inches='tight')

# patterns = ["/", "\\\\", "|", "-", "+", "xxx", "o", "O", ".", "*"]
# # 输入统计数据
# periods = ('20', '25', '30', '35', '40')
# eta_IC = [85, 127, 167, 222, 262]
# eta_IC_ = [85, 127, 167, 222, 62]
# gpa_IC = [1.1888, 1.4436, 1.6822, 1.9044, 2.1107]
# AA_IC = [1.1888, 1.4436, 1.6822, 1.9044, 2.1107]
# fig1 = plt.gcf()
# bar_width = 0.25  # 条形宽度
# index1 = np.arange(len(periods))
# index2 = index1 + bar_width
# # index3 = index2 + bar_width
# # 使用两次 bar 函数画出两组条形图
# plt.bar(index1, height=gpa_IC, width=bar_width, edgecolor='black',
#         color='white', label='SBT-PC', hatch=patterns[1])
# plt.bar(index2, height=AA_IC, width=bar_width,
#         edgecolor='black', color='white', label='SBN',  hatch=patterns[5])
# # 让横坐标轴刻度显示 periods 里的饮用水， index_male + bar_width/2 为横坐标轴刻度的位置
# plt.xticks(index1 + bar_width/2, periods)
# plt.xlabel('distance threshold ' + u'\u03B5', xFont)
# plt.ylabel('|CE| (10^4)', yFont)  # 纵坐标轴标题
# plt.tick_params(labelsize=tickSize)
# plt.yticks([0, 0.5, 1.0, 1.5, 2.0], ['0', '0.5', '1.0', '1.5', '2.0'])
# # plt.legend(prop=legendFont_bar, handlelength=0, frameon=False, loc=2)
# plt.legend(prop=legendFont_bar, frameon=False,  handlelength=1, loc=2, ncol=1)
# plt.show()
# fig1.savefig(r'imgs/NY-ep-CE.png', dpi=300, bbox_inches='tight')
