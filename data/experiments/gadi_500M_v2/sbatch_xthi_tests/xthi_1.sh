#!/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=debug
#SBATCH --time=02:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=xthi_1.out

export OMP_NUM_THREADS=256
let cpu_m1=${OMP_NUM_THREADS}-1
export GOMP_CPU_AFFINITY="0-$cpu_m1"

numactl -i all ./xthi |sort -k 6 -n

