# Unload modules
# module unload intel-compiler intel-mkl python python2 python3
# module unload python3
# Load module, always specify version number.
module unload python3
module unload intel-mkl
module load intel-mkl/2020.3.304
module load python3/3.9.2
 
#install dependent modules
# python3 -m pip install -v --no-binary :all: --user -r requirements.txt

module load intel-mkl/2019.3.199