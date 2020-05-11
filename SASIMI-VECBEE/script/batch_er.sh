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

errorBound=(0.001 0.003 0.005 0.008 0.01 0.03 0.05)

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
                ./sasimi-vecbee -i ${file} -e ${error} --maxLevel 1 > log/${filename}_${error}.log
            done
        fi
    fi
done
