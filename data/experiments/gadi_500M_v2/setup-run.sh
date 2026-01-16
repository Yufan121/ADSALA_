#!/bin/bash

# assuming I am in directory 'adsala/data_collect/real 500M'
# get Slurm interactive session
# or equivalent settings for a Slurm jobscript
# salloc --exclusive -N 1 -n 1 -c 256 --threads-per-core=2

module load py-pandas/1.3.4

