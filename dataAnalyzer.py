#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os


class experimentCase:

    def __init__(self, fileName):
        self.ckts = ['c880', 'c1908', 'c2670', 'c3540', 'c5315', 'c7552', 'alu4', 'rca32', 'cla32', 'ksa32', 'mtp8', 'wal8']
        fp = open(fileName, 'r')
        try:
            data = fp.readlines()
        finally:
            fp.close()
        for line in data:
            if line.startswith('fileName = '):
                self.fileName = line[11:-1]
            elif line.startswith('single selection time = '):
                self.time = float(line[24:])
            elif line.startswith('error rate = '):
                self.errorRate = float(line[13:])
            elif line.startswith('#literals = '):
                self.nLiterals = int(line[12:])
            # elif line.startswith('area = '):
            #     self.area = int(line[7:])
            # elif line.startswith('delay = '):
            #     self.delay = float(line[8:])
            elif line.startswith('Total Area\t\t= '):
                self.area = float(line[14:])
            elif line.startswith('Most Negative Slack\t= '):
                self.delay = float(line[23:])
            else:
                assert(0)

    def __str__(self):
        return self.fileName + '\t' + str(self.errorRate) + '\t' + str(self.area) + '\t' + str(self.delay) + '\t' + str(self.nLiterals) + '\t' + str(self.time)


    def __lt__(self, other):
        if isinstance(other, experimentCase):
            id0 = self.ckts.index(self.fileName)
            id1 = self.ckts.index(other.fileName)
            return id0 < id1 or (id0 == id1 and self.errorRate < other.errorRate)
        else:
            return NotImplemented


if __name__ == '__main__':
    directory = './log/'
    fileNames = os.listdir(directory)
    csList = []
    for fileName in fileNames:
        cs = experimentCase(directory+fileName)
        csList.append(cs)
        # print(cs)

    csList.sort()
    circuits = set([])
    for cs in csList:
        circuits.add(cs.fileName)

    nInfo = 5
    nEr = 7
    nRows = nEr * nInfo
    nCols = len(circuits)
    table = []
    for i in range(nRows):
        tmp = []
        for j in range(nCols):
            tmp.append('')
        table.append(tmp)
    for index in range(len(csList)):
        cs = csList[index]
        col = index // nEr
        rowBase = index % nEr * nInfo
        # print(cs)
        # print(rowBase, col)
        table[rowBase + 0][col] = str(cs.area)
        table[rowBase + 1][col] = str(cs.delay)
        table[rowBase + 2][col] = str(cs.errorRate)
        table[rowBase + 3][col] = str(cs.nLiterals)
        table[rowBase + 4][col] = str(cs.time)

    for row in table:
        for ele in row:
            print(ele, end='\t')
        print('')
