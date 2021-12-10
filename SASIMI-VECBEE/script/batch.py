# -*- coding: utf-8 -*-

import os

# for i in range(70, 100):
#     cmd = './sasimi-vecbee -i data/su/c7552.blif -e 0.03 -f 100000 > log/c7552_' + str(i) + '.log &'
#     print(cmd)
#     os.system(cmd)


cmd = 'echo \"\" > test.log'
os.system(cmd)
rd = 0
for root, dirs, files in os.walk("./result/motivation/log-without-accurate-estimation", topdown=False):
    for name in sorted(files):
        fileName = os.path.join(root, name)
        cmd = 'echo \"run ' + str(rd) + '\" >> test.log'
        rd += 1
        os.system(cmd)
        cmd = 'cat ' + fileName + ' | grep -e \"high\" -e \"new area = \" >> test.log'
        os.system(cmd)
        print(cmd)
