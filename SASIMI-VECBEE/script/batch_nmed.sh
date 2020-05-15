#!/bin/bash

rm -rf build/
mkdir build/
cd build/
cmake ..
make -j4
cd ..

rm -rf log/
mkdir log/

rm -rf appNtk/
mkdir appNtk/

errorBound=(0.0000153 0.0000306 0.0000612 0.0001224 0.0002448 0.0004896 0.0009792 0.0019584)

for file in data/su/*
do
    if test -f $file
    then
        name=`basename $file`
        filename="${name%%.*}"
        if [[ "$name" == cla32.blif || "$name" == ksa32.blif || "$name" == mtp8.blif || "$name" == rca32.blif || "$name" == wal8.blif ]]
        then
            for error in ${errorBound[*]}
            do
                echo ${file} ${error}
                ./sasimi-vecbee -i ${file} -e ${error} --metricType nmed --maxLevel 1 > log/${filename}_${error}.log
            done
        fi
    fi
done
