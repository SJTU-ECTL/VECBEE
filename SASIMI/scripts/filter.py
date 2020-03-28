#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv

# csvRd = csv.reader(open('result/er_raw.csv', encoding='utf-8'))
# erBound = [0.001, 0.003, 0.005, 0.008, 0.01, 0.03, 0.05]
# bestError = [dict(), dict(), dict(), dict(), dict(), dict(), dict()]
# bestArea = [dict(), dict(), dict(), dict(), dict(), dict(), dict()]
# bestDelay = [dict(), dict(), dict(), dict(), dict(), dict(), dict()]
# for row in csvRd:
#     if row[0] == 'circuit':
#         continue
#     circuit = row[0]
#     er = float(row[1])
#     area = float(row[2])
#     delay = float(row[3])
#     # print(circuit, er, area, delay)
#     for i in range(len(erBound)):
#         if er <= erBound[i]:
#             if circuit not in bestArea[i]:
#                 bestError[i][circuit] = er
#                 bestArea[i][circuit] = area
#                 bestDelay[i][circuit] = delay
#             elif bestArea[i][circuit] > area:
#                 bestError[i][circuit] = er
#                 bestArea[i][circuit] = area
#                 bestDelay[i][circuit] = delay
#             elif bestArea[i][circuit] == area and bestDelay[i][circuit] > delay:
#                 bestError[i][circuit] = er
#                 bestArea[i][circuit] = area
#                 bestDelay[i][circuit] = delay
#
# for i in range(len(bestArea)):
#     for circuit, delay in bestArea[i].items():
#         print(circuit + ',' + str(bestError[i][circuit]) + ',' + str(delay) + ',' + str(bestDelay[i][circuit]))

# csvRd = csv.reader(open('result/aem_raw.csv', encoding='utf-8'))
csvRd = csv.reader(open('result/aem_comp_raw.csv', encoding='utf-8'))
maxEd = {'rca32': 2**33 - 1, 'cla32': 2**33 - 1, 'mul8': 2**16 - 1, 'ksa32': 2**33 - 1, 'wtm8': 2**16 - 1}
# erBound = [2**i / (2**33 - 1) for i in range(17, 25)]
erBound = [0.00049509, 0.00152063, 0.00351277, 0.00452652, 0.00050688, 0.00249902, 0.00299411, 0.00451473, 0.00150884, 0.00502161, 0.00050688, 0.00102554, 0.00251081, 0.00400786, 0.00298232, 0.00410216]
bestArea = [dict() for i in range(20)]
bestDelay = [dict() for i in range(20)]
for row in csvRd:
    if row[0] == 'circuit':
        continue
    circuit = row[0]
    nmed = float(row[1]) / maxEd[circuit]
    area = float(row[2])
    delay = float(row[3])
    # print(circuit, nmed, area, delay)
    for i in range(len(erBound)):
        if nmed <= erBound[i]:
            if circuit not in bestArea[i]:
                bestArea[i][circuit] = area
                bestDelay[i][circuit] = delay
            elif bestArea[i][circuit] > area:
                bestArea[i][circuit] = area
                bestDelay[i][circuit] = delay
            elif bestArea[i][circuit] == area and bestDelay[i][circuit] > delay:
                bestArea[i][circuit] = area
                bestDelay[i][circuit] = delay

for i in range(len(bestArea)):
    for circuit, delay in bestArea[i].items():
        print(circuit + ',' + str(erBound[i]) + ',' + str(delay) + ',' + str(bestDelay[i][circuit]))
