#!/bin/bash

rm test.csv

for file in data/su/*
do
    if test -f $file
    then
        name=`basename $file`
        filename="${name%%.*}"
        if [[ "$name" == *.blif ]]
        then
            echo ${filename}
            # echo -e "${filename},\c" >> test.csv
            ./sasimi-vecbee -i ${file}
            # echo -e "" >> test.csv
        fi
    fi
done
