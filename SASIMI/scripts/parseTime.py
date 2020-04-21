#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os


for root, _, files in os.walk('./er_random_pattern/log'):
    for name in files:
        fileName = os.path.join(root, name)
        with open(fileName, 'r') as f:
            lines = f.readlines()
        pos = lines[-5].find('ALL running time: ')
        assert(pos != -1)
        time = round(float(lines[-5][18: -1]), 1)
        print(name.rstrip('.log') + "," + str(time))
