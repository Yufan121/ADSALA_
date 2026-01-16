#!/bin/bash

git clone https://github.com/marcodelapierre/hpc-dummies

cd hpc-dummies/misc
export CRAYPE_LINK_TYPE='dynamic'
cc -g -O3 -fopenmp -o xthi xthi.c

cd ../..
cp -p hpc-dummies/misc/xthi .

rm -fr hpc-dummies/

