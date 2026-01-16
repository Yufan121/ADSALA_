for i in `seq 6 1 15`;
do
job="$i.sh"
sbatch $job
echo "Job submitted = $i"
done