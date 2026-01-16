#!/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=work
#SBATCH --time=1-00:00:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=STDOUT/15.out

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
    }' 15.d)

    # echo "$col $LINE_CNT"
done

num_rows=`expr $LINE_CNT - 1`
echo "number of rows: $num_rows"
num_of_duplicate=10

export OMP_PROC_BIND=close
export OMP_PLACES=threads

for i in `seq 1 1 96`;
do
	for line_id in `seq 1 1 $num_rows`; 
	do 
		export OMP_NUM_THREADS=$i
        	line_id=$[(line_id-1)*num_of_duplicate+1]
        	echo $line_id
		${NUMA} ../../lib/main 15.d mkl single ${OMP_NUM_THREADS} $line_id
	done
	echo "Test ended for nt = $i"
done


exit
