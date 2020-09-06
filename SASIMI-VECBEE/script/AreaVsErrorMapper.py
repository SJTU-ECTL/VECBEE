#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os


for root, _, files in os.walk('./result/maxLevel-NMED/appNtk-level-inf/'):
    for name in files:
        pos0 = name.find('_')
        pos1 = name[pos0 + 1:].find('_')
        pos2 = name[pos0 + 1 + pos1 + 1:].find('_')
        ckt = name[: pos0]
        error = name[pos0 + 1 + pos1 + 1: pos0 + 1 + pos1 + 1 + pos2]
        ret = os.popen('sis -x -c \"read_library data/library/mcnc.genlib; read_blif ' + os.path.join(root, name) + '; map; print_map_stats\" | grep -e \"Area\" -e \"Most Negative Slack\"').read()
        pos1 = ret.find('=')
        pos2 = ret[pos1:].find('\n')
        area = ret[pos1 + 2: pos1 + pos2]
        pos3 = ret[pos1 + pos2:].find('=')
        pos4 = ret[pos1 + pos2 + pos3:].find('\n')
        delay = ret[pos1 + pos2 + pos3 + 3: pos1 + pos2 + pos3 + pos4]
        print(ckt + ',' + error + ',' + area + ',' + delay)
