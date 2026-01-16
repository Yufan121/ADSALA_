for i in `seq 11 1 17`;
do
job="$i.sh"
qsub $job
echo "Job submitted = $i"
done