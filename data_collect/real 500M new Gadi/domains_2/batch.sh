for i in `seq 0 1 17`;
do
job="$i.sh"
sbatch $job
echo "Job submitted = $i"
done