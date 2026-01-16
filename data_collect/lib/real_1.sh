#!/usr/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=work
#SBATCH --time=1-00:00:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=STDOUT

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
    }' real.d)

    # echo "$col $LINE_CNT" change real.d!!
done

num_rows=`expr $LINE_CNT - 1`
echo "number of rows: $num_rows"
num_of_duplicate=10

# get machine-specific number of threads
num_threads=`lscpu | grep "Thread(s) per core" | awk '{print $4}'`
# get machine-specific number of cores
num_cores=`lscpu | grep "Core(s) per socket" | awk '{print $4}'`
# get machine-specific number of sockets
num_sockets=`lscpu | grep "Socket(s)" | awk '{print $2}'`
# get total number of threads
num_total_threads=`expr $num_threads \* $num_cores \* $num_sockets`
echo "number of threads: $num_total_threads"

subroutine="gemm"

for i in `seq 1 1 $num_total_threads`;
do
	for line_id in `seq 1 1 $num_rows`; 
	do 
        # set number of threads
		export OMP_NUM_THREADS=$i
		let cpu_m1=${OMP_NUM_THREADS}-1
		export GOMP_CPU_AFFINITY="0-$cpu_m1"
        line_id=$[(line_id-1)*num_of_duplicate+1]
        echo $line_id

        # echo in red color
        echo "$NUMA ../lib/main ${subroutine} real.d mkl single ${OMP_NUM_THREADS} $line_id" |  awk '{printf "\033[31m%s\033[0m\n", $0}'
        # run the program
		${NUMA} ../lib/main ${subroutine} real.d mkl single ${OMP_NUM_THREADS} $line_id 
	done
    # echo in green color
	echo "Test ended for nt = $i" |  awk '{printf "\033[32m%s\033[0m\n", $0}'
done


exit
