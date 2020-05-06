#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os


for root, _, files in os.walk('./result/NMED/log'):
    for name in files:
        fileName = os.path.join(root, name)
        with open(fileName, 'r') as f:
            lines = f.readlines()
        pos = lines[-4].find('time = ')
        assert(pos != -1)
        time = round(float(lines[-4][7: -3])/1000000, 1)
        print(name.rstrip('.log') + "," + str(time))
