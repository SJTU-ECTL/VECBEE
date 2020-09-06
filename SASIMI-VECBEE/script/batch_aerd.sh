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

levelList=(1 2 4)

for file in data/su/*
do
    if test -f $file
    then
        name=`basename $file`
        filename="${name%%.*}"
        if [[ "$name" == c880.blif || "$name" == c1908.blif || "$name" == c2670.blif || "$name" == rca32.blif || "$name" == cla32.blif || "$name" == ksa32.blif ]]
        then
            for level in ${levelList[*]}
            do
                echo ${file} ${level}
                ./sasimi-vecbee -i ${file} --maxLevel ${level} > log/${filename}_level_${level}.log &
            done
        fi
    fi
done
