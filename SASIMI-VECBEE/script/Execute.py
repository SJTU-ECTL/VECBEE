# -*- coding: utf-8 -*-

import os

cmd = './sasimi-vecbee -i data/su/c7552.blif -f 1024 -e 0.01 | grep -e \"high\" -e \"area = \" >> motivation.log'
print(cmd)
for i in range(100):
    if i == 0:
        os.system('echo run ' + str(i) + ' > motivation.log')
    else:
        os.system('echo run ' + str(i) + ' >> motivation.log')
    os.system(cmd)
