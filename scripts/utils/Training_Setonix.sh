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

module load py-pandas/1.3.4
module load py-scikit-learn/1.0.1
module load py-matplotlib/3.4.3
python3 main.py

exit
