#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv

csvRd = csv.reader(open('result/NMED/table/NMEDRaw.csv', encoding='utf-8'))
# erBound = [0.001, 0.003, 0.005, 0.008, 0.01, 0.03, 0.05]
erBound = [0.0000153, 0.0000306, 0.0000612, 0.0001224, 0.0002448, 0.0004896, 0.0009792, 0.0019584]
for i in range(len(erBound)):
    erBound[i] *= 100000
bestError = [dict(), dict(), dict(), dict(), dict(), dict(), dict(), dict()]
bestArea =  [dict(), dict(), dict(), dict(), dict(), dict(), dict(), dict()]
bestDelay = [dict(), dict(), dict(), dict(), dict(), dict(), dict(), dict()]
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
