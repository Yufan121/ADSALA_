#!/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=work
#SBATCH --time=1-00:00:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=STDOUT.txt

source /software/projects/director2178/yx7184/miniconda3/etc/profile.d/conda.sh
conda activate base


#### params

# csv_path="Gadi_sdot_500M.csv"
# csv_path="Gadi_strsv_500M.csv"
# csv_path="Gadi_sgemm_500M.csv"
csv_path="Setonix_ssymm_500M.csv"
# csv_path="Setonix_dgemm_500M.csv"

num_total_threads=256

echo "csv path: $csv_path"
echo "number of threads: $num_total_threads"


#### run program

python3 model_selection.py ../model_training/$csv_path $num_total_threads

exit
