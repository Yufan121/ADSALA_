# for foolders whose name contains syrk, syr2k, trmm, trsm, cd into it and do ...

source /scratch/kx58/yx7184/miniconda/etc/profile.d/conda.sh
conda activate base


for dir in `ls -d ../model_training/*syrk*.csv ../model_training/*trmm*.csv ../model_training/*syr2k*.csv ../model_training/*trsm*.csv ../model_training/*symm*.csv ../model_training/*gemm*.csv`
do
    # cd ${dir}

        echo $dir

        # remove path from dir 
        dir=${dir##*/}
        
        # Extract "ssymm" from the filename
        extracted_string=${dir#*_} # This will remove everything before and including the first underscore
        extracted_string=${extracted_string%_*} # This will remove everything after and including the last underscore

        # generate bash script from Selection_Gadi.sh
        # replace ssymm with $extracted_string
        sed "s/ssymm/$extracted_string/g" Selection_Gadi.sh > Selection_Gadi_$extracted_string.sh

    # cd ../
    
done