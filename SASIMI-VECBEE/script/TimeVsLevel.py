#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import matplotlib.pyplot as plt


csvRd = csv.reader(open('result/compare-time/ERTimeRaw.csv', encoding='utf-8'))
cktList = ['c880', 'c1908', 'c2670', 'RCA32', 'CLA32', 'KSA32']
tick = [1, 2, 4, 10]
tab = ['1', '2', '4', '$+\infty$']
mk = ['o', 'x', '+', '*', 's', 'd', 'v', '^', '<', '>', 'h', 'p']

cnt = 0
cntCkt = 0
tr = []
for row in csvRd:
    if cnt == 4:
        print(tick, tr)
        lines = plt.plot(tick, tr, markersize=10, label=cktList[cntCkt], marker=mk[cntCkt], fillstyle='none')
        lines[0].set_linewidth(1.5)
        cnt = 0
        cntCkt += 1
        tr = []
        continue
    tr.append(float(row[2]))
    # print(cktList[cntCkt], tab[cnt], row[2])
    cnt += 1


ftSize = 13
plt.xlabel('Level limitation $l$', fontsize=ftSize)
plt.ylabel('Time ratio', fontsize=ftSize)
# plt.xticks(fontsize=ftSize)
plt.xticks(tick, tab, fontsize=ftSize)
plt.yticks(fontsize=ftSize)
plt.legend(loc='upper right', bbox_to_anchor=(1.3, 1), fontsize=ftSize)
# plt.show()
plt.savefig('result/figures/TimeVSl.png', dpi=300, bbox_inches='tight')
