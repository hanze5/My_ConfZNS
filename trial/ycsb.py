# -*- coding: utf-8 -*-
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
eFont1 = {'family': 'Times New Roman',
         'weight': '1',
         'size': 16,
         }

eFont2 = {'family': 'Times New Roman',
         'weight': '1',
         'size': 20,
         }


A_3200_origin = [9, 0, 0, 0, 0, 0, 0, 137, 88, 83, 78, 112, 114, 130, 106, 63, 57, 134, 78, 85, 95, 46, 60, 125, 46, 44, 69, 72, 77, 76, 99, 73, 55, 101, 68, 51, 75, 49, 53, 51, 76, 72, 44, 64, 44, 59, 54, 61, 43, 64, 58, 65, 55, 55, 57, 87, 30, 64, 44, 42, 50, 23, 38, 51, 26, 69, 40, 75, 61, 28, 29, 21, 40, 38, 52, 24, 21, 32, 32, 29, 47, 62, 67, 25, 38, 28, 43, 52, 19, 52, 31, 20, 16, 20, 37, 21, 28, 39, 42, 29, 8, 13, 38, 26, 28, 24, 30, 52, 41, 21, 31, 16, 21, 24, 46, 20, 28, 57, 37, 25, 28, 52, 32, 27, 21, 12, 15, 16, 22, 4, 29, 12, 24, 11, 20, 6, 16, 12, 14, 16, 19, 12, 12, 10, 13, 12, 11, 9, 13, 7, 6, 6, 9, 4, 5, 4, 3, 5, 4, 3, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

A_1600_origin = [3, 0, 0, 0, 0, 0, 0, 83, 86, 88, 70, 100, 97, 72, 45, 63, 76, 76, 92, 46, 65, 73, 59, 66, 59, 47, 74, 57, 56, 54, 41, 29, 57, 59, 58, 46, 49, 59, 33, 64, 18, 43, 17, 41, 31, 45, 42, 46, 37, 49, 20, 25, 27, 42, 36, 55, 51, 45, 52, 47, 63, 6, 50, 34, 35, 42, 10, 33, 27, 17, 31, 44, 26, 43, 38, 26, 42, 13, 28, 33, 24, 13, 26, 15, 17, 33, 17, 16, 9, 37, 14, 36, 37, 16, 28, 17, 19, 10, 8, 16, 22, 22, 10, 24, 11, 21, 14, 33, 20, 22, 21, 8, 13, 7, 10, 8, 7, 16, 9, 12, 13, 22, 9, 16, 13, 17, 8, 11, 7, 3, 10, 8, 11, 10, 4, 9, 11, 3, 9, 2, 3, 4, 2, 2, 1, 3, 3, 0, 0, 1, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

A_800_origin = [3, 0, 0, 0, 0, 0, 0, 89, 91, 51, 53, 71, 44, 68, 38, 58, 54, 38, 29, 29, 43, 32, 37, 32, 19, 23, 43, 29, 39, 28, 43, 23, 38, 28, 33, 34, 32, 37, 29, 21, 21, 27, 16, 29, 36, 30, 20, 31, 24, 25, 27, 15, 14, 34, 24, 36, 15, 12, 14, 19, 25, 12, 19, 16, 22, 15, 14, 24, 11, 19, 11, 17, 12, 19, 10, 26, 16, 0, 4, 12, 10, 5, 9, 6, 17, 8, 7, 8, 4, 7, 6, 1, 4, 2, 3, 1, 6, 2, 7, 4, 2, 5, 0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

A_400_origin =  [3, 0, 0, 0, 0, 0, 0, 39, 41, 29, 32, 29, 43, 17, 25, 33, 23, 25, 32, 31, 32, 27, 21, 13, 7, 9, 15, 16, 16, 23, 16, 9, 9, 12, 14, 10, 13, 17, 3, 12, 5, 22, 8, 13, 22, 5, 22, 9, 5, 7, 7, 8, 14, 8, 6, 5, 4, 6, 8, 0, 6, 4, 7, 5, 7, 2, 5, 2, 0, 4, 2, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

A_origin = np.concatenate([A_3200_origin, A_1600_origin, A_800_origin,A_400_origin])


#创建x轴的值，假设每个数组的索引对应x轴的一个点
x = np.arange(len(A_origin))


B_3200_origin = [9, 0, 0, 0, 0, 0, 0, 135, 70, 140, 54, 68, 59, 87, 84, 54, 82, 72, 72, 102, 127, 54, 84, 30, 38, 57, 89, 90, 95, 64, 145, 73, 42, 99, 130, 112, 67, 51, 52, 59, 117, 52, 124, 112, 37, 77, 38, 85, 87, 36, 50, 44, 46, 95, 24, 89, 56, 66, 75, 10, 95, 33, 58, 9, 74, 90, 43, 77, 51, 83, 35, 12, 80, 10, 56, 73, 35, 99, 41, 68, 45, 30, 75, 49, 38, 52, 87, 37, 50, 42, 30, 44, 66, 74, 50, 27, 50, 13, 38, 93, 26, 58, 67, 44, 31, 46, 39, 50, 21, 30, 59, 34, 95, 23, 42, 45, 26, 12, 50, 18, 68, 18, 39, 22, 31, 59, 33, 29, 56, 5, 54, 61, 75, 75, 44, 46, 53, 46, 60, 25, 55, 55, 46, 48, 54, 51, 19, 32, 30, 37, 60, 58, 26, 48, 34, 51, 32, 57, 36, 40, 52, 50, 38, 28, 36, 49, 41, 41, 35, 35, 36, 47, 42, 32, 50, 30, 34, 35, 32, 15, 22, 23, 29, 25, 32, 39, 20, 19, 27, 30, 33, 19, 38, 19, 33, 23, 34, 18, 29, 25, 23, 26, 29, 32, 12, 14, 11, 14, 17, 8, 13, 13, 7, 10, 16, 7, 13, 5, 8, 7, 13, 8, 2, 8, 6, 2, 2, 5, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
B_1600_origin = [3, 0, 0, 0, 0, 0, 0, 85, 81, 63, 75, 52, 76, 43, 78, 52, 43, 52, 66, 48, 52, 60, 40, 48, 33, 84, 75, 46, 64, 56, 50, 42, 61, 35, 47, 38, 48, 36, 52, 45, 38, 39, 29, 42, 49, 53, 25, 37, 43, 23, 55, 33, 35, 48, 45, 33, 41, 36, 41, 32, 36, 26, 44, 38, 26, 18, 47, 13, 23, 28, 20, 58, 40, 33, 42, 27, 34, 35, 47, 22, 29, 56, 41, 21, 26, 31, 22, 28, 20, 47, 22, 45, 28, 32, 48, 48, 31, 27, 41, 38, 18, 31, 24, 14, 29, 20, 23, 47, 37, 46, 28, 31, 32, 26, 28, 25, 21, 16, 33, 23, 36, 29, 22, 17, 18, 36, 28, 20, 24, 14, 35, 31, 20, 15, 26, 12, 27, 19, 21, 16, 23, 10, 17, 28, 21, 21, 30, 35, 32, 16, 20, 21, 14, 16, 19, 28, 9, 21, 14, 19, 14, 9, 13, 15, 20, 20, 13, 13, 17, 8, 8, 18, 13, 15, 10, 10, 9, 10, 19, 11, 11, 5, 5, 17, 7, 7, 6, 8, 7, 4, 13, 6, 3, 2, 1, 1, 3, 2, 2, 1, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
B_800_origin = [3, 0, 0, 0, 0, 0, 0, 60, 72, 64, 47, 55, 62, 30, 54, 32, 42, 50, 33, 39, 44, 27, 39, 27, 41, 27, 31, 35, 41, 52, 23, 26, 31, 40, 27, 23, 35, 26, 27, 37, 24, 27, 34, 25, 27, 31, 23, 30, 38, 26, 38, 31, 26, 28, 33, 18, 19, 16, 19, 18, 22, 13, 15, 20, 23, 24, 11, 7, 7, 24, 33, 28, 33, 14, 17, 26, 18, 11, 14, 10, 15, 25, 12, 20, 9, 12, 23, 9, 24, 5, 22, 11, 12, 10, 6, 12, 13, 20, 7, 16, 18, 13, 15, 10, 13, 10, 15, 18, 4, 22, 20, 16, 11, 18, 19, 12, 10, 8, 9, 8, 6, 9, 11, 8, 8, 8, 6, 10, 6, 4, 9, 4, 5, 4, 6, 4, 3, 1, 2, 1, 5, 5, 5, 2, 1, 3, 2, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
B_400_origin = [3, 0, 0, 0, 0, 0, 0, 51, 51, 24, 37, 37, 50, 25, 46, 44, 28, 25, 35, 42, 35, 35, 42, 34, 18, 25, 17, 12, 15, 19, 19, 14, 19, 13, 23, 17, 22, 12, 15, 12, 16, 19, 18, 19, 19, 11, 5, 19, 16, 9, 19, 16, 8, 8, 16, 11, 14, 19, 5, 10, 4, 7, 18, 4, 1, 7, 6, 9, 8, 5, 7, 10, 6, 2, 7, 14, 6, 6, 17, 8, 7, 3, 11, 8, 5, 6, 7, 9, 5, 0, 4, 12, 3, 2, 6, 1, 1, 1, 0, 3, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]


B_origin = np.concatenate([B_3200_origin, B_1600_origin, B_800_origin,B_400_origin])




F_3200_origin = [9, 0, 0, 0, 0, 0, 0, 109, 81, 91, 59, 75, 81, 58, 70, 66, 86, 70, 89, 60, 55, 96, 65, 73, 61, 65, 41, 85, 27, 28, 65, 68, 56, 58, 40, 60, 51, 72, 36, 29, 62, 46, 41, 70, 42, 16, 50, 63, 47, 46, 18, 37, 61, 12, 36, 43, 36, 52, 22, 15, 52, 21, 52, 16, 19, 21, 33, 31, 18, 19, 32, 20, 28, 32, 23, 15, 17, 17, 21, 24, 19, 9, 28, 10, 12, 14, 34, 32, 10, 38, 20, 22, 29, 38, 13, 22, 17, 27, 20, 20, 11, 18, 17, 16, 16, 8, 25, 26, 26, 23, 7, 10, 18, 22, 9, 18, 23, 14, 12, 1, 15, 5, 6, 18, 8, 16, 9, 16, 10, 10, 14, 3, 14, 13, 11, 5, 5, 7, 4, 5, 3, 5, 0, 0, 5, 3, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
F_1600_origin = [3, 0, 0, 0, 0, 0, 0, 99, 80, 73, 54, 90, 54, 34, 34, 45, 33, 63, 51, 38, 28, 47, 52, 42, 32, 40, 42, 32, 60, 37, 52, 56, 34, 26, 32, 50, 24, 28, 25, 40, 20, 29, 34, 63, 42, 28, 51, 36, 18, 21, 27, 29, 29, 19, 20, 32, 23, 22, 17, 16, 32, 11, 22, 27, 3, 33, 22, 26, 30, 13, 6, 16, 23, 12, 25, 6, 12, 16, 18, 7, 21, 14, 4, 15, 12, 19, 14, 8, 20, 12, 17, 5, 11, 18, 5, 6, 5, 12, 11, 4, 6, 10, 6, 6, 10, 6, 7, 2, 6, 6, 0, 5, 1, 4, 2, 4, 3, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
F_800_origin = [3, 0, 0, 0, 0, 0, 0, 72, 42, 71, 51, 34, 42, 16, 53, 32, 36, 45, 17, 27, 22, 16, 28, 47, 38, 8, 26, 32, 12, 19, 15, 33, 17, 26, 12, 30, 27, 9, 12, 32, 20, 23, 22, 15, 21, 15, 19, 15, 17, 10, 9, 7, 8, 16, 13, 7, 14, 9, 10, 19, 10, 9, 13, 12, 12, 10, 4, 3, 5, 5, 5, 5, 4, 1, 3, 4, 5, 7, 9, 7, 4, 2, 2, 2, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
F_400_origin = [3, 0, 0, 0, 0, 0, 0, 44, 34, 31, 29, 31, 19, 19, 12, 27, 24, 16, 25, 14, 15, 17, 16, 17, 17, 18, 19, 9, 9, 10, 16, 5, 3, 1, 1, 14, 12, 11, 24, 7, 12, 8, 7, 9, 11, 4, 7, 12, 8, 2, 8, 1, 1, 1, 2, 2, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

F_origin = np.concatenate([F_3200_origin, F_1600_origin, F_800_origin,F_400_origin])

# 定义数据组和对应的qps标签
data_groups = [
    ("A", [A_3200_origin, A_1600_origin, A_800_origin, A_400_origin], ['qps=3200', 'qps=1600', 'qps=800', 'qps=400']),
    ("B", [B_3200_origin, B_1600_origin, B_800_origin, B_400_origin], ['qps=3200', 'qps=1600', 'qps=800', 'qps=400']),
    ("F", [F_3200_origin, F_1600_origin, F_800_origin, F_400_origin], ['qps=3200', 'qps=1600', 'qps=800', 'qps=400'])
]


for i, (group_label, datasets, qps_labels) in enumerate(data_groups):
    plt.figure(figsize=(8, 5))
    
    for j, (data, qps) in enumerate(zip(datasets, qps_labels)):
        plt.plot(x[:len(data)], data, label=f'{group_label} {qps}')
    
    plt.legend(prop={'size': 'large'})
    plt.xlabel('Zone ID', fontproperties=eFont2)
    plt.ylabel('Reset Counts', fontproperties=eFont2)
    
    # 保存每组数据的图表
    plt.savefig(f'pic/ycsb_{group_label.lower()}_origin.png', dpi=720)
    plt.close()  # 关闭当前图形，以便于创建下一张图




A_3200_simple = [29, 0, 0, 0, 0, 0, 0, 29, 29, 29, 29, 24, 11, 29, 23, 10, 28, 26, 29, 29, 29, 29, 28, 27, 27, 25, 28, 29, 29, 29, 29, 28, 28, 29, 21, 29, 26, 28, 23, 29, 28, 29, 29, 29, 28, 21, 23, 22, 29, 27, 29, 23, 29, 29, 29, 28, 28, 29, 28, 26, 29, 18, 27, 20, 23, 28, 24, 28, 22, 28, 28, 29, 19, 29, 29, 29, 29, 24, 28, 29, 25, 27, 14, 29, 26, 28, 26, 25, 26, 28, 28, 23, 24, 27, 29, 24, 27, 23, 29, 28, 29, 27, 25, 20, 29, 23, 29, 25, 27, 29, 28, 28, 27, 28, 27, 28, 29, 23, 29, 29, 28, 27, 28, 25, 29, 28, 28, 25, 28, 26, 29, 29, 23, 29, 29, 28, 27, 23, 19, 27, 28, 22, 24, 23, 28, 28, 28, 27, 28, 28, 28, 28, 23, 27, 28, 28, 27, 27, 23, 28, 26, 28, 28, 8, 28, 28, 21, 19, 19, 28, 20, 28, 27, 24, 19, 23, 27, 28, 9, 28, 25, 28, 20, 28, 28, 27, 28, 28, 26, 28, 28, 23, 28, 27, 23, 28, 27, 28, 28, 28, 27, 27, 28, 20, 23, 26, 25, 28, 20, 28, 26, 28, 27, 26, 9, 28, 28, 28, 28, 28, 28, 23, 28, 22, 27, 28, 28, 25, 28, 25, 28, 28, 28, 23, 27, 28, 28, 28, 27, 25, 27, 25, 27, 28, 28, 27, 27, 28, 26, 28, 28, 23, 28, 28, 28, 28]
A_1600_simple =  [2, 0, 0, 0, 0, 0, 0, 21, 21, 22, 13, 22, 7, 22, 22, 22, 21, 22, 22, 22, 22, 22, 20, 22, 18, 8, 19, 18, 14, 22, 19, 18, 22, 22, 22, 19, 21, 20, 22, 22, 19, 21, 21, 22, 16, 21, 21, 19, 10, 22, 20, 22, 22, 22, 10, 7, 10, 21, 22, 21, 16, 22, 22, 20, 22, 19, 21, 18, 21, 7, 20, 21, 22, 21, 19, 22, 21, 18, 20, 21, 22, 21, 19, 12, 20, 21, 16, 7, 21, 16, 7, 19, 14, 15, 14, 7, 7, 22, 18, 21, 21, 7, 7, 21, 7, 22, 22, 21, 21, 19, 14, 17, 20, 11, 16, 21, 21, 13, 19, 20, 13, 21, 21, 21, 16, 7, 15, 21, 7, 21, 20, 21, 7, 14, 14, 20, 18, 14, 21, 21, 21, 7, 15, 20, 7, 6, 16, 9, 21, 7, 13, 21, 21, 7, 20, 14, 21, 21, 7, 17, 15, 18, 20, 20, 7, 21, 21, 21, 7, 7, 6, 21, 21, 20, 20, 21, 21, 21, 7, 21, 13, 18, 20, 20, 20, 21, 16, 21, 21, 21, 21, 21, 21, 17, 10, 18, 21, 21, 21, 19, 13, 21, 21, 19, 21, 19, 20, 21, 19, 21, 19, 21, 21, 21, 17, 15, 21, 21, 21, 20, 14, 15, 19, 20, 21, 20, 17, 21, 21, 21, 13, 11, 21, 14, 19, 21, 21, 21, 11, 21, 20, 21, 21, 21, 14, 20, 16, 19, 21, 21, 21, 21, 20, 21, 21, 21]
A_800_simple = [0, 0, 0, 0, 0, 0, 11, 7, 10, 10, 11, 9, 10, 10, 9, 10, 9, 10, 10, 10, 8, 9, 10, 10, 10, 10, 10, 4, 8, 9, 10, 3, 10, 3, 10, 7, 10, 8, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 10, 8, 10, 10, 9, 9, 7, 6, 6, 10, 10, 10, 10, 10, 10, 10, 7, 10, 10, 10, 3, 10, 8, 4, 10, 10, 10, 10, 3, 10, 9, 10, 10, 9, 10, 10, 10, 10, 3, 10, 10, 10, 9, 10, 9, 10, 10, 10, 9, 10, 10, 10, 6, 3, 3, 9, 10, 9, 9, 9, 10, 10, 10, 10, 10, 10, 4, 8, 10, 10, 10, 10, 8, 3, 10, 4, 9, 6, 10, 7, 10, 10, 10, 9, 10, 10, 10, 9, 10, 10, 10, 10, 10, 10, 10, 10, 9, 10, 10, 10, 9, 10, 7, 9, 9, 10, 10, 3, 10, 10, 3, 10, 10, 9, 10, 9, 10, 9, 9, 10, 10, 10, 9, 9, 10, 10, 8, 10, 9, 9, 7, 9, 8, 9, 10, 10, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 7, 10, 6, 10, 6, 10, 9, 9, 8, 7, 10, 10, 9, 10, 8, 10, 10, 10, 9, 10, 8, 9, 10, 8, 2, 10, 10, 9, 9, 7, 10, 10, 10, 10, 8, 10, 9, 9, 8, 9, 6, 10, 10, 10, 9, 9, 9, 10, 10, 9, 5, 10, 7, 9, 6, 9]
A_400_simple =  [0, 0, 0, 0, 0, 0, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 3, 4, 4, 4, 4, 4, 3, 4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 3, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 4, 4, 2, 4, 3, 4, 4, 4, 4, 4, 3, 4, 4, 2, 3, 4, 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 3, 3, 4, 3, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 2, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 3, 3, 4, 4, 4, 4, 3, 4, 4, 3, 3, 4, 3, 3, 3, 4, 4, 3, 4, 3, 4, 4, 3, 3, 4, 3, 4, 4, 3, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 3, 3, 3, 3, 4, 4, 3, 3, 3, 3, 4, 4, 4, 4, 3, 3, 3, 4, 3, 4, 4, 3, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3]

A_simple = np.concatenate([A_3200_simple, A_1600_simple, A_800_simple,A_400_simple])


B_3200_simple =  [53, 0, 0, 0, 0, 0, 0, 34, 42, 33, 52, 48, 49, 15, 55, 44, 50, 46, 39, 52, 53, 41, 48, 47, 45, 18, 42, 48, 41, 51, 53, 51, 32, 54, 49, 45, 41, 40, 38, 51, 55, 22, 22, 22, 54, 44, 45, 39, 49, 34, 51, 48, 22, 23, 50, 44, 55, 33, 43, 50, 55, 33, 37, 46, 55, 22, 36, 44, 45, 49, 38, 36, 54, 44, 42, 49, 50, 47, 50, 54, 28, 45, 40, 52, 53, 52, 50, 44, 48, 54, 30, 48, 47, 30, 54, 33, 53, 31, 52, 20, 54, 51, 51, 52, 38, 53, 51, 40, 54, 33, 36, 52, 52, 53, 39, 34, 47, 54, 20, 42, 47, 53, 42, 51, 46, 38, 44, 51, 39, 53, 54, 39, 52, 49, 22, 39, 37, 48, 52, 31, 51, 50, 46, 48, 37, 37, 53, 46, 45, 48, 54, 54, 46, 35, 54, 37, 40, 54, 40, 44, 48, 52, 27, 48, 53, 40, 48, 45, 25, 53, 40, 19, 21, 39, 40, 39, 53, 40, 40, 38, 33, 41, 36, 50, 40, 39, 49, 51, 20, 51, 41, 53, 52, 53, 20, 44, 33, 51, 19, 53, 34, 30, 34, 44, 40, 41, 54, 27, 54, 42, 46, 43, 17, 23, 53, 54, 46, 54, 18, 45, 48, 19, 49, 40, 44, 20, 36, 50, 33, 53, 42, 40, 51, 51, 49, 43, 52, 13, 52, 23, 38, 25, 51, 52, 49, 19, 54, 54, 37, 50, 49, 42, 50, 51, 46, 48]
B_1600_simple =  [2, 0, 0, 0, 0, 0, 0, 22, 25, 27, 23, 24, 20, 27, 26, 20, 27, 21, 24, 26, 27, 19, 25, 27, 27, 27, 27, 23, 22, 25, 27, 26, 23, 27, 10, 20, 23, 27, 19, 24, 23, 26, 26, 27, 26, 18, 27, 21, 27, 26, 22, 26, 22, 25, 23, 21, 27, 26, 27, 27, 23, 26, 21, 27, 25, 24, 12, 27, 27, 27, 27, 27, 23, 23, 27, 21, 23, 26, 23, 20, 25, 26, 27, 18, 25, 15, 26, 26, 23, 26, 26, 25, 24, 27, 17, 25, 23, 20, 23, 23, 25, 19, 27, 23, 26, 24, 27, 17, 26, 27, 26, 27, 27, 27, 20, 9, 23, 27, 26, 26, 23, 22, 14, 25, 18, 19, 27, 23, 20, 27, 27, 25, 26, 21, 21, 21, 27, 21, 25, 25, 22, 26, 26, 26, 26, 24, 20, 18, 27, 20, 22, 23, 26, 27, 27, 26, 26, 24, 27, 21, 25, 26, 25, 26, 27, 27, 22, 21, 26, 25, 23, 27, 19, 27, 24, 27, 26, 21, 27, 25, 18, 27, 26, 18, 21, 21, 27, 22, 25, 23, 18, 27, 20, 26, 21, 21, 18, 24, 26, 25, 26, 26, 22, 26, 22, 18, 25, 19, 26, 10, 24, 26, 26, 23, 26, 24, 25, 24, 25, 20, 21, 25, 26, 21, 26, 21, 26, 24, 25, 21, 25, 26, 26, 26, 25, 26, 21, 26, 23, 25, 26, 19, 20, 11, 19, 26, 26, 26, 15, 26, 23, 25, 25, 23, 26, 26]
B_800_simple =  [0, 0, 0, 0, 0, 0, 14, 13, 14, 12, 13, 14, 5, 12, 13, 14, 14, 11, 5, 14, 14, 14, 5, 5, 5, 14, 14, 14, 14, 14, 14, 14, 6, 14, 14, 14, 14, 14, 5, 5, 12, 5, 14, 13, 14, 13, 13, 14, 14, 10, 13, 13, 13, 11, 5, 8, 13, 14, 14, 10, 11, 14, 13, 13, 14, 9, 13, 14, 13, 12, 14, 12, 14, 14, 14, 14, 13, 13, 14, 13, 14, 14, 13, 11, 12, 13, 11, 10, 14, 14, 14, 14, 12, 14, 14, 14, 14, 13, 14, 14, 14, 13, 14, 14, 13, 13, 10, 11, 14, 12, 14, 11, 11, 12, 10, 13, 14, 11, 14, 14, 9, 11, 11, 14, 11, 14, 13, 14, 13, 9, 12, 13, 13, 4, 14, 14, 12, 11, 11, 14, 14, 14, 13, 9, 14, 11, 9, 13, 14, 14, 13, 9, 13, 14, 14, 14, 12, 14, 14, 13, 11, 12, 13, 11, 14, 12, 14, 14, 13, 12, 14, 12, 12, 12, 14, 10, 13, 9, 13, 13, 13, 14, 10, 9, 12, 14, 5, 7, 12, 14, 13, 14, 13, 14, 14, 14, 4, 13, 13, 13, 14, 11, 12, 6, 4, 12, 14, 10, 6, 14, 10, 10, 9, 10, 11, 4, 14, 13, 12, 12, 13, 13, 10, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 12, 11, 12, 9, 13, 13, 12, 13, 12, 8, 13, 13, 11, 13, 13, 13, 7, 4, 13, 13, 8, 13]
B_400_simple =  [0, 0, 0, 0, 0, 0, 7, 7, 2, 4, 7, 7, 6, 7, 4, 5, 4, 7, 6, 7, 6, 7, 7, 6, 5, 7, 7, 7, 4, 7, 7, 6, 6, 7, 7, 7, 6, 5, 1, 7, 6, 6, 6, 7, 6, 4, 7, 4, 7, 6, 7, 4, 7, 5, 7, 4, 4, 6, 6, 7, 7, 6, 7, 2, 7, 5, 6, 6, 4, 6, 6, 6, 7, 5, 6, 6, 6, 4, 7, 7, 5, 4, 5, 6, 2, 6, 3, 6, 7, 7, 6, 6, 6, 7, 1, 5, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 5, 5, 6, 1, 6, 6, 6, 6, 6, 5, 5, 6, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 5, 6, 5, 6, 6, 1, 5, 4, 6, 6, 6, 6, 6, 5, 6, 6, 6, 4, 5, 6, 6, 6, 6, 5, 6, 4, 6, 6, 5, 6, 5, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 5, 3, 6, 3, 6, 5, 6, 6, 5, 6, 6, 6, 4, 6, 5, 5, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 6, 4, 5, 6, 6, 6, 5, 6, 6, 6, 6, 5, 6, 5, 3, 5, 4, 6, 6, 6, 5, 5, 6, 4, 4, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 4, 6, 5, 4, 6, 6, 5, 4, 5, 6]

B_simple = np.concatenate([B_3200_simple, B_1600_simple, B_800_simple,B_400_simple])



F_3200_simple =  [3, 0, 0, 0, 0, 0, 0, 21, 21, 21, 21, 21, 5, 21, 21, 13, 21, 21, 21, 21, 21, 21, 21, 4, 21, 6, 21, 21, 21, 21, 21, 21, 21, 6, 21, 21, 20, 21, 21, 21, 3, 21, 21, 19, 21, 20, 21, 21, 8, 17, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 2, 20, 21, 20, 21, 21, 21, 21, 21, 21, 20, 21, 21, 21, 20, 21, 21, 21, 5, 21, 20, 21, 6, 2, 21, 20, 21, 21, 21, 21, 21, 6, 21, 20, 21, 20, 21, 21, 9, 21, 21, 4, 20, 21, 10, 20, 6, 5, 21, 19, 3, 21, 20, 21, 20, 21, 2, 20, 21, 21, 21, 21, 21, 21, 20, 20, 20, 6, 21, 20, 21, 20, 4, 3, 20, 20, 21, 5, 20, 5, 6, 21, 20, 21, 20, 20, 21, 19, 20, 20, 20, 20, 21, 20, 20, 20, 20, 20, 4, 20, 20, 20, 20, 20, 20, 2, 20, 20, 19, 20, 20, 3, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 4, 20, 3, 20, 20, 20, 20, 20, 5, 20, 20, 20, 20, 20, 1, 20, 20, 20, 20, 19, 20, 20, 20, 20, 20, 20, 20, 19, 20, 18, 20, 20, 7, 20, 5, 5, 20, 20, 20, 20, 2, 20, 20, 20, 20, 20, 20, 20, 20, 4, 20, 20, 3, 20, 20, 20, 20, 20, 20, 20, 20, 19, 20, 20, 20]
F_1600_simple =  [2, 0, 0, 0, 0, 0, 0, 2, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 3, 13, 13, 13, 4, 13, 13, 13, 13, 13, 13, 13, 13, 2, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 4, 13, 13, 13, 13, 3, 2, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 7, 12, 2, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 8, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 1, 13, 13, 13, 13, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 3, 12, 12, 8, 13, 11, 13, 13, 2, 13, 12, 13, 13, 13, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 3, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 2, 13, 1, 12, 13, 13, 13, 13, 13, 13, 13, 13, 12, 13, 12, 12, 13, 13, 13, 6, 13, 12, 3, 9, 12, 13, 1, 13, 13, 13, 13, 13, 2, 13, 13, 12, 12, 12, 13, 13, 13, 13, 12, 13, 13, 12, 12, 12, 1, 13, 13, 12, 12, 13, 13, 2, 13, 12, 0, 13, 3, 13, 12, 13, 12, 12, 12, 12, 13, 11, 12, 12, 13, 12, 2, 12, 12, 3, 12, 0, 12, 12]
F_800_simple =  [0, 0, 0, 0, 0, 0, 7, 7, 6, 7, 7, 7, 6, 6, 7, 2, 7, 7, 7, 7, 7, 7, 7, 7, 1, 7, 7, 6, 5, 7, 7, 7, 5, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 2, 7, 7, 7, 7, 7, 1, 7, 6, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 7, 6, 2, 7, 7, 6, 7, 1, 7, 6, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 7, 7, 6, 7, 7, 6, 6, 7, 7, 7, 1, 6, 6, 7, 6, 6, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 7, 6, 6, 6, 7, 7, 6, 6, 7, 7, 6, 6, 6, 6, 1, 6, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 1, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 1, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 6]
F_400_simple =  [0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 2, 2, 3, 2, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 2, 3, 3, 3, 3, 1, 3, 2, 3, 2, 3, 3, 3, 3, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 2, 3, 3, 3, 3, 2, 3, 2, 3, 2, 3, 3, 3, 3, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 2, 2, 3, 3, 2, 2, 3, 3, 2, 1, 1, 2, 3, 1, 3, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2]

F_simple = np.concatenate([F_3200_simple, F_1600_simple, F_800_simple,F_400_simple])

# 定义数据组和对应的qps标签
data_groups = [
    ("A", [A_3200_simple, A_1600_simple, A_800_simple, A_400_simple], ['qps=3200', 'qps=1600', 'qps=800', 'qps=400']),
    ("B", [B_3200_simple, B_1600_simple, B_800_simple, B_400_simple], ['qps=3200', 'qps=1600', 'qps=800', 'qps=400']),
    ("F", [F_3200_simple, F_1600_simple, F_800_simple, F_400_simple], ['qps=3200', 'qps=1600', 'qps=800', 'qps=400'])
]

for i, (group_label, datasets, qps_labels) in enumerate(data_groups):
    plt.figure(figsize=(8, 5))
    
    for j, (data, qps) in enumerate(zip(datasets, qps_labels)):
        plt.plot(x[:len(data)], data, label=f'{group_label} {qps}')
    
    plt.legend(prop={'size': 'large'})
    plt.xlabel('Zone ID', fontproperties=eFont2)
    plt.ylabel('Reset Counts', fontproperties=eFont2)
    
    # 保存每组数据的图表
    plt.savefig(f'pic/ycsb_{group_label.lower()}_simple.png', dpi=720)
    plt.close()  # 关闭当前图形，以便于创建下一张图



data_groups = {
    'A': {'origin': A_origin, 'simple': A_simple},
    'B': {'origin': B_origin, 'simple': B_simple},
    'F': {'origin': F_origin, 'simple': F_simple}
}

# 遍历数据组，为每组数据绘制一个完整的图
for group_label, group_data in data_groups.items():
    plt.figure(figsize=(10, 5))  # 创建一个新的图形
    plt.plot(group_data['origin'], label=f'{group_label} Origin')
    plt.plot(group_data['simple'], label=f'{group_label} Simple')
    
    plt.xlabel('Blockgroup ID', fontproperties=eFont2)  # 设置x轴标签
    plt.ylabel('Erase Counts', fontproperties=eFont2)  # 设置y轴标签
    plt.legend(fontsize='large')  # 设置图例的字体大小
    
    # 保存图表为PNG文件
    plt.savefig(f'pic/ycsb_{group_label.lower()}.png', dpi=720)
    
    # 关闭当前图形，以便于创建下一张图
    plt.close()

# 遍历data_groups字典，并输出每个数组的统计值
for group_label, arrays in data_groups.items():
    for array_label, array in arrays.items():
        # 计算总和
        sum_val = np.sum(array)
        
        # 计算最大值
        max_val = np.max(array)
        
        # 计算最小值
        min_val = np.min(array)
        
        # 计算最大最小值之差
        range_val = max_val - min_val
        
        # 计算方差
        variance_val = np.var(array)
        
        # 计算标准差
        std_val = np.sqrt(variance_val)
        
        # 打印结果
        print(f"Group: {group_label}, Array: {array_label}")
        print("总和:", sum_val)
        print("最大值:", max_val)
        print("最大最小值之差:", range_val)
        print("方差:", variance_val)
        print("标准差:", std_val)
        print("-" * 40)  # 打印分隔线