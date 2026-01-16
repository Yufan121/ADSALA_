#!/bin/bash -l
#PBS -q normal
#PBS -P kx58
#PBS -j oe
#PBS -l walltime=23:00:10,mem=192GB
#PBS -l other=hyperthread
#PBS -l wd
#PBS -l ncpus=48
#PBS -o ./STDOUT/8.out

NUMA="numactl -i all"

#!/bin/bash

fn=0.d
# get number of lines in the file
LINE_CNT=`wc -l $fn | awk '{print $1}'`

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

subroutine="dot"

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

        command="$NUMA ../../lib/main ${subroutine} 8.d mkl single ${OMP_NUM_THREADS} $line_id"

        # echo in red color
        echo $command |  awk '{printf "\033[31m%s\033[0m\n", $0}'
        # run the program
		$command
	done
    # echo in green color
	echo "Test ended for nt = $i" |  awk '{printf "\033[32m%s\033[0m\n", $0}'
done


exit
