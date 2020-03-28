#!/usr/bin/env python
# -*- coding: utf-8 -*-


import matplotlib.pyplot as plt


def CollectData(fileName):
    with open(fileName, 'r') as f:
        lines = f.readlines()
    estErr, accErr, iterNum = [], [], []
    cnt = 0
    for i in range(len(lines)):
        pos = lines[i].find('Now Error is')
        if pos != -1:
            baseErr = float(lines[i][pos + 13: -1])
            if baseErr > 0.05:
                break
            pos2 = lines[i + 1].find(',')
            estErr0 = baseErr + float(lines[i + 1][24: pos2])
            accErr0 = baseErr + float(lines[i + 1][pos2 + 21: -1])
            estErr.append(estErr0 * 100)
            accErr.append(accErr0 * 100)
            cnt += 1
            iterNum.append(cnt)
            print(baseErr, estErr0, accErr0)
    return [estErr, accErr, iterNum]


ftSize = 13
estErr, accErr, iterNum = CollectData('EERvsSER/c880.log')
plt.plot(iterNum, estErr, color='#00FF00', marker='o', linestyle='--', fillstyle='none', markersize=10, label='EER of c880')
plt.plot(iterNum, accErr, color='#00FF00', marker='o', linestyle='-',  fillstyle='full', markersize=10,  label='AER of c880')
estErr, accErr, iterNum = CollectData('EERvsSER/rca32.log')
plt.plot(iterNum, estErr, color='#FF0000', marker='s', linestyle='--', fillstyle='none', markersize=10, label='EER of RCA32')
plt.plot(iterNum, accErr, color='#FF0000', marker='s', linestyle='-',  fillstyle='full', markersize=10,  label='AER of RCA32')
estErr, accErr, iterNum = CollectData('EERvsSER/cla32.log')
plt.plot(iterNum, estErr, color='#0000FF', marker='d', linestyle='--', fillstyle='none', markersize=10, label='EER of CLA32')
plt.plot(iterNum, accErr, color='#0000FF', marker='d', linestyle='-',  fillstyle='full', markersize=10,  label='AER of CLA32')
plt.xlabel('Iteration', fontsize=ftSize)
plt.ylabel('Error rate (%)', fontsize=ftSize)
plt.xticks(fontsize=ftSize)
plt.yticks(fontsize=ftSize)
plt.legend(loc='upper right', bbox_to_anchor=(0.8, 1), fontsize=ftSize)
# plt.show()
plt.savefig('plot.png', dpi=600, bbox_inches='tight')
