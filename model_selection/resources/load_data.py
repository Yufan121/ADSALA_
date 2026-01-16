#Yufan Xia 10.2.2021

import pandas as pd
import numpy as np  
import matplotlib.pyplot as plt
from pandas.plotting import scatter_matrix
from sklearn.preprocessing import MinMaxScaler
from sklearn.preprocessing import StandardScaler
from sklearn import preprocessing

from sklearn.neighbors import LocalOutlierFactor
import random       
from sklearn.model_selection import learning_curve
from xgboost import XGBRegressor as XGBR
from sklearn.feature_selection import SequentialFeatureSelector
from sklearn.model_selection import train_test_split
from sklearn.model_selection import StratifiedKFold


num_of_duplicate = 10
n_cores = 96 # will be overwritten by main.py
ndims = 3

class splitterGroup():
    """
    A class for splitting data into 48 sets for cross-validation.

    Attributes:
    n_splits (int): The number of splits to create.
    random_seed (int): The random seed for shuffling the data.
    group_size (int): The size of each group.

    Methods:
    split(X, y, groups): Splits the data into 48 sets for cross-validation.
    get_n_splits(X, y, groups): Returns the number of splits.
    """
    n_splits=5
    random_seed=100                     
    group_size=n_cores*10               

    def __init__(self, n_splits=5, group_size=n_cores*num_of_duplicate, random_seed=0):
        self.n_splits=n_splits
        self.group_size=group_size      
        return
                                        
    def split(self,X=None, y=None, groups=None):
        multiples = self.group_size
        if X.shape[0]/multiples % 1 != 0:
            print("Warning: data size not divisible by group size, dropping last few rows.")
        n_lines = int((X.shape[0]/multiples))
        n_samples_per_set = n_lines//self.n_splits
        indices = np.arange(multiples*n_lines)
        all_indices = np.arange(X.shape[0])

        splits = []
        for i in range(0, self.n_splits):
            splits.append([list(set(all_indices) - set(indices[multiples*i*n_samples_per_set:multiples*(i+1)*n_samples_per_set])),
                          indices[multiples*i*n_samples_per_set:multiples*(i+1)*n_samples_per_set],])
        return splits
    
    def get_n_splits(self,X=None, y=None, groups=None):
        return self.n_splits

class cv_stratified():
    """
    A class for stratified cross-validation.

    Attributes:
    n_splits (int): The number of splits to create.
    random_seed (int): The random seed for shuffling the data.
    n_bins (int): The number of bins to use for stratification.

    Methods:
    split(X, y, groups): Splits the data into stratified sets for cross-validation.
    get_n_splits(X, y, groups): Returns the number of splits.
    """
    n_splits=5
    random_seed=100
    n_bins=30               

    def __init__(self, n_splits=5, n_bins=30, random_seed=0):
        self.n_splits=n_splits
        self.n_bins=n_bins      
        self.random_seed=random_seed      
        return

    def split(self,X=None, y=None, groups=None):
        cut = pd.cut(y, bins=self.n_bins, labels=range(self.n_bins))
        cut = np.array(cut, dtype=int)
        sss = StratifiedKFold(n_splits=self.n_splits)
        return sss.split(X, cut)
    
    def get_n_splits(self,X=None, y=None, groups=None):
        return self.n_splits


def trendline(data):
    """
    Calculates the slope of a trendline for a given set of data.

    Args:
    data (numpy.ndarray): The data to calculate the trendline slope for.

    Returns:
    float: The slope of the trendline.
    """
    order=1
    index=[i for i in range(1,len(data)+1)]
    coeffs = np.polyfit(index, list(data), order)
    slope = coeffs[-2]
    return float(slope)

def plot_time_core(data_row):
    """
    Plots the data for a single core over time.

    Args:
    data_row (pandas.Series): The data for a single core over time.
    """
    plt.plot(np.arange(0,n_cores)+1,data_row[n_dims:])
    plt.show()
    return

