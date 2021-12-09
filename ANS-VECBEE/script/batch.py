# -*- coding: utf-8 -*-

import os

ckts = [
'adder',
'arbiter',
'bar',
'cavlc',
'ctrl',
'dec',
'div',
'i2c',
'int2float',
'log2',
'max',
'mem',
'multiplier',
'priority',
'router',
'sin',
'sqrt',
'square',
'voter'
]

for ckt in ckts:
    cmd = 'sis -x -c \"read_library data/genlib/mcnc.genlib; read_blif mapped/' + ckt + '_back.blif; print_map_stats; print_delay -a -p 1\" | grep -e \"Area\" -e \"arrival\"'
    # cmd = 'sis -x -c \"read_library data/genlib/mcnc.genlib; read_blif mapped/' + ckt + '_small_delay.blif; print_map_stats; print_delay -a -p 1\" | grep -e \"Area\" -e \"arrival\"'
    # cmd = 'sis -x -c \"read_library data/genlib/mcnc.genlib; read_blif data/epfl/' + ckt + '_gate.blif; print_map_stats; print_delay -a -p 1\" | grep -e \"Area\" -e \"arrival\"'
    # cmd = 'sis -x -c \"read_blif ' + ckt + '_gate_sop.blif; write_blif ' + ckt + '.blif\"'
    # cmd = './main -f data/epfl/' + ckt + '.blif > log/' + ckt + '.log &'
    print(cmd)
    os.system(cmd)
