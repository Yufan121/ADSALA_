# Library Implementation

### Outline
- `BLAS` is the folder for integrating BLAS package, it calls `predictor` at runtime
- `predictor` is the folder for runtim model evaluation
- `test` folder is for testing purposes, it link to ADSALA library

### Run the program
- `mkdir build` then `cd build`
- `cmake ..` then `make`
- Go to the `build/test`, `test` is the test program
- Use `batch.sh` to submit to compute node
- Model file, config file and test domains csv file should all be in the same directory

### Caveats:

- need a data folder inside the `build/test` folder