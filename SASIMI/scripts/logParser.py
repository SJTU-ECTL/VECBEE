#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

for root, _, files in os.walk('er_random_pattern/record/test'):
    for name in files:
        pos0 = root.find('/test/')
        ckt = root[pos0 + 6:]
        pos0 = name.find('_')
        if (pos0 == -1):
            continue
        pos1 = name[pos0 + 1:].find('_')
        error = name[pos0 + pos1 + 2:].rstrip('.blif')
        ret = os.popen('sis -x -c \"read_library blif_used/mcnc.genlib; read_blif ' + os.path.join(root, name) + '; map; print_map_stats\" | grep -e \"Area\" -e \"Most Negative Slack\"').read()
        pos1 = ret.find('=')
        pos2 = ret[pos1:].find('\n')
        area = ret[pos1 + 2: pos1 + pos2]
        pos3 = ret[pos1 + pos2:].find('=')
        pos4 = ret[pos1 + pos2 + pos3:].find('\n')
        delay = ret[pos1 + pos2 + pos3 + 3: pos1 + pos2 + pos3 + pos4]
        print(ckt + ',' + error + ',' + area + ',' + delay)

# for root, _, files in os.walk('aem_random_pattern/record/Confi_AEM'):
#     for name in files:
#         pos0 = root.find('/Confi_AEM/')
#         ckt = root[pos0 + 11:]
#         pos0 = name.find('_')
#         if (pos0 == -1):
#             continue
#         pos1 = name[pos0 + 1:].find('_')
#         error = name[pos0 + pos1 + 2:].rstrip('.blif')
#         ret = os.popen('sis -x -c \"read_library blif_used/mcnc.genlib; read_blif ' + os.path.join(root, name) + '; map; print_map_stats\" | grep -e \"Area\" -e \"Most Negative Slack\"').read()
#         pos1 = ret.find('=')
#         pos2 = ret[pos1:].find('\n')
#         area = ret[pos1 + 2: pos1 + pos2]
#         pos3 = ret[pos1 + pos2:].find('=')
#         pos4 = ret[pos1 + pos2 + pos3:].find('\n')
#         delay = ret[pos1 + pos2 + pos3 + 3: pos1 + pos2 + pos3 + pos4]
#         print(ckt + ',' + error + ',' + area + ',' + delay)
