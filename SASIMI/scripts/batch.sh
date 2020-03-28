#!/bin/bash

make rebuild

rm -rf log/
rm -rf record/
mkdir record
mkdir record/test
if [ ! -d log ]
then
    mkdir log
fi

for file in ./blif_used/*
do
    if test -f $file
    then
        name=`basename $file`
		filename="${name%%.*}"
       	if [[ "$name" == *.blif ]]
       	then
            echo ${filename}
            if [ ! -d ./record/test/${filename} ]
            then
                mkdir ./record/test/${filename}
            fi
            (nohup ./main ${filename} > log/${filename}.log &)
		fi
    fi
done
