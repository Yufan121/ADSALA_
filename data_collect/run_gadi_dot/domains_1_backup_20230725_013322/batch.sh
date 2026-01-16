for i in `seq 0 1 20`;
do
job="$i.sh"
qsub $job
echo "Job submitted = $i"
done