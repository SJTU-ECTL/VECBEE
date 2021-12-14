#!/usr/bin/env python
# -*- coding: utf-8 -*-


import matplotlib.pyplot as plt


def CollectData(fileName):
    with open(fileName, 'r') as f:
        lines = f.readlines()
    estErr, accErr, iterNum = [], [], []
    cnt = 0
    for i in range(len(lines)):
        pos = lines[i].find('estimated error ')
        if pos != -1:
            estErr0 = float(lines[i][pos + 16: -1])
            accErr0 = float(lines[i + 1][8: -1])
            estErr.append(estErr0 * 100)
            accErr.append(accErr0 * 100)
            cnt += 1
            iterNum.append(cnt)
            print(estErr0, accErr0)
    return [estErr, accErr, iterNum]


plt.rcParams['figure.figsize'] = (6.5, 2.5)

ftSize = 13
estErr, accErr, iterNum = CollectData('result/maxLevel-ER/log-level-1/c880_0.05.log')
plt.plot(iterNum, estErr, color='#00FF00', marker='o', linestyle='--', fillstyle='none', markersize=10, label='EER of c880')
plt.plot(iterNum, accErr, color='#00FF00', marker='o', linestyle='-',  fillstyle='full', markersize=10,  label='SER of c880')
estErr, accErr, iterNum = CollectData('result/maxLevel-ER/log-level-1/rca32_0.05.log')
plt.plot(iterNum, estErr, color='#FF0000', marker='s', linestyle='--', fillstyle='none', markersize=10, label='EER of RCA32')
plt.plot(iterNum, accErr, color='#FF0000', marker='s', linestyle='-',  fillstyle='full', markersize=10,  label='SER of RCA32')
estErr, accErr, iterNum = CollectData('result/maxLevel-ER/log-level-1/cla32_0.05.log')
plt.plot(iterNum, estErr, color='#0000FF', marker='d', linestyle='--', fillstyle='none', markersize=10, label='EER of CLA32')
plt.plot(iterNum, accErr, color='#0000FF', marker='d', linestyle='-',  fillstyle='full', markersize=10,  label='SER of CLA32')
plt.xlabel('Iteration', fontsize=ftSize)
plt.ylabel('Error rate (%)', fontsize=ftSize)
plt.xticks(fontsize=ftSize)
plt.yticks(fontsize=ftSize)
plt.legend(loc='upper right', bbox_to_anchor=(0.66, 1), fontsize=11)
# plt.tight_layout()
# plt.show()
plt.savefig('result/figures/EstimateErrorVSAccurateError.eps', dpi=1000, bbox_inches='tight')
