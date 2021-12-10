#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import matplotlib.pyplot as plt
import numpy as np


mk = ['1', 'v', '+', '*', 's', 'd', 'o', '^', '<', '>', 'h', 'p', 'x']
cl = ['tab:blue', 'tab:brown', 'tab:orange', 'tab:pink', 'tab:green', 'tab:cyan', 'tab:red']

ftSize = 13
cnt = 0
cntCkt = 0
tr = []
baseline = 2870.0

# accurate
# er = np.array([
# 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.00365,0.00365,0.00365,0.00365,0.00365,0.00365,0.00365,0.00365,0.00365,0.00365,0.0073,0.01095,0.01095,0.0146,0.01825,0.0219,0.02555,0.0292,0.0292,0.03285,0.03285,0.0365,0.04015,0.0438,0.04745,0.0511,0.12775,0.13505,0.1606,0.2044,0.33945,0.37595,0.42705,0.63145,0.82855,0.98915
# ])
# rra = np.array([
# 0,2.9972,3.7084,4.064,4.4196,4.4704,4.5212,4.572,4.6228,4.7244,4.7752,4.826,4.8768,4.9784,5.0292,5.08,5.3848,5.588,5.7404,5.7912,5.842,5.8928,5.9436,5.9944,6.0452,6.4008,6.7564,6.858,7.0612,7.5184,7.7216,7.7216,7.7724,7.9756,8.0264,8.1788,8.2296,8.382,8.4328,8.4328,8.4836,8.636,9.7028,9.9568,10.2616,10.8204,11.5316,11.7348,11.8872,12.3444,12.5476,12.6492
# ])
# lines = plt.plot(er, rra, color='r', marker='o', markersize=10, markeredgewidth=2.0, fillstyle='none', label='with high accuracy\nerror estimation')
# lines[0].set_linewidth(2.0)
# plt.text(er[-1] + 0.01, rra[-1] - 0.8, '(0.99, 12.65)', fontsize=ftSize)

# dashed line
plt.plot([1, 1], [-1, 13], linewidth=1.0, linestyle='--', color='k')

# accurate
with open('./result/motivation/c7552-with-accurate-estimation.log') as f:
    lines = f.readlines()

ers = []
rras = []
er = [0.]
rra = [0.]
for line in lines:
    line = line[:-1]
    # print(line)
    if line.find('run ') != -1:
        if line != 'run 0':
            ers.append(er)
            rras.append(rra)
        er = [0.]
        rra = [0.]
    elif line.find('high') != -1:
        currEr = float(line[22:])
        er.append(currEr * 100.0)
    elif line.find('area') != -1:
        rra.append((1 - float(line[7:]) / baseline) * 100.0)

# print(ers[0], rras[0])
# print(ers[1], rras[1])

allRra = []
length = len(ers)
for i in range(length):
    allRra.append(rras[i][-2])
allRra = np.array(allRra)
# print(allRra.min())
# print(allRra.max())
# print(np.median(allRra))
indexMin = np.argwhere(allRra == allRra.min())[0][0]
indexMax = np.argwhere(allRra == allRra.max())[0][0]
indexMed = np.argwhere(allRra == np.median(allRra))[0][0]
# print(indexMin)
# print(indexMax)
# print(indexMed)
lines = plt.plot(ers[indexMin], rras[indexMin], marker='o', markersize=10, markeredgewidth=2.0, fillstyle='none', label='with accurate$-$worst')
lines[0].set_linewidth(1.0)
# print(ers[indexMin], rras[indexMin])
# plt.text(ers[indexMin][-2] - 0.15, rras[indexMin][-2] - 0.8, '(' + '%.2f,' % (ers[indexMin][-2]) + '%.2f' % (rras[indexMin][-2]) + ')', fontsize=ftSize)

lines = plt.plot(ers[indexMax], rras[indexMax], marker='x', markersize=10, markeredgewidth=2.0, fillstyle='none', label='with accurate$-$best')
lines[0].set_linewidth(1.0)
# print(ers[indexMax], rras[indexMax])
# plt.text(ers[indexMax][-2] -0.20, rras[indexMax][-2] - 0.8, '(' + '%.2f,' % (ers[indexMax][-2]) + '%.2f' % (rras[indexMax][-2]) + ')', fontsize=ftSize)

