#!/bin/bash
#PBS -q normal
#PBS -P xb00
#PBS -j oe
#PBS -l walltime=10:00:10,mem=192GB
#PBS -l other=hyperthread
#PBS -l wd
#PBS -l ncpus=48
#PBS -o ./STDOUT/5.out

NUMA="numactl -i all"

#!/bin/bash

columns=(m)
LINE_CNT=0

# get num of rows 
for col in "${columns[@]}"
do
    LINE_CNT=$(awk '-F[\t ]*,[\t ]*' -vcol=${col} '
    FNR==1 {
        for(i=1; i<=NF; ++i) {
            if($i == col) {
                col = i;
                break;
            }
        }
        if(i>NF) {
            exit 1;
        }
    }
    FNR>1 {
        if($col) maxc=FNR;
    }
    END{
        print maxc;
    }' 5.d)

    # echo "$col $LINE_CNT" change 5.d!!
done

num_rows=`expr $LINE_CNT - 1`
echo "number of rows: $num_rows"
num_of_duplicate=10
export OMP_PROC_BIND=close
export OMP_PLACES=cores


for i in `seq 4 1 98`;
do
	for line_id in `seq 1 1 $num_rows`; 
	do 
		export OMP_NUM_THREADS=$i
		# let cpu_m1=${OMP_NUM_THREADS}-1
		# export GOMP_CPU_AFFINITY="0-$cpu_m1"
        	line_id=$[(line_id-1)*num_of_duplicate+1]
        	echo $line_id
		${NUMA} ../../lib/main 5.d mkl single ${OMP_NUM_THREADS} $line_id # remember to change this, since a template
	done
	echo "Test ended for nt = $i"
done


exit
