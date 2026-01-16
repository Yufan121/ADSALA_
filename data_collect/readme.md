# ADSALA GEMM single/double precision

### Code Overview

This piece of code is for collecting the training data of ADSALA GEMM program. 

Here are some details:

1. Generate-domain is for domain generation; Main program is for doing the experiments according to the domains
2. For use across different machines, we use a library manager to manage the available libraries. The current version is configured for AMD BLIS library. 
3. To make convenience of your running, I created useful bash scripts, run them according to the instructions below then the program will do the job.

### Compile

This project uses Cmake. 

- To compile, go to the build folder then `cmake ..` and `make` (delete `CMakeCache.txt` and `make clean` if needed, but do not delete other files in the `build` folder)
- After the compile is complete, we will get two executables: `generate-domain` and `main`. Run `sh move_executable.sh` will transfer these two executables to the place where they are needed (`real X00M` which represents different problem size).

### To run the program:
- Enter directory `real X00M`
- Execute `sh run.sh`, this will do:
  - Generate test domains (Experiment sizes can be adjusted in `run.sh`)
  - Split into subjobs
  - Submit subjobs (this command is `cd domains \n sh batch.sh` will need to be uncomment if you really wish to submit the jobs)
- After the job is finished, run `combine_domains.sh` to collect result in `batch_domains.csv`

### We are currently running on `real 500M` range

To modify domain range, precision (float/double) and number of samples, just alter the params in command `./generate-domain  0 100 42354 z__.d 4 0 ` in `real X00M/run.sh`. 

`./generate-domain  0 100 42354 z__.d 4 0 ` generates domains from 0-100M, with random seed 42354, store in file z__.d and with sample magnitude of 4 (bigger value gives larger number of samples), the last number 0 means single precision. To use double precision, change the precision param from 0 to 1; an additional change is needed: change `single` in line 55 of `real X00M/real.sh` into `double`.

### Changes from Calum's version

This work is a modified version from Calum Snowdon's work.

- Modified from Calum version
- switched to rapid csv for data collection
- 10 times inside one iteration
