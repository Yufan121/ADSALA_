#!/bin/bash -l
#SBATCH --account=director2178
#SBATCH --partition=long
#SBATCH --time=4-00:00:00
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=256
#SBATCH --threads-per-core=2
#SBATCH --output=STDOUT

# test affinity
#th1=0 # exclusive access to node
#OMP_NUM_THREADS=32 GOMP_CPU_AFFINITY="$((th1+0))-$((th1+15)),$((th1+128))-$((th1+OMP_NUM_THREADS+112-1))" numactl -i all /home/mdelapierre/hpc-dummies/misc/xthi_aocc >0.affinity.test.out

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

    # echo "$col $LINE_CNT"
done

num_rows=`expr $LINE_CNT - 1`
echo "number of rows: $num_rows"
num_of_duplicate=10

th1=0 # exclusive access to node

for i in `seq 1 1 16`;
do
	for line_id in `seq 1 1 $num_rows`; #0-15.
	do 
		export OMP_NUM_THREADS=$i
		let cpu_m1=${OMP_NUM_THREADS}-1
		export GOMP_CPU_AFFINITY="$((th1+0))-$((th1+cpu_m1))"
        	line_id=$[(line_id-1)*num_of_duplicate+1]
        	echo $line_id
		${NUMA} ../main real.d blis single ${OMP_NUM_THREADS} $line_id
	done
	echo "Test ended for nt = $i"
done
for i in `seq 17 1 32`;
do
        for line_id in `seq 1 1 $num_rows`; # 0-15, 128-143.
        do
                export OMP_NUM_THREADS=$i
                let cpu_m1=${OMP_NUM_THREADS}+112-1
                export GOMP_CPU_AFFINITY="$((th1+0))-$((th1+15)),$((th1+128))-$((th1+cpu_m1))"
                line_id=$[(line_id-1)*num_of_duplicate+1]
                echo $line_id
                ${NUMA} ../main real.d blis single ${OMP_NUM_THREADS} $line_id
        done
        echo "Test ended for nt = $i"
done


exit
