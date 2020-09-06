#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv

csvRd = csv.reader(open('result/maxLevel-NMED/level-inf.csv', encoding='utf-8'))
# erBound = [0.05]
erBound = [0.0000153 * 100000]
# erBound = [0.001, 0.003, 0.005, 0.008, 0.01, 0.03, 0.05]
# erBound = [0.0000153, 0.0000306, 0.0000612, 0.0001224, 0.0002448, 0.0004896, 0.0009792, 0.0019584]
# erBound = [0.050687623, 0.102554028, 0.25108055, 0.400785855, 0.049508841, 0.152062868, 0.351277014, 0.452652259, 0.050687623, 0.249901768, 0.299410609, 0.451473477, 0.150884086, 0.5021611, 0.298231827, 0.41021611]
# for i in range(len(erBound)):
#     # erBound[i] /= 100
#     erBound[i] *= 100000
bestError = [dict() for i in range(len(erBound))]
bestArea =  [dict() for i in range(len(erBound))]
bestDelay = [dict() for i in range(len(erBound))]
for row in csvRd:
    if row[0] == 'circuit':
        continue
    circuit = row[0]
    er = float(row[1])
    area = float(row[2])
    delay = float(row[3])
    for i in range(len(erBound)):
        if er <= erBound[i]:
            if circuit not in bestArea[i]:
                bestError[i][circuit] = er
                bestArea[i][circuit] = area
                bestDelay[i][circuit] = delay
            elif bestArea[i][circuit] > area:
                bestError[i][circuit] = er
                bestArea[i][circuit] = area
                bestDelay[i][circuit] = delay
            elif bestArea[i][circuit] == area and bestDelay[i][circuit] > delay:
                bestError[i][circuit] = er
                bestArea[i][circuit] = area
                bestDelay[i][circuit] = delay

for i in range(len(bestArea)):
    for circuit, delay in bestArea[i].items():
        print(circuit + ',' + str(bestError[i][circuit]) + ',' + str(delay) + ',' + str(bestDelay[i][circuit]))
