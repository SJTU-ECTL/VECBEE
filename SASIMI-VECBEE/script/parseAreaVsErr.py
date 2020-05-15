#!/usr/bin/env python
# -*- coding: utf-8 -*-


import csv
from matplotlib import pyplot as plt


# csvFile = open('result/maxLevel-1/ER/table/ERFilter.csv', 'r')
# reader = csv.reader(csvFile)
# cnt = 0
# mk = ['o', 'x', '+', '*', 's', 'd', 'v', '^', '<', '>', 'h', 'p']
# cntMk = 0
# for item in reader:
#     circuit = item[0]
#     if circuit == '' or circuit == 'circuit':
#         continue
#     erBound = float(item[2])
#     oriArea = float(item[1])
#     appArea = float(item[3])
#     print(circuit, erBound, appArea / oriArea)
#     cnt += 1
#     if cnt == 1:
#         error, ar = [], []
#     error.append(erBound * 100)
#     ar.append(appArea / oriArea)
#     if cnt == 7:
#         # print(error, ar)
#         plt.plot(error, ar, label=circuit, marker=mk[cntMk], fillstyle='none', markersize=8)
#         cnt = 0
#         cntMk += 1
# ftSize = 13
# plt.xlabel('Error rate (%)', fontsize=ftSize)
# plt.ylabel('Area ratio', fontsize=ftSize)
# plt.xticks(fontsize=ftSize)
# plt.yticks(fontsize=ftSize)
# plt.legend(loc='upper right', bbox_to_anchor=(1.30, 1), fontsize=ftSize)
# # plt.legend(fontsize=ftSize)
# # plt.show()
# plt.savefig('ErrorRateVSArea.png', dpi=300, bbox_inches='tight')


csvFile = open('result/maxLevel-1/NMED/table/NMEDFilter.csv', 'r')
reader = csv.reader(csvFile)
cnt = 0
mk = ['o', 'x', '+', '*', 's', 'd', 'v', '^', '<', '>', 'h', 'p']
cntMk = 0
for item in reader:
    circuit = item[0]
    if circuit == '' or circuit == 'circuit':
        continue
    erBound = float(item[3])
    oriArea = float(item[1])
    appArea = float(item[4])
    print(circuit, erBound, appArea, oriArea, appArea / oriArea)
    cnt += 1
    if cnt == 1:
        error, ar = [], []
    error.append(erBound * 100)
    ar.append(appArea / oriArea)
    if cnt == 8:
        plt.plot(error, ar, label=circuit, marker=mk[cntMk], fillstyle='none', markersize=8)
        cnt = 0
        cntMk += 1
ftSize = 13
plt.xlabel('AEM rate (%)', fontsize=ftSize)
plt.ylabel('Area ratio', fontsize=ftSize)
plt.xticks(fontsize=ftSize)
plt.yticks(fontsize=ftSize)
plt.legend(loc='upper right', bbox_to_anchor=(1.30, 1), fontsize=ftSize)
# plt.legend(fontsize=ftSize)
# plt.show()
plt.savefig('AverageErrorVSArea.png', dpi=300, bbox_inches='tight')
