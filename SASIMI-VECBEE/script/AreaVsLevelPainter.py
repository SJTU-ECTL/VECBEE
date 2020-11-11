#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import matplotlib.pyplot as plt
import numpy as np

# csvRd = csv.reader(open('result/maxLevel-ER/AreaVsLevelUnderER5e-2.csv', encoding='utf-8'))
csvRd = csv.reader(open('result/maxLevel-NMED/AreaVsLevelUnderNMED153e-5.csv', encoding='utf-8'))
# cktList = ['c880', 'c1908', 'c2670', 'RCA32', 'CLA32', 'KSA32']
cktList = ['CLA32', 'KSA32', 'MUL8', 'RCA32', 'WAL8']
arList = np.zeros([4, len(cktList)])

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

print(arList)

name_list = cktList
num_list0 = arList[0]
num_list1 = arList[1]
num_list2 = arList[2]
num_list3 = arList[3]
x = list(range(len(num_list0)))
total_width, n = 1.0, 5
width = total_width / n

ftSize = 13
plt.bar(x, num_list0, width=width, label='$l=1$')
for i in range(len(x)):
    x[i] = x[i] + width
plt.bar(x, num_list1, width=width, label='$l=2$', tick_label=cktList)
for i in range(len(x)):
    x[i] = x[i] + width
plt.bar(x, num_list2, width=width, label='$l=4$')
for i in range(len(x)):
    x[i] = x[i] + width
plt.bar(x, num_list3, width=width, label='$l=\infty$')
for i in range(len(x)):
    x[i] = x[i] + width
plt.xticks(fontsize=ftSize)
plt.yticks(fontsize=ftSize)
plt.ylabel('Area ratio', fontsize=ftSize)
plt.legend(loc='upper right', bbox_to_anchor=(1.25, 1), fontsize=ftSize)
# plt.show()
# plt.savefig('result/figures/differentLevelER.png', dpi=300, bbox_inches='tight')
plt.savefig('result/figures/differentLevelNMED.png', dpi=300, bbox_inches='tight')
