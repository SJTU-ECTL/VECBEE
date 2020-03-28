#!/bin/bash

make rebuild

rm -rf log/
rm -rf record/
mkdir record
mkdir record/Confi_AEM
mkdir log

for file in ./blif_used/*
do
    if test -f $file
    then
        name=`basename $file`
		filename="${name%%.*}"
       	if [ "$name" == cla32.blif ] || [ "$name" == ksa32.blif ] || [ "$name" == mul8.blif ] || [ "$name" == rca32.blif ] || [ "$name" == wtm8.blif ]
       	then
            echo ${filename}
            if [ ! -d ./record/Confi_AEM/${filename} ]
            then
                mkdir ./record/Confi_AEM/${filename}
            fi
            (nohup ./main ${filename} > log/${filename}.log &)
		fi
    fi
done
