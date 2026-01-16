#!/bin/bash
#PBS -q normal
#PBS -P dv19
#PBS -j oe
#PBS -l walltime=24:00:10,mem=32GB
#PBS -l wd
#PBS -l ncpus=48
#

# module unload python3
# module unload intel-mkl
# module load intel-mkl/2020.3.304

# module load python3/3.9.2
source /scratch/kx58/yx7184/miniconda/etc/profile.d/conda.sh
conda activate base


# get machine-specific number of threads
num_threads=`lscpu | grep "Thread(s) per core" | awk '{print $4}'`
# get machine-specific number of cores
num_cores=`lscpu | grep "Core(s) per socket" | awk '{print $4}'`
# get machine-specific number of sockets
num_sockets=`lscpu | grep "Socket(s)" | awk '{print $2}'`
# get total number of threads
num_total_threads=`expr $num_threads \* $num_cores \* $num_sockets`


# csv_path="Gadi_sdot_500M.csv"
# csv_path="Gadi_ssymm_500M.csv"
# csv_path="Gadi_strsv_500M.csv"
# csv_path="Gadi_sgemm_500M.csv"
csv_path="Gadi_ssymm_500M.csv"

num_total_threads=96

echo "csv path: $csv_path"
echo "number of threads: $num_total_threads"

python3 main.py $csv_path $num_total_threads


cp xgb_* /home/659/yx7184/Project\ ADSALA/Barca\ repo/ADSALA/build/library_frame/test

exit
