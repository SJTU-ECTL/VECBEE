#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import matplotlib.pyplot as plt


csvRd = csv.reader(open('result/compare-time/ERTimeRaw-old.csv', encoding='utf-8'))
cktList = ['c880', 'c1908', 'c2670', 'RCA32', 'CLA32', 'KSA32', 'Arithmean']
tick = [1, 2, 4, 10]
tab = ['1', '2', '4', '$+\infty$']
mk = ['1', 'v', '+', '*', 's', 'd', 'o', '^', '<', '>', 'h', 'p', 'x']
cl = ['tab:blue', 'tab:brown', 'tab:orange', 'tab:pink', 'tab:green', 'tab:cyan', 'tab:red']

ftSize = 13
cnt = 0
cntCkt = 0
tr = []
for row in csvRd:
    # print(row)
    if cnt == 4:
        print(tick, tr)
        if cktList[cntCkt] == 'Arithmean':
            lines = plt.plot(tick, tr, markersize=10, label=cktList[cntCkt], marker=mk[cntCkt], color='r')
            lines[0].set_linewidth(1.5)
            for i in range(4):
                txt = ('%.4f' % (tr[i]))
                if i == 1:
                    plt.text(tick[i] - 0.5, tr[i] + 0.0005, txt, fontsize=ftSize, color='r')
                else:
                    plt.text(tick[i] - 0.9, tr[i] + 0.0003, txt, fontsize=ftSize, color='r')
        else:
            lines = plt.plot(tick, tr, markersize=8, label=cktList[cntCkt], marker=mk[cntCkt], fillstyle='none', color=cl[cntCkt])
            lines[0].set_linewidth(0.75)
        cnt = 0
        cntCkt += 1
        tr = []
        continue
    tr.append(float(row[2]))
    # print(cktList[cntCkt], tab[cnt], row[2])
    cnt += 1


plt.xlabel('Level limitation $l$', fontsize=ftSize)
plt.ylabel('Time ratio', fontsize=ftSize)
plt.xticks(tick, tab, fontsize=ftSize)
plt.yticks(fontsize=ftSize)
plt.xlim([0, 11])
plt.legend(loc='upper right', bbox_to_anchor=(1.35, 1), fontsize=ftSize)
# plt.show()
plt.savefig('result/figures/TimeVSl.eps', format='eps', dpi=1000, bbox_inches='tight')