def load_raw_data(file_name, seed=0):
    """
    Load raw data from a csv file and preprocess it for model training.

    Args:
    file_name (str): The path to the csv file containing the raw data.
    seed (int, optional): The random seed for shuffling the data. Defaults to 0.

    Returns:
    pandas.DataFrame: The preprocessed data for model training.
    """
    
    # read data from csv file
    data = pd.read_csv(file_name)
    
    # drop any rows with zero entry
    data = data.loc[~(data==0).any(axis=1)]
    data = data.dropna()

    if data.shape[1] > ndims+n_cores:
        print("data shape not right, cropping to ndims(%d)+n_cores"%(ndims))
        data = data.iloc[:, :3+n_cores]
    
    data=data.values
    for i in range(data.shape[0]//10): # average every 10 rows
        data[10*i:10*(i+1), :] = np.mean(data[10*i:10*(i+1), :], axis=0)
    data = data[::10]
    data= pd.DataFrame(data)

    return data

# def shuffle_data(data, seed=0):
#     """
#     Shuffles the input data by slicing it into smaller chunks, shuffling those chunks, and then concatenating them back together.

#     Args:
#         data (pandas.DataFrame): The input data to be shuffled.
#         seed (int, optional): The random seed to use for shuffling. Defaults to 0.

#     Returns:
#         pandas.DataFrame: The shuffled data.
#     """
#     num_of_duplicate = 1
#     slice_list = []
#     for i in range(data.shape[0]//num_of_duplicate):
#         slice_list.append(data.iloc[num_of_duplicate*i:num_of_duplicate*(i+1), :])
#     random.seed(seed)
#     random.shuffle(slice_list)
#     data_new = slice_list[0]
#     for i in range(data.shape[0]//num_of_duplicate -1):
#         data_new = pd.concat([data_new, slice_list[i+1]], axis=0)

#     return data_new
#     data_new.columns = ['m', 'k', 'n', 'iter_index','ncores', 'time']
#     return data_new

# def expand_data(data): #original without iter_number
#     data_nd = data.values   
#     ##Raw data to regression data
#     data_new_nd = np.zeros((n_cores*data_nd.shape[0], 5))
#     print(data_new_nd.shape)
#     for rid in range(0, data_nd.shape[0]):
#         for cid in range(3, data_nd.shape[1]):
#             point = np.array([[data_nd[rid, 0], data_nd[rid, 1], data_nd[rid, 2], cid-2, data_nd[rid, cid]]])# number of cores starting from 1
#             print(rid)
#             data_new_nd[(rid)*n_cores + cid-3] = point
#     data_new_nd = data_new_nd.astype(int)
#     data_new = pd.DataFrame(data_new_nd)
#     data_new.columns = ['m', 'k', 'n', 'ncores', 'time']
#     return data_new

def expand_data(data):
    """
    Expands the given data by melting it into a long format and renaming the columns.

    Args:
        data (pandas.DataFrame): The data to be expanded.

    Returns:
        pandas.DataFrame: The expanded data in a long format.
    """
    
    # print("before melt: ", data)
    # assign names, only the first n_dims columns has names
    if data.shape[1] == 3 + n_cores:
        data.columns = ['m', 'k', 'n'] + list(range(1, n_cores+1))
        data_melted = data.melt(id_vars=['m', 'k', 'n'], var_name='ncores', value_name='time') # 
        # sort by m, k, n, ncores
        data_melted = data_melted.sort_values(by=['m', 'k', 'n', 'ncores'])
    elif data.shape[1] == 2 + n_cores:
        data.columns = ['m', 'n'] + list(range(1, n_cores+1))
        data_melted = data.melt(id_vars=['m', 'n'], var_name='ncores', value_name='time')
        # sort by m, n, ncores
        data_melted = data_melted.sort_values(by=['m', 'n', 'ncores'])
    elif data.shape[1] == 1 + n_cores:
        data.columns = ['n'] + list(range(1, n_cores+1))
        data_melted = data.melt(id_vars=['n'], var_name='ncores', value_name='time')
        # sort by n, ncores
        data_melted = data_melted.sort_values(by=['n', 'ncores'])
    else: 
        print(data.shape[1])
        raise ValueError("Input data must have at least 1 column.")
    # print("after melt: ", data_melted)
        
    return data_melted


# def expand_data(data):#for Kmeans
#     data_nd = data.values 
#     ##Raw data to regression data
#     data_new_nd = np.zeros((48*data_nd.shape[0], 8))
#     for rid in range(0, data_nd.shape[0]):
#         for cid in range(6, data_nd.shape[1]):
#             point = np.array([[data_nd[rid, 0], data_nd[rid, 1], data_nd[rid, 2], data_nd[rid, 3], data_nd[rid, 4], data_nd[rid, 5], cid-2, data_nd[rid, cid]]])
#             data_new_nd[(rid)*48 + cid-6] = point
#     data_new_nd = data_new_nd.astype(int)
#     data_new = pd.DataFrame(data_new_nd)
#     data_new.columns = ['m', 'k', 'n', 'cluster 0', 'cluster 1', 'cluster 2','ncores', 'time']
#     return data_new


def feature_composition(data_new):
    """
    Computes new features based on the input data and returns a new DataFrame with the original and new features.

    Args:
        data_new (pandas.DataFrame): The input data, with columns 'm', 'k', 'n', 'ncores', and 'time'.

    Returns:
        pandas.DataFrame: A new DataFrame with the original and new features, and the 'time' column moved to the end.
    """
    # detect the dimensions of the input data
    if 'm' in data_new.columns and 'k' in data_new.columns and 'n' in data_new.columns:
        dims = 'mkn'
    elif 'm' in data_new.columns and 'n' in data_new.columns:
        dims = 'mn'
    elif 'n' in data_new.columns:
        dims = 'n'
    else:
        raise ValueError("Input data must have at least 'n' column.")

    usable_cols = []
    # compute new features based on the input data
    if dims == 'mkn':
        data_new['m*k*n']=data_new['m']*data_new['k']*data_new['n']
        data_new['mfootprint']=4*(data_new['m']*data_new['k']+data_new['m']*data_new['n']+data_new['k']*data_new['n'])
        data_new['m*k']=data_new['m']*data_new['k'] #6
        data_new['k*n']=data_new['k']*data_new['n']
        data_new['m*n']=data_new['m']*data_new['n']
        data_new['m/ncores']=data_new['m']/data_new['ncores']
        data_new['k/ncores']=data_new['k']/data_new['ncores']
        data_new['n/ncores']=data_new['n']/data_new['ncores']
        data_new['m*k/ncores']=data_new['m*k']/data_new['ncores']
        data_new['k*n/ncores']=data_new['k*n']/data_new['ncores']
        data_new['m*n/ncores']=data_new['m*n']/data_new['ncores']
        data_new['m*k*n/ncores']=data_new['m*k*n']/data_new['ncores']
        data_new['mfootprint/ncores']=data_new['mfootprint']/data_new['ncores']
        usable_cols = ['m', 'k', 'n', 'ncores', 'm*k*n', 'mfootprint', 'm*k', 'k*n', 'm*n', 'm/ncores', 'k/ncores', 'n/ncores', 'm*k/ncores', 'k*n/ncores', 'm*n/ncores', 'm*k*n/ncores', 'mfootprint/ncores']
        
        # ensure the first num_threas rows' last x column doest not equal, otherwise raise error
        if data_new.iloc[0, -1] == data_new.iloc[1, -1] or data_new.iloc[0, -2] == data_new.iloc[1, -2] or data_new.iloc[0, -3] == data_new.iloc[1, -3] or data_new.iloc[0, -4] == data_new.iloc[1, -4] or data_new.iloc[0, -5] == data_new.iloc[1, -5] or data_new.iloc[0, -6] == data_new.iloc[1, -6] or data_new.iloc[0, -7] == data_new.iloc[1, -7] or data_new.iloc[0, -8] == data_new.iloc[1, -8]:
            raise ValueError("First two rows' last column are the same, please check data.")

    elif dims == 'mn':
        data_new['m*n']=data_new['m']*data_new['n']
        data_new['m/ncores']=data_new['m']/data_new['ncores']
        data_new['n/ncores']=data_new['n']/data_new['ncores']
        data_new['m*n/ncores']=data_new['m*n']/data_new['ncores']
        usable_cols = ['m', 'n', 'ncores', 'm*n', 'm/ncores', 'n/ncores', 'm*n/ncores']
        
        # ensure the first num_threas rows' last x column doest not equal, otherwise raise error
        if data_new.iloc[0, -1] == data_new.iloc[1, -1] or data_new.iloc[0, -2] == data_new.iloc[1, -2] or data_new.iloc[0, -3] == data_new.iloc[1, -3]:
            print(data_new.iloc[:n_cores])
            raise ValueError("First two rows' last column are the same, please check data.")

    else:
        data_new['n/ncores']=data_new['n']/data_new['ncores']
        usable_cols = ['n', 'ncores', 'n/ncores']
        
        # ensure the first num_threas rows' last column doest not equal, otherwise raise error
        if data_new.iloc[0, -1] == data_new.iloc[1, -1]:
            raise ValueError("First two rows' last column are the same, please check data.")
        

    # move the 'time' column to the end
    time = data_new.pop('time')
    data_new.insert(data_new.shape[1],'time',time)
    
    # apply only the new columns that are usable

    # # apply only the new columns that are usable
    # data_new = data_new[usable_cols + ['time']]

    return data_new, usable_cols

def feature_scale(X, scaler=None):#create scaler for train, use existing for test
    if scaler==None:
        scaler = StandardScaler()
        scaler.fit(X)
        print("** No scaler, creating one.")
    data_scaled_np = scaler.transform(X)
    data_scaled = pd.DataFrame(data_scaled_np)
    return scaler, data_scaled    
                                      
def box_cox(X, lmbda):
    if lmbda==0:                      
        return np.log(X)
    X = (X**lmbda - 1) / lmbda        
    return X  

def Yeo_Johnson(X, transformer=None):
    if transformer==None:
        transformer = preprocessing.PowerTransformer(method='yeo-johnson', standardize=False)
        transformer.fit(X)
        print("** No scaler, creating one.")
    X = transformer.transform(X)

    return X, transformer                       
                        
                                      
def outlier_removal(X, y, threshold=-1):
    lof = LocalOutlierFactor(n_neighbors=30, algorithm= 'kd_tree', n_jobs=-2)
    lof.fit(np.array(X))
    outlier_score= lof.negative_outlier_factor_
    #implement a 5% removal mechanism.
    outlier_non_index = outlier_score>threshold
    X=X.loc[outlier_non_index, :]
    y=y.loc[outlier_non_index]
    return X, y
                                      
def resampling(data, under_samp=True):

    data = np.array(data)
    additional = data[data[:, -1] > 5e4]
    for j in range(0,15):
        data = np.concatenate([data,additional], axis=0)
    data = pd.DataFrame(data)

    # data = np.array(data)
    # for i in range(1,5):
    #     additional = data[data[:, -1] > i*5e3]
    #     for j in range(0,5):
    #         data = np.concatenate([data,additional], axis=0)
    # data = pd.DataFrame(data)


#     for i in range(1,5):
#         additional = data.loc[data.time > i*5e3]
# #         additional = additional.loc[additional.time <= (i+1)*2.5e3]
#         for j in range(0,i):
#             data = pd.concat([data,additional], axis=0)

    # data = smogn.smoter(data = data, y = 'time', under_samp=under_samp, samp_method = "extreme")


#     X=data.drop('time', axis=1)
#     y=data['time']

    data.columns = ['m', 'k', 'n', 'ncores', 'time']

    return data



def TTS(X, y, ratio=0.3, num_bins=30):
    """
    Splits the input data into training and testing sets using stratified sampling based on the target variable.

    Args:
        X (array-like): The input features.
        y (array-like): The target variable.
        ratio (float, optional): The proportion of the dataset to include in the test split. Defaults to 0.3.
        num_bins (int, optional): The number of bins to use for stratified sampling. Defaults to 30.

    Returns:
        tuple: A tuple containing the training and testing sets for the input data.
    """
    while True:
        try:
            # print(X)
            
            num_raw_points = X.shape[0]/n_cores
            print("num_raw_points: ", num_raw_points)
            
            # compress  every n_cores rows into one row, concatenate them together
            X_groups = X.values
            y_groups = y.values
            X_groups = X_groups.reshape((int(num_raw_points), n_cores*X_groups.shape[1]))
            y_groups = y_groups.reshape((int(num_raw_points), n_cores))
            X_groups = pd.DataFrame(X_groups)
            y_groups = pd.DataFrame(y_groups)
            # print("X_groups.shape: ", X_groups.shape)
            # print("y_groups.shape: ", y_groups.shape)
            
            
            # get cut by averaging y_groups every row by pd.cut
            cut = pd.cut(y_groups.mean(axis=1), bins=num_bins, labels=range(num_bins))
            cut = np.array(cut, dtype=int)

            
            # split into training and testing sets (group)
            X_train, X_test, y_train, y_test = train_test_split(X_groups, y_groups, test_size=ratio, random_state=4342, stratify=cut)
            
            # expand X_train and X_test
            X_train = X_train.values.reshape((X_train.shape[0]*n_cores, X_train.shape[1]//n_cores))
            X_test = X_test.values.reshape((X_test.shape[0]*n_cores, X_test.shape[1]//n_cores))

            # expand y_train and y_test
            y_train = y_train.values.reshape((y_train.shape[0]*n_cores)).reshape((y_train.shape[0]*n_cores))
            y_test = y_test.values.reshape((y_test.shape[0]*n_cores)).reshape((y_test.shape[0]*n_cores))
            
            
            # print X_test
            # print("X_test:")
            # print(X_test[:n_cores])
            
            # cut = pd.cut(y, bins=num_bins, labels=range(num_bins)) #q  
            # cut = np.array(cut, dtype=int)

            # X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=ratio, random_state=42, stratify=cut)
            
            # return 
            return X_train, X_test, y_train, y_test
        except ValueError as e:
            if 'The least populated class in y has only 1 member' in str(e):
                num_bins = num_bins // 2
                print('Reducing number of bins to ' + str(num_bins))
            else:
                raise e


def get_dict_of_best_nt(raw_data):
    dict_ = dict()
    raw_data = raw_data.values
    for row in raw_data:
        row = row.tolist()
        index = row.index(min(row[3:])) - n_dims #get min time index of nt
        dict_[row[0], row[1], row[2]] = index
    return dict_
    
# def feature_selection(X, y, num_of_features=17, direction='forward'):
#     sfs = SequentialFeatureSelector(LinearRegression(), n_features_to_select=num_of_features, cv=splitter48(5),direction=direction, n_jobs=-2)
#     X_new = sfs.fit_transform(X,y)

#     return sfs, X_new

# def feature_selection(X, y, num_of_features=17, direction='forward'):

#     XGB = XGBR(learning_rate=0.1, n_estimators=100, max_depth=7, min_child_weight=0.2, seed=0, subsample=0.7, colsample_bytree=0.7, gamma=7.84, reg_alpha=9.63, reg_lambda=5.36)
#     XGB.fit(X,y)
#     importance = XGB.feature_importances_
#     threshold = np.sort(importance)[-num_of_features]
#     bool_matrix = importance >= threshold

#     X = np.array(X)
#     X = X[:, bool_matrix]

#     return bool_matrix, X

def feature_selection(X, y, corr_thres=0.8):

    corr = X.corr().abs()
    upper_tri = corr.where(np.triu(np.ones(corr.shape),k=1).astype(np.bool_))
    to_drop = [column for column in upper_tri.columns if any(upper_tri[column] > corr_thres)]
    print("Dropped ",len(to_drop), "features: ",(to_drop))
    X = X.drop(to_drop, axis=1)


    return to_drop, X















##create pipeline###

def data_pipe(file_name, max_num_threads=48, outlier_rem=False, threshold=-1, scaler=None, lamb=0.17, corr_thres=0.8, resamp=False, seed=0):
    # Set number of cores as a global variable
    global n_cores
    n_cores = max_num_threads

    # Load raw data
    print("** Loading data")
    data = load_raw_data(file_name, seed=seed)

    # Expand data to 48 times
    print("** Expanding data")
    data = expand_data(data)

    # Create new features
    print("** Feature creating")
    data, all_cols = feature_composition(data)
    X, y = data.drop('time', axis=1), data['time']
    cols = X.columns
    print(type(X))
    print(type(y))
    
    # Split data into training and testing sets using stratified sampling
    print("** Train test splitting")
    X, X_test, y, y_test = TTS(X, y, ratio=.15, num_bins=30)
    # print(X.shape, X_test.shape, y.shape, y_test.shape)
    
    # Transform features using Yeo-Johnson transformation
    print("** Feature transforming")
    X, trans = Yeo_Johnson(X)
    X_test, _ = Yeo_Johnson(X_test, transformer=trans)

    # Scale features using StandardScaler
    print("** Feature scaling")
    scaler, X = feature_scale(X)
    scaler, X_test = feature_scale(X_test, scaler)
    

    # Remove outliers from data
    print("** Removing outliers")
    # convert to dataframes
    X = pd.DataFrame(X)
    X_test = pd.DataFrame(X_test)
    y = pd.Series(y)
    y_test = pd.Series(y_test)
    if outlier_rem:
        original_len = X.shape[0]
        X, y = outlier_removal(X, y, threshold=threshold)
        now_len = X.shape[0]
        print('** ' + str(original_len - now_len) + ' Outliers removed.')
    else:
        print('** No outlier removal applied.')

    # Name features
    X.columns = cols
    X_test.columns = cols

    # Select features using feature importance
    print("** Selecting features using feature importance")
    to_drop, X = feature_selection(X, y, corr_thres)
    X_test = X_test.drop(to_drop, axis=1)

    # Convert X and X_test to pandas dataframes
    X = pd.DataFrame(X)
    X_test = pd.DataFrame(X_test)

    # Set column names for X_test
    cols = X_test.columns
    X.columns = cols

    # Return data
    return X, y, X_test, y_test, scaler, trans, None, None, cols, all_cols                                                                                




def test_data_preprocess(file_name, max_num_threads = 48, trans = None, scaler=None, cols = None, seed=0): # only features are needed, use pre-determined params for transform and scale 
    
    # Set number of cores as a global variable
    global n_cores
    n_cores = max_num_threads

    # Load raw data
    print("** Loading data")
    data = load_raw_data(file_name, seed=seed)
    

    # Expand data to 48 times
    print("** Expanding data")
    data = expand_data(data)
    
    # make sure data has 2 columns
    if data.shape[1] < 3 or data.shape[1] > 5: # 3 for n, 4 for m, n, 5 for m, k, n, time not dropped yet
        print(data.shape[1])
        raise ValueError("Input data must have at least 2 columns.")

    # Create new features
    print("** Feature creating")
    data, all_cols = feature_composition(data)
    X_test, _ = data.drop('time', axis=1), data['time'] # keep only features
    cols_all = X_test.columns
    # print(type(X))
    # print(type(y))
    
    # # Split data into training and testing sets using stratified sampling
    # print("** Train test splitting")
    # X, X_test, y, y_test = TTS(X, y, ratio=.15, num_bins=30)
    # # print(X.shape, X_test.shape, y.shape, y_test.shape)
    
    print("** Feature transforming")
    X_test, _ = Yeo_Johnson(X_test, transformer= trans)
    # print("After Yeo", X_test[0, :])

    # feature scale # change to standardization
    print("** Feature Scaling")
    scaler,X_test = feature_scale(X_test, scaler)
    # print("After Scale", X_test.iloc[0, :])
    

    # # Remove outliers from data
    # print("** Removing outliers")
    # # convert to dataframes
    # X = pd.DataFrame(X)
    # X_test = pd.DataFrame(X_test)
    # y = pd.Series(y)
    # y_test = pd.Series(y_test)
    # if outlier_rem:
    #     original_len = X.shape[0]
    #     X, y = outlier_removal(X, y, threshold=threshold)
    #     now_len = X.shape[0]
    #     print('** ' + str(original_len - now_len) + ' Outliers removed.')
    # else:
    #     print('** No outlier removal applied.')

    # Name features
    X_test.columns = cols_all

    #feature selection
    print("** Selecting feature")
    # X_test = X_test.drop(to_drop, axis=1)
    X_test = pd.DataFrame(X_test, columns = cols)


    # Return data
    return X_test

    # return X, y, X_test, y_test, scaler, trans, None, None, cols, all_cols                                                                                






                                         
# get a intuitive performance metrics    
def correct_pred_rate(y_real, y_pred):   
    range_ = y_real.shape[0]//n_cores#n of tests (other than tasks, wholes)
    count_correct = 0
    loss_list = []
    best_list = []
    pred_list = []
    index_list = []
    pred_sum = 0
    best_sum = 0
    for i in range(range_):
        i = n_cores*i
        index_pred = np.argmin(y_pred[i: i+n_cores])
        index_real = np.argmin(y_real[i: i+n_cores])
        index_list.append(index_real)
        if index_pred==index_real:#add 1 to correct count
            count_correct+=1
        pred_sum += y_real[i+index_pred]
        best_sum += y_real[i+index_real]
        loss_list.append((y_real[i+index_pred]-y_real[i+index_real])/y_real[i+index_real])
        best_list.append(y_real[i+index_real])#append best time
        pred_list.append(y_real[i+index_pred])#append pred time
    sum_ratio = pred_sum / best_sum

    return (count_correct/range_), np.mean(loss_list), sum_ratio, best_list, pred_list, index_list

def blind_guess_rate(y_real):
    range_ = y_real.shape[0]//n_cores#n of tests (other than tasks, wholes)
    count_correct = 0
    loss_list = []
    best_list = []
    pred_list = []
    pred_sum = 0
    best_sum = 0

    for i in range(range_):
        i = n_cores*i

        index = np.argmin(y_real[i: i+n_cores])
        pred_sum += y_real[i+n_cores-1]
        best_sum += y_real[i+index]

        loss_list.append((y_real[i+n_cores-1]-y_real[i+index])/y_real[i+index])
        best_list.append(y_real[i+index])#append best time
        pred_list.append(y_real[i+n_cores-1])#append pred time
#         print((y_real[i+n_cores-1]-y_real[i+index])/y_real[i+index])                                 
        if index==n_cores-1:                                                                           
            count_correct+=1                                                                           
    sum_ratio = pred_sum / best_sum                                                                    
                                                                                                       
    return (count_correct/range_), np.mean(loss_list), sum_ratio, best_list, pred_list                 
                                                                                                       
                                                                                                       
def plot_learning_curve(estimator, title, X, y, axes=None, ylim=None, cv=None,                         
                        n_jobs=None, train_sizes=np.linspace(.1, 1.0, 5), save_file="learning_curve.pdf"):
    if axes is None:
        _, axes = plt.subplots(1, 3, figsize=(20, 5))                                                  
                                                                                                       
    axes[0].set_title(title)                                                                           
    if ylim is not None:                                                                               
        axes[0].set_ylim(*ylim)                                                                        
    axes[0].set_xlabel("Training examples")                                                            
    axes[0].set_ylabel("Score")                                                                        
                                                                                                       
    train_sizes, train_scores, test_scores, fit_times, _ = learning_curve(estimator, X, y, cv=cv, n_jobs=n_jobs,
                       train_sizes=train_sizes,
                       return_times=True)
    train_scores_mean = np.mean(train_scores, axis=1)
    train_scores_std = np.std(train_scores, axis=1)
    test_scores_mean = np.mean(test_scores, axis=1)
    test_scores_std = np.std(test_scores, axis=1)
    fit_times_mean = np.mean(fit_times, axis=1)
    fit_times_std = np.std(fit_times, axis=1)
    print(test_scores)
    # Plot learning curve
    axes[0].grid()
    axes[0].fill_between(train_sizes, train_scores_mean - train_scores_std,
                         train_scores_mean + train_scores_std, alpha=0.1,
                         color="r")
    axes[0].fill_between(train_sizes, test_scores_mean - test_scores_std,
                         test_scores_mean + test_scores_std, alpha=0.1,
                         color="g")
    axes[0].plot(train_sizes, train_scores_mean, 'o-', color="r",
                 label="Training score")
    axes[0].plot(train_sizes, test_scores_mean, 'o-', color="g",
                 label="Cross-validation score")
    axes[0].legend(loc="best")
                              
    # Plot n_samples vs fit_times
    axes[1].grid()            
    axes[1].plot(train_sizes, fit_times_mean, 'o-')
    axes[1].fill_between(train_sizes, fit_times_mean - fit_times_std,
                         fit_times_mean + fit_times_std, alpha=0.1)
    axes[1].set_xlabel("Training examples")
    axes[1].set_ylabel("fit_times")
    axes[1].set_title("Scalability of the model")
                              
    # Plot fit_time vs score
    axes[2].grid()          
    axes[2].plot(fit_times_mean, test_scores_mean, 'o-')
    axes[2].fill_between(fit_times_mean, test_scores_mean - test_scores_std,
                         test_scores_mean + test_scores_std, alpha=0.1)
    axes[2].set_xlabel("fit_times")
    axes[2].set_ylabel("Score")
    axes[2].set_title("Performance of the model")
                            
    plt.savefig(save_file)
    return axes              
                            
                            
def speedup_plot(y_test, y_predict_test): 
    test_correct_rate,test_potential_improvement,sum_ratio,test_best, test_pred, index_list = correct_pred_rate(y_test,y_predict_test)
    test_correct_rate_48,test_potential_improvement_48,sum_ratio_48, test_best_48, test_pred_48 = blind_guess_rate(y_test)

    # print(test_correct_rate)
    
    #average/overall speedup
    test_pred_np = np.array(test_pred)#best time by ML algo
    test_48_np = np.array(test_pred_48)#time by max out threads
    test_best_np = np.array(test_best)#time by actual best
    # test_pred_np=test_pred_np+2.0e3 #add algo time

    # predicted
    speedup = test_48_np / test_pred_np
    overall_speedup = test_48_np.sum() / test_pred_np.sum()

    print("***Model Speedup***")
    print(speedup.mean())
    print(overall_speedup)


    # actual
    speedup_actual = test_48_np / test_best_np
    overall_speedup_actual = test_48_np.sum() / test_best_np.sum()
     

    speedup_best = test_48_np / test_best_np # actual potential
    overall_speedup_best = test_48_np.sum() / test_best_np.sum()
                                                    
    print("*** Best Speedup Possible ***")                                   
    print(speedup_best.mean())
    print(overall_speedup_best)

    # mean_of_slow = test_pred_np[speedup<=1].mean()
    # mean_of_fast = test_pred_np[speedup>1].mean()


    # # print(mean_of_slow-mean_of_fast)#Check! All good!
    # print(mean_of_slow)
    # print(mean_of_fast)