#!/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=debug
#SBATCH --time=02:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=xthi_2.out

export OMP_PROC_BIND=close
export OMP_PLACES=threads
export OMP_NUM_THREADS=256

numactl -i all ./xthi |sort -k 6 -n

