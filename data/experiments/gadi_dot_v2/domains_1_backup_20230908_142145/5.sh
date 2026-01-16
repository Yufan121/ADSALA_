#!/bin/bash
#PBS -q express
#PBS -P kx58
#PBS -j oe
#PBS -l walltime=10:00:10,mem=192GB
#PBS -l storage=scratch/kx58
#PBS -l other=hyperthread
#PBS -l wd
#PBS -l ncpus=48

# module load intel-mkl


NUMA="numactl -i all"
file_path="./5.d"

num_rows=$(wc -l < "$file_path")
# num_rows -= 1
num_rows=`expr $num_rows - 1`

# export OMP_PROC_BIND=close
# export OMP_PLACES=cores

# get machine-specific number of threads
num_threads=`lscpu | grep "Thread(s) per core" | awk '{print $4}'`
# get machine-specific number of cores
num_cores=`lscpu | grep "Core(s) per socket" | awk '{print $4}'`
# get machine-specific number of sockets
num_sockets=`lscpu | grep "Socket(s)" | awk '{print $2}'`
# get total number of threads
num_total_threads=`expr $num_threads \* $num_cores \* $num_sockets`
echo "number of threads: $num_total_threads"

export MKL_DYNAMIC=FALSE # https://www.intel.com/content/www/us/en/docs/onemkl/developer-guide-windows/2023-0/mkl-dynamic.html
export MKL_VERBOSE=2

export OMP_DYNAMIC=FALSE
export OMP_PROC_BIND=close # spread
export OMP_PLACES=cores # threads

subroutine="dot"

for i in `seq 1 1 $num_total_threads`; do
    echo "Test started for nt = $i"
    # for line_id in $(seq 1 10 $num_rows); do # 10 is interval
    for line_id in `seq 1 1 10`; do # 10 is interval``
        echo line_id=$line_id 
        export OMP_NUM_THREADS=$i
        # export MKL_NUM_THREADS=$i
        # export GOMP_CPU_AFFINITY="0-$((OMP_NUM_THREADS - 1))"

        echo omp_num_threads=$OMP_NUM_THREADS "*********************************************"

        # run the program
        ${NUMA} ../../lib/main ${subroutine} 5.d mkl single ${i} $line_id

        echo "********************************************************************"
        echo
    done
    echo "Test ended for nt = $i"
    echo
    echo
done

exit