lines = plt.plot(ers[indexMed], rras[indexMed], marker='+', markersize=10, markeredgewidth=2.0, fillstyle='none', label='with accurate$-$median')
lines[0].set_linewidth(1.0)
# print(ers[indexMed], rras[indexMed])
plt.text(ers[indexMed][-1] - 0.17, rras[indexMed][-1] - 1.8, 'median,\n(' + '%.2f,' % (ers[indexMed][-1]) + '%.2f' % (rras[indexMed][-1]) + ')', fontsize=ftSize)



# approximate
with open('./result/motivation/c7552-without-accurate-estimation.log') as f:
    lines = f.readlines()

ers = []
rras = []
er = [0.]
rra = [0.]
for line in lines:
    line = line[:-1]
    # print(line)
    if line.find('run ') != -1:
        if line != 'run 0':
            ers.append(er)
            rras.append(rra)
        er = [0.]
        rra = [0.]
    elif line.find('high') != -1:
        currEr = float(line[22:])
        er.append(currEr * 100.0)
    elif line.find('area') != -1:
        rra.append((1 - float(line[7:]) / baseline) * 100.0)

# print(ers[0], rras[0])
# print(ers[1], rras[1])

allRra = []
length = len(ers)
for i in range(length):
    allRra.append(rras[i][-2])
allRra = np.array(allRra)
# print(allRra.min())
# print(allRra.max())
# print(np.median(allRra))
indexMin = np.argwhere(allRra == allRra.min())[0][0]
indexMax = np.argwhere(allRra == allRra.max())[0][0]
indexMed = np.argwhere(allRra == np.median(allRra))[0][0]
# print(indexMin)
# print(indexMax)
# print(indexMed)
lines = plt.plot(ers[indexMin], rras[indexMin], marker='o', markersize=10, markeredgewidth=2.0, fillstyle='none', label='without accurate$-$worst')
lines[0].set_linewidth(1.0)
# print(ers[indexMin], rras[indexMin])
# plt.text(ers[indexMin][-2] - 0.15, rras[indexMin][-2] - 0.8, '(' + '%.2f,' % (ers[indexMin][-2]) + '%.2f' % (rras[indexMin][-2]) + ')', fontsize=ftSize)

lines = plt.plot(ers[indexMax], rras[indexMax], marker='x', markersize=10, markeredgewidth=2.0, fillstyle='none', label='without accurate$-$best')
lines[0].set_linewidth(1.0)
# print(ers[indexMax], rras[indexMax])
# plt.text(ers[indexMax][-2] -0.20, rras[indexMax][-2] - 0.8, '(' + '%.2f,' % (ers[indexMax][-2]) + '%.2f' % (rras[indexMax][-2]) + ')', fontsize=ftSize)

lines = plt.plot(ers[indexMed], rras[indexMed], marker='+', markersize=10, markeredgewidth=2.0, fillstyle='none', label='without accurate$-$median')
lines[0].set_linewidth(1.0)
print(ers[indexMed], rras[indexMed])
plt.text(ers[indexMed][-2] - 0.15, rras[indexMed][-2] - 1.8, 'median,\n(' + '%.2f,' % (ers[indexMed][-2]) + '%.2f' % (rras[indexMed][-2]) + ')', fontsize=ftSize)
plt.text(ers[indexMed][-1] - 0.50, rras[indexMed][-1] - 1.8, 'median,\n(' + '%.2f,' % (ers[indexMed][-1]) + '%.2f' % (rras[indexMed][-1]) + ')', fontsize=ftSize)



# plots
plt.xlabel('Error rate (%)', fontsize=ftSize)
plt.ylabel('Ratio of reduced area (%)', fontsize=ftSize)
# plt.xlim([-0.1, 1.4])
plt.ylim([-1, 13])
plt.xticks(fontsize=ftSize)
plt.yticks(fontsize=ftSize)
# plt.legend(loc='lower right', bbox_to_anchor=(1.50, 1), fontsize=ftSize)
plt.legend(loc='upper right', fontsize=10)
# plt.show()
plt.savefig('result/figures/motivation.eps', format='eps', dpi=1000, bbox_inches='tight')
