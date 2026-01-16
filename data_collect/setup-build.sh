#!/bin/bash

# assuming I am in adsala root directory
# also assuming I have added Marco,s mod files inside 'data_collect/real 500M'

# Build AMDblis 3.1 with Spack and GCC compiler
module load spack/0.17.0
sg $PAWSEY_PROJECT -c 'spack install amdblis@3.1~suphandling threads=openmp'
blis_path="$( spack find -p amdblis@3.1 | grep /software | tr -s ' ' | cut -d ' ' -f 2 )"
# copy over AMDblis library files
cd 'data_collect/lib'
rm -f libblis-mt.*
cp -p $blis_path/lib/libblis-mt.* . 
module unload spack

# build main executable
module load cmake/3.21.4
export CRAYPE_LINK_TYPE='dynamic' # what is this

cmake -DCMAKE_CXX_COMPILER=CC .. # change compiler
make

module unload cmake

