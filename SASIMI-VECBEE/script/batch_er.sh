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

# errorBound=(0.001 0.003 0.005 0.008 0.01 0.03 0.05)
errorBound=(0.05)

for file in data/su/*
do
    if test -f $file
    then
        name=`basename $file`
        filename="${name%%.*}"
        if [[ "$name" == *.blif ]]
        then
            for error in ${errorBound[*]}
            do
                echo ${file} ${error}
                ./sasimi-vecbee -i ${file} -e ${error} -o appNtk-level-${L}/ --maxLevel ${L}  > log-level-${L}/${filename}_${error}.log &
            done
        fi
    fi
done
