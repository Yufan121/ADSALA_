#!/bin/bash
#PBS -q normal
#PBS -P kx58
#PBS -j oe
#PBS -l walltime=24:00:10,mem=32GB
#PBS -l wd
#PBS -l ncpus=48

source /scratch/kx58/yx7184/miniconda/etc/profile.d/conda.sh
conda activate base



#### params

# csv_path="Gadi_sdot_500M.csv"
# csv_path="Gadi_strsv_500M.csv"
# csv_path="Gadi_sgemm_500M.csv"
csv_path="Gadi_ssymm_500M.csv"
# csv_path="Gadi_dgemm_500M.csv"

num_total_threads=96

echo "csv path: $csv_path"
echo "number of threads: $num_total_threads"


#### run program

python3 model_selection.py ../model_training/$csv_path $num_total_threads

exit
