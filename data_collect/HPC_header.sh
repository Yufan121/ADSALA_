#!/bin/bash
#PBS -q normal
#PBS -P dv19
#PBS -j oe
#PBS -l walltime=10:00:10,mem=192GB
#PBS -l other=hyperthread
#PBS -l wd
#PBS -l ncpus=48
#PBS -o ./STDOUT