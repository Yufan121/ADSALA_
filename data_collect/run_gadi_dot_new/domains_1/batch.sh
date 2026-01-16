for i in `seq 1 1 10`;
do
job="$i.sh"
qsub $job
echo "Job submitted = $i"
done