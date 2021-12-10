#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np

# csvRd = csv.reader(open('result/maxLevel-ER/AreaVsLevelUnderER5e-2.csv', encoding='utf-8'))
csvRd = csv.reader(open('result/maxLevel-NMED/AreaVsLevelUnderNMED153e-5.csv', encoding='utf-8'))
# cktList = ['c880', 'c1908', 'c2670', 'RCA32', 'CLA32', 'KSA32']
cktList = ['CLA32', 'KSA32', 'MUL8', 'RCA32', 'WAL8']
arList = np.zeros([4, len(cktList)])
timeList = np.zeros([len(cktList), 4])
mk = ['o', 'v', 'h', 'x', 's', 'd', 'o', '^', '<', '>', 'h', 'p', 'x']

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
    timeList[pos][0] = float(row[6])
    timeList[pos][1] = float(row[14])
    timeList[pos][2] = float(row[22])
    timeList[pos][3] = float(row[30])
print(arList)
for i in range(len(cktList)):
    timeList[i] /= timeList[i][3]
print(timeList)

name_list = cktList
num_list0 = arList[0]
num_list1 = arList[1]
num_list2 = arList[2]
num_list3 = arList[3]
x = list(range(len(num_list0)))
total_width, n = 1.0, 5
width = total_width / n
ftSize = 13
# plt.style.use('ggplot')
colors = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:gray', 'tab:olive', 'tab:cyan']

fig, ax1 = plt.subplots()
alpha = 0.7
ax1.bar(x, num_list0, width=width, label='$l=1$', alpha=alpha)
for i in range(len(x)):
    x[i] = x[i] + width
ax1.bar(x, num_list1, width=width, label='$l=2$', alpha=alpha, tick_label=cktList)
for i in range(len(x)):
    x[i] = x[i] + width
ax1.bar(x, num_list2, width=width, label='$l=4$', alpha=alpha)
for i in range(len(x)):
    x[i] = x[i] + width
ax1.bar(x, num_list3, width=width, label='$l=\infty$', alpha=alpha)
for i in range(len(x)):
    x[i] = x[i] + width
ax1.set_ylabel('Area ratio', fontsize=ftSize)
ax1.set_ylim(0.3, 1.01)
plt.xticks(fontsize=ftSize)
plt.yticks(fontsize=ftSize)
# ax1.legend(fontsize=ftSize, title='area ratio', title_fontsize=ftSize, bbox_to_anchor=(0.8, 1), loc='upper right')
ax1.legend(fontsize=11, title='area ratio', title_fontsize=10, loc='upper left', bbox_to_anchor=(0.0, 0.9))
plt.tight_layout()


ax2 = ax1.twinx()
ax2.set_ylabel('Normalized runtime', fontsize=ftSize)
plt.xticks(fontsize=ftSize)
plt.yticks(fontsize=ftSize)
for i in range(len(cktList)):
    x = np.array(list(range(4))) * width + i
    ax2.plot(x, timeList[i], label=cktList[i], color=colors[i + 1], marker=mk[i])
ax2.legend(fontsize=11, title='norm. runtime', title_fontsize=10, loc='lower left', bbox_to_anchor=(0.0, 0.2))
ax2.set_ylim(0.3, 1.01)
plt.tight_layout()
plt.savefig('result/figures/differentLevelNMED.eps', dpi=1000, format='eps', bbox_inches='tight')
# plt.show()
