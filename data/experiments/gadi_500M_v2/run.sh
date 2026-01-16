module load py-pandas/1.3.4
../lib/generate-domain  0 500 72376 z__.d 3 0 # generate domain(min_size, max_size (in MB), random seed, filename, sample stepsize, isDouble (0 or 1))
# python3 domain_split.py #split and create batch job
# cd domains
# sh batch.sh
