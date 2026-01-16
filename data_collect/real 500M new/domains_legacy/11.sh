#!/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=work
#SBATCH --time=1-00:00:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=STDOUT/11.out

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
    }' 11.d)

    # echo "$col $LINE_CNT" change 11.d!!
done

num_rows=`expr $LINE_CNT - 1`
echo "number of rows: $num_rows"
num_of_duplicate=10

for i in `seq 1 1 256`;
do
	for line_id in `seq 1 1 $num_rows`; 
	do 
		export OMP_NUM_THREADS=$i
		let cpu_m1=${OMP_NUM_THREADS}-1
		export GOMP_CPU_AFFINITY="0-$cpu_m1"
        	line_id=$[(line_id-1)*num_of_duplicate+1]
        	echo $line_id
		${NUMA} ../../lib/main 11.d blis single ${OMP_NUM_THREADS} $line_id # remember to change this, since a template
	done
	echo "Test ended for nt = $i"
done


exit
