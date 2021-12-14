#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np

# csvRd = csv.reader(open('result/maxLevel-ER/AreaVsLevelUnderER5e-2.csv', encoding='utf-8'))
csvRd = csv.reader(open('result/maxLevel-NMED/AreaVsLevelUnderNMED153e-5.csv', encoding='utf-8'))
# cktList = ['c880', 'c1908', 'c2670', 'RCA32', 'CLA32', 'KSA32']
# cktList = ['CLA32', 'KSA32', 'MUL8', 'RCA32', 'WTM8']
cktList = ['RCA32', 'CLA32', 'KSA32', 'MUL8', 'WTM8']
arList = np.zeros([4, len(cktList)])
# timeList = np.zeros([len(cktList), 4])
timeList = np.zeros([4, len(cktList)])
mk = ['o', 'v', 'h', 'x', 's', 'd', 'o', '^', '<', '>', 'h', 'p', 'x']
colors = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:gray', 'tab:olive', 'tab:cyan']
# colors = ['#E41A1C', '#377EB8', '#4DAF4A', '#984EA3', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:gray', 'tab:olive', 'tab:cyan']

plt.rcParams['figure.figsize'] = (6.0, 3.5)
fig, (ax1, ax2) = plt.subplots(1, 2, sharey=True)

for row in csvRd:
    if row[0] == 'circuit' or row[0] == '':
        continue
    if row[8] == '':
        continue
    if row[0] not in cktList:
        continue
    print(row[0] + ',' + row[8] + ',' + row[16] + ',' + row[24] + ',' + row[32])
    pos = cktList.index(row[0])
    arList[0][pos] = float(row[8])
    arList[1][pos] = float(row[16])
    arList[2][pos] = float(row[24])
    arList[3][pos] = float(row[32])
    timeList[0][pos] = float(row[6])
    timeList[1][pos] = float(row[14])
    timeList[2][pos] = float(row[22])
    timeList[3][pos] = float(row[30])
print(arList)
baseTime = np.zeros(len(cktList))
for i in range(len(cktList)):
    baseTime[i] = timeList[3][i]
for i in range(4):
    for j in range(len(cktList)):
        timeList[i][j] = timeList[i][j] / baseTime[j]
print(timeList)


num_list0 = -timeList[0]
num_list1 = -timeList[1]
num_list2 = -timeList[2]
num_list3 = -timeList[3]
x = list(range(len(num_list0)))
total_width, n = 1.0, 5
width = total_width / n
ftSize = 11
alpha = 1.0
ax1.barh(x, num_list0, height=width, alpha=alpha, color=colors[0])
for i in range(len(x)):
    x[i] = x[i] + width
ax1.barh(x, num_list1, height=width, alpha=alpha, tick_label=cktList, color=colors[1])
for i in range(len(x)):
    x[i] = x[i] + width
ax1.barh(x, num_list2, height=width, alpha=alpha, color=colors[2])
for i in range(len(x)):
    x[i] = x[i] + width
ax1.barh(x, num_list3, height=width, alpha=alpha, color=colors[3])
print(x, baseTime)
for i in range(5):
    ax1.text(num_list3[i] - 0.04, x[i] - 0.05, '%ds' % baseTime[i], fontsize=ftSize)
    print(x[i], num_list3[i])
ax1.set_xlabel('Normalized runtime', fontsize=ftSize)
ax1.set_xlim(-1.05, -0.85)
ax1.tick_params(axis='x', labelsize=ftSize)
ax1.tick_params(axis='y', labelsize=ftSize)
ax1.set_xticks((-1.0, -0.95, -0.90, -0.85), (1.0, 0.95, 0.90, 0.85))
# plt.yticks(fontsize=ftSize)
# # ax1.legend(fontsize=ftSize, title='area ratio', title_fontsize=ftSize, bbox_to_anchor=(0.8, 1), loc='upper right')
# # plt.legend(fontsize=11, title='area ratio', title_fontsize=11, loc='upper left', bbox_to_anchor=(0.0, 0.74))
# # plt.legend(fontsize=ftSize)

num_list0 = arList[0]
num_list1 = arList[1]
num_list2 = arList[2]
num_list3 = arList[3]
x = list(range(len(num_list0)))
total_width, n = 1.0, 5
width = total_width / n
ax2.barh(x, num_list0, height=width, label='$l=1$', alpha=alpha, color=colors[0])
for i in range(len(x)):
    x[i] = x[i] + width
ax2.barh(x, num_list1, height=width, label='$l=2$', alpha=alpha, tick_label=cktList, color=colors[1])
for i in range(len(x)):
    x[i] = x[i] + width
ax2.barh(x, num_list2, height=width, label='$l=4$', alpha=alpha, color=colors[2])
for i in range(len(x)):
    x[i] = x[i] + width
ax2.barh(x, num_list3, height=width, label='$l=\infty$', alpha=alpha, color=colors[3])
ax2.set_xlabel('Area ratio', fontsize=ftSize)
ax2.set_xlim(0.28, 0.92)
ax2.tick_params(axis='x', labelsize=ftSize)
ax2.tick_params(axis='y', labelsize=ftSize)
# ax2.legend(fontsize=ftSize, title='area ratio', title_fontsize=ftSize, bbox_to_anchor=(0.8, 1), loc='upper right')
# plt.legend(fontsize=11, title='area ratio', title_fontsize=11, loc='upper left', bbox_to_anchor=(0.0, 0.74))
ax2.legend(fontsize=ftSize)


ax1.spines['right'].set_visible(False)#关闭子图1中底部脊
ax2.spines['left'].set_visible(False)##关闭子图2中顶部脊

plt.tight_layout()
# plt.show()
plt.savefig('result/figures/differentLevelNMED.eps', dpi=1000, format='eps', bbox_inches='tight')
