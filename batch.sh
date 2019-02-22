#!/bin/bash

make rebuild

if [ ! -d log ]
then
    mkdir log
fi

errorArray=(0.001 0.003 0.005 0.008 0.01 0.03 0.05)
len=${#errorArray[*]}

for file in data/su/*
do
    if test -f $file
    then
        name=`basename $file`
		filename="${name%%.*}"
       	if [[ "$name" == *.blif ]]
       	then
            for er in ${errorArray[@]}
            do
                echo -e 'time ./main -f' ${file} '-n 10000' -e ${er} '> log/'${er}'_'${filename}'.log'
                time ./main -f ${file} -n 10000 -e ${er} > log/${er}_${filename}.log
            done
		fi
    fi
done


