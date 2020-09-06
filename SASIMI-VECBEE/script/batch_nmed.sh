#!/bin/bash

rm -rf build/
mkdir build/
cd build/
cmake ..
make -j4
cd ..

L=4

rm -rf log-level-${L}/
mkdir log-level-${L}/

rm -rf appNtk-level-${L}/
mkdir appNtk-level-${L}/


# errorBound=(0.0000153 0.0000306 0.0000612 0.0001224 0.0002448 0.0004896 0.0009792 0.0019584)
errorBound=(0.0000153)

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
                ./sasimi-vecbee -i ${file} -e ${error} --metricType nmed -o appNtk-level-${L}/ --maxLevel ${L}  > log-level-${L}/${filename}_${error}.log &
            done
        fi
    fi
done
