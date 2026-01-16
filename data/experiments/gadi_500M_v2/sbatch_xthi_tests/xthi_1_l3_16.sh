#!/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=debug
#SBATCH --time=02:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=xthi_1_l3_16.out

th1=0 # exclusive access to node
export OMP_NUM_THREADS=16
let cpu_m1=${OMP_NUM_THREADS}+120-1
export GOMP_CPU_AFFINITY="$((th1+0))-$((th1+7)),$((th1+128))-$((th1+cpu_m1))"

numactl -i all ./xthi |sort -k 6 -n

