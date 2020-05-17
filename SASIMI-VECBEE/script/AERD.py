#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os


for root, _, files in os.walk('./result/AERD/log'):
    for name in files:
        fileName = os.path.join(root, name)
        with open(fileName, 'r') as f:
            lines = f.readlines()
        pos = lines[-9].find(',')
        assert pos != -1
        pos2 = lines[-9][pos + 1: -1].find(',')
        assert pos2 != -1
        pos3 = name.find("_level_")
        print(name[: pos3] + "," + name[pos3 + 7] + "," + lines[-9][pos + 1: pos + pos2 + 1] + "," + lines[-9][pos + 1 + pos2 + 1: -1])
