#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os


class experimentCase:

    def __init__(self, fileName):
        fp = open(fileName, 'r')
        try:
            data = fp.readlines()
        finally:
            fp.close()
        for line in data:
            pass
            # print(line)


if __name__ == '__main__':
    directory = './log/'
    fileNames = os.listdir(directory)
    for fileName in fileNames:
        cs = experimentCase(directory+fileName)
