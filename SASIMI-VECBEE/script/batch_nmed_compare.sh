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


echo rca32
./sasimi-vecbee -i data/su/rca32.blif -e 0.00050688 --metricType nmed > log/rca32_0.00050688.log
./sasimi-vecbee -i data/su/rca32.blif -e 0.00102554 --metricType nmed > log/rca32_0.00102554.log
./sasimi-vecbee -i data/su/rca32.blif -e 0.00251081 --metricType nmed > log/rca32_0.00251081.log
./sasimi-vecbee -i data/su/rca32.blif -e 0.00400786 --metricType nmed > log/rca32_0.00400786.log
echo cla32
./sasimi-vecbee -i data/su/cla32.blif -e 0.00049509 --metricType nmed > log/cla32_0.00049509.log
./sasimi-vecbee -i data/su/cla32.blif -e 0.00152063 --metricType nmed > log/cla32_0.00152063.log
./sasimi-vecbee -i data/su/cla32.blif -e 0.00351277 --metricType nmed > log/cla32_0.00351277.log
./sasimi-vecbee -i data/su/cla32.blif -e 0.00452652 --metricType nmed > log/cla32_0.00452652.log
echo ksa32
./sasimi-vecbee -i data/su/ksa32.blif -e 0.00050688 --metricType nmed > log/ksa32_0.00050688.log
./sasimi-vecbee -i data/su/ksa32.blif -e 0.00249902 --metricType nmed > log/ksa32_0.00249902.log
./sasimi-vecbee -i data/su/ksa32.blif -e 0.00299411 --metricType nmed > log/ksa32_0.00299411.log
./sasimi-vecbee -i data/su/ksa32.blif -e 0.00451473 --metricType nmed > log/ksa32_0.00451473.log
echo mul8
./sasimi-vecbee -i data/su/mtp8.blif -e 0.00150884 --metricType nmed > log/mtp8_0.00150884.log
./sasimi-vecbee -i data/su/mtp8.blif -e 0.00502161 --metricType nmed > log/mtp8_0.00502161.log
echo wtm8
./sasimi-vecbee -i data/su/wal8.blif -e 0.00298232 --metricType nmed > log/wal8_0.00298232.log
./sasimi-vecbee -i data/su/wal8.blif -e 0.00410216 --metricType nmed > log/wal8_0.00410216.log
