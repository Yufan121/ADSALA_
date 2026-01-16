import numpy as np
import pandas as pd
import sys
plt_yes=True
try:
    import matplotlib.pyplot as plt
except ImportError:
    plt_yes=False
from scipy.stats import qmc



MAX_SIZE = 500e6 # 500 MB
NUM_POINTS = 1e6

def within_range(sample, operation, double=False):
    if double:
        dtype_size = 8
    else:
        dtype_size = 4

    if operation == 'gemm':
        m, n, k = sample.T
        return (m*k + k*n + m*n) * dtype_size < MAX_SIZE
    elif operation == 'symm':
        m, n = sample.T
        return (n*m + m*m +n*m) * dtype_size < MAX_SIZE
    elif operation == 'syrk':
        n, k = sample.T
        return (n*k + n*n) * dtype_size < MAX_SIZE
    elif operation == 'syr2k':
        n, k = sample.T
        return (2*n*k + n*n) * dtype_size < MAX_SIZE
    elif operation == 'trmm':
        m, n = sample.T
        return (n*m + m*m) * dtype_size < MAX_SIZE
    elif operation == 'trsm':
        m, n = sample.T
        return (n*m + m*m) * dtype_size < MAX_SIZE
    elif operation == 'gemv':
        m, n = sample.T
        return (n*m + m + n) * dtype_size < MAX_SIZE
    elif operation in ['trsv', 'syr']:
        n, = sample.T
        return (n*n + n) * dtype_size < MAX_SIZE
    elif operation in ['dot', 'axpy']:
        n, = sample.T
        return 2 * n * dtype_size < MAX_SIZE
    else:
        raise ValueError(f"Operation {operation} not supported")

def generate_halton_sequence(ndims, num_points):
    sampler = qmc.Halton(d=ndims)
    _ = sampler.fast_forward(20) # skip the first 20 points
    
    print(f"Generating {num_points} points for {ndims} dimensions")
    sample = sampler.random(n=num_points) # of shape (num_points, ndims)
    return sample

def scale_sequence(ndims=3, sample=None, operation='gemm', double=False): 
    max_scaled = 200000
    if ndims == 1: 
        max_scaled = 1e8
        print(MAX_SIZE, max_scaled)
    elif ndims == 2:
        max_scaled = 1e6
        print(MAX_SIZE, max_scaled)
    elif ndims == 3:
        max_scaled = 1e5
        print(MAX_SIZE, max_scaled)

    l_bounds = np.zeros(ndims)
    u_bounds = np.full(ndims, max_scaled)
    sample = qmc.scale(sample, l_bounds, u_bounds).astype(int)
    sample = sample[within_range(sample, operation, double).T]
    return sample

def main():
    # operations = ['gemm', 'symm', 'gemv', 'syr', 'trsv', 'dot', 'axpy']
    # operations = ['symm', 'trsv', 'dot']
    operations = ['syrk', 'syr2k', 'trmm', 'trsm']
    # operations = ['gemm']
    # operations = ['dot']
    # operations = ['symm']
    for_testing = True
    double = False # set this to True if you want to use double precision

    
    ndims_dict = {'gemm': 3, 'symm': 2, 'syrk': 2,'syr2k': 2,'trmm': 2,'trsm': 2, 'gemv': 2, 'syr': 1, 'trsv': 1, 'dot': 1, 'axpy': 1}

    for operation in operations:
        print(f"Generating {operation} domain")
        
        global MAX_SIZE, NUM_POINTS
    
        # adjust the number of points based on the operation
        NUM_POINTS_ = NUM_POINTS
        ndims = ndims_dict[operation]
        if operation == 'dot' and double == False:
            NUM_POINTS_ = NUM_POINTS * 1/600
            if for_testing:
                MAX_SIZE = MAX_SIZE * 1/10
                NUM_POINTS_ = NUM_POINTS_ * 10
        if operation == 'trsv' and double == False:
            NUM_POINTS_ = NUM_POINTS * 10
        if operation == 'symm':
            if double == False:
                NUM_POINTS_ = NUM_POINTS * 3.5 
            else:
                NUM_POINTS_ = NUM_POINTS * 3.5 * 2
        if operation == 'gemm' and double == True:
            NUM_POINTS_ = NUM_POINTS * 1.1
        if operation == 'syrk':
            if double == False:
                NUM_POINTS_ = NUM_POINTS * 3.5 * 0.5
            else:
                NUM_POINTS_ = NUM_POINTS * 3.5
        if operation == 'syr2k':
            if double == False:
                NUM_POINTS_ = NUM_POINTS * 3.5 * 0.9
            else:
                NUM_POINTS_ = NUM_POINTS * 3.5 * 0.9 * 2
        if operation == 'trmm':
            if double == False:
                NUM_POINTS_ = NUM_POINTS * 3.5 * 0.5
            else: 
                NUM_POINTS_ = NUM_POINTS * 3.5 * 0.5 * 2
        if operation == 'trsm':
            if double == False:
                NUM_POINTS_ = NUM_POINTS * 3.5 * 0.5
            else:
                NUM_POINTS_ = NUM_POINTS * 3.5 * 0.5 * 2
            
        if for_testing:
            NUM_POINTS_ = NUM_POINTS_ / 10
            
        sample = generate_halton_sequence(ndims, int(NUM_POINTS_))

        sample = scale_sequence(ndims=ndims,sample=sample,operation=operation,double=double)

        print(f"Generated {len(sample)} points for {operation}")

        # Other code...
        
        if for_testing:
            path = "test_data/"
        else:
            path = "data/"

        if plt_yes:
            if ndims == 1:
                plt.hist(sample, bins=100)
            else:
                plt.scatter(sample[:, 0], sample[:, 1], s=1)
            plt.savefig(f"{path}domain{MAX_SIZE/1e6:.0f}MB-{len(sample)}points_{operation}_double_{double}.png")
            plt.show()
        else:
            print("matplotlib not installed, skipping plot")
            print(pd.DataFrame(sample).describe())
            
        df = pd.DataFrame(sample)
        if for_testing:
            df.to_csv(f"{path}domain{MAX_SIZE/1e6:.0f}MB-{len(sample)}points_{operation}_{double}.d", index=None)
        else:
            df.to_csv(f"{path}domain{MAX_SIZE/1e6:.0f}MB-{len(sample)}points_{operation}_{double}.csv", index=None)
        if not for_testing:
            df = df.loc[df.index.repeat(10)]
            df.to_csv(f"{path}domain{MAX_SIZE/1e6:.0f}MB-{len(sample)}points_{operation}_{double}.d", index=None)

if __name__ == "__main__":
    main()
