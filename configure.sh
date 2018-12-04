#!/bin/bash

sudo apt update
sudo apt install libreadline-dev
git clone git@github.com:berkeley-abc/abc.git

cd abc/
make ABC_USE_PIC=1 libabc.so
sudo cp libabc.so /usr/lib/
cd -
