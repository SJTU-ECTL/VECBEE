#!/bin/bash

make rebuild

# rm -rf log/
# rm -rf record/
# mkdir record
# mkdir record/Confi_AEM
# mkdir log

echo rca32
./main rca32 0.00050688 > log/rca32_0.00050688.log
./main rca32 0.00102554 > log/rca32_0.00102554.log
./main rca32 0.00251081 > log/rca32_0.00251081.log
./main rca32 0.00400786 > log/rca32_0.00400786.log
echo cla32
./main cla32 0.00049509 > log/cla32_0.00049509.log
./main cla32 0.00152063 > log/cla32_0.00152063.log
./main cla32 0.00351277 > log/cla32_0.00351277.log
./main cla32 0.00452652 > log/cla32_0.00452652.log
echo ksa32
./main ksa32 0.00050688 > log/ksa32_0.00050688.log
./main ksa32 0.00249902 > log/ksa32_0.00249902.log
./main ksa32 0.00299411 > log/ksa32_0.00299411.log
./main ksa32 0.00451473 > log/ksa32_0.00451473.log
echo mul8
./main mul8 0.00150884 > log/mul8_0.00150884.log
./main mul8 0.00502161 > log/mul8_0.00502161.log
echo wtm8
./main wtm8 0.00298232 > log/wtm8_0.00298232.log
./main wtm8 0.00410216 > log/wtm8_0.00410216.log
