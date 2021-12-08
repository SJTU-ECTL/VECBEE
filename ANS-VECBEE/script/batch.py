# -*- coding: utf-8 -*-

import os

ckts = [
'adder',
'arbiter',
'bar',
'cavlc',
'ctrl',
'dec',
# 'div',
'i2c',
'int2float',
'max',
# 'mem',
# 'multiplier',
'priority',
'router',
'sin'
# 'sqrt',
# 'square',
# 'voter'
]
cktsHard = [
'div',
'mem',
'multiplier',
'sqrt',
'square',
'voter'
]

for ckt in cktsHard:
    # cmd = 'sis -x -c \"read_library ../genlib/mcnc.genlib; read_blif ' + ckt + '_gate.blif; print_map_stats; print_delay -a\" | grep -A1 -e \"Area\" -e \"using\"'
    # cmd = 'sis -x -c \"read_blif ' + ckt + '_gate_sop.blif; write_blif ' + ckt + '.blif\"'
    cmd = './main -f data/epfl/' + ckt + '.blif > log/' + ckt + '.log &'
    print(cmd)
    os.system(cmd)
