#Yufan Xia 10.2.2021

#from cv2 import threshold
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

            
num_of_duplicate = 1
n_cores=48

class splitter48():#CVSplit class before data augmentation
    n_splits=5
    random_seed=100                     
    group_size=n_cores*10               

    def __init__(self, n_splits=5, group_size=n_cores*10, random_seed=0):#groupsize = n_cores * num_of_duplicate
        self.n_splits=n_splits
        self.group_size=group_size      
        return
                                        
    def split(self,X=None, y=None, groups=None):
        multiples = self.group_size#number of duplicates
        if X.shape[0]/multiples % 1 != 0:
            print("not a multiple of 48")
        n_lines = int((X.shape[0]/multiples))
        n_samples_per_set = n_lines//self.n_splits
        indices = np.arange(multiples*n_lines)
        all_indices = np.arange(X.shape[0])#1,2,3,...,n

        splits = []
        for i in range(0, self.n_splits):#Kfold, already shuffled when loading
            splits.append([list(set(all_indices) - set(indices[multiples*i*n_samples_per_set:multiples*(i+1)*n_samples_per_set])),
                          indices[multiples*i*n_samples_per_set:multiples*(i+1)*n_samples_per_set],])
        return splits
    
    def get_n_splits(self,X=None, y=None, groups=None):
        return self.n_splits

class cv_stratified():#CVSplit class before data augmentation
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


#abnormal trend
def trendline(data):
    order=1
    index=[i for i in range(1,len(data)+1)]
    coeffs = np.polyfit(index, list(data), order)
    slope = coeffs[-2]
    return float(slope)

def plot_time_core(data_row):
    plt.plot(np.arange(0,n_cores)+1,data_row[3:])
    plt.show()
    return

#load raw data in 51 columns
def load_raw_data(file_name, seed=0):
    data = pd.read_csv(file_name)
    data = data.dropna()
    if data.shape[1] != 3+n_cores:
        print("data shape not right")
    
    #use mean
    data=data.values
    for i in range(data.shape[0]//10):#calculate mean
        data[10*i:10*(i+1), 3:] = np.mean(data[10*i:10*(i+1), 3:], axis=0)
    data = data[::10]
    data= pd.DataFrame(data)
    

    #add number of iterations as a feature
    # iter_index = np.arange(0, data.shape[0])%num_of_duplicate
    # data.insert(3, 'iter_index', iter_index)

    # data = shuffle(data) #shuffle data to simplify the TTS and cross validation

    # data = shuffle_data(data, seed)# for 10 iterations
    return data

def shuffle_data(data, seed=0):
    slice_list = []
    for i in range(data.shape[0]//num_of_duplicate):
        slice_list.append(data.iloc[num_of_duplicate*i:num_of_duplicate*(i+1), :])
    random.seed(seed)
    random.shuffle(slice_list)#randomize 
    data_new = slice_list[0]
    for i in range(data.shape[0]//num_of_duplicate -1):
        data_new = pd.concat([data_new, slice_list[i+1]], axis=0)

    return data_new

# def expand_data(data):#with iter_number
#     data_nd = data.values    
#     ##Raw data to regression data
#     data_new_nd = np.zeros((48*data_nd.shape[0], 6))
#     for rid in range(0, data_nd.shape[0]):
#         for cid in range(4, data_nd.shape[1]):
#             point = np.array([[data_nd[rid, 0], data_nd[rid, 1], data_nd[rid, 2],data_nd[rid, 3], cid-2, data_nd[rid, cid]]])
#             data_new_nd[(rid)*48 + cid-4] = point
#     data_new_nd = data_new_nd.astype(int)
#     data_new = pd.DataFrame(data_new_nd)
#     data_new.columns = ['m', 'k', 'n', 'iter_index','ncores', 'time']
#     return data_new

def expand_data(data):#original without iter_number
    data_nd = data.values   
    ##Raw data to regression data
    data_new_nd = np.zeros((n_cores*data_nd.shape[0], 5))
    for rid in range(0, data_nd.shape[0]):
        for cid in range(3, data_nd.shape[1]):
            point = np.array([[data_nd[rid, 0], data_nd[rid, 1], data_nd[rid, 2], cid-2, data_nd[rid, cid]]])# number of cores starting from 1
            data_new_nd[(rid)*n_cores + cid-3] = point
    data_new_nd = data_new_nd.astype(int)
    data_new = pd.DataFrame(data_new_nd)
    data_new.columns = ['m', 'k', 'n', 'ncores', 'time']
    return data_new

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
    data_new['m*k*n']=data_new['m']*data_new['k']*data_new['n']
    data_new['mfootprint']=4*(data_new['m']*data_new['k']+data_new['m']*data_new['n']+data_new['k']*data_new['n'])
    data_new['m*k']=data_new['m']*data_new['k']#6
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
    time = data_new.pop('time')
    data_new.insert(data_new.shape[1],'time',time)
    
    #feature selection
#     cols = ['ncores', 'm*k*n', 'k*n', 'm*n', 'm*k*n/ncores']#Forward
#     cols = ['m', 'ncores', 'm*k', 'k*n', 'k*n/ncores', 'm*k*n/ncores']
    # cols =['m*k', 'k*n', 'm*n', 'n/ncores', 'm*k*n/ncores', 'mfootprint/ncores']#backward

    # cols.append('time')
    # data_new=data_new[cols]
    return data_new

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

def TTS(X, y,ratio=0.3, num_bins=30):
    cut = pd.cut(y, bins=num_bins, labels=range(num_bins))
    cut = np.array(cut, dtype=int)

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=ratio, random_state=42, stratify=cut)


    # divide = num_of_duplicate*round(data.shape[0]//num_of_duplicate*(1-ratio))
    # data_test = data.iloc[divide:]
    # data = data.iloc[:divide, :]

    # data = data.reset_index(drop=True)
    # data_test = data_test.reset_index(drop=True)

    return X_train, X_test, y_train, y_test


def get_dict_of_best_nt(raw_data):
    dict_ = dict()
    raw_data = raw_data.values
    for row in raw_data:
        row = row.tolist()
        index = row.index(min(row[3:])) -3 #get min time index of nt
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
    upper_tri = corr.where(np.triu(np.ones(corr.shape),k=1).astype(np.bool))
    to_drop = [column for column in upper_tri.columns if any(upper_tri[column] > corr_thres)]
    print("Dropped ",len(to_drop), "features: ",(to_drop))
    X = X.drop(to_drop, axis=1)


    return to_drop, X















##create pipeline###
def data_pipe(file_name, max_num_threads = 48, outlier_rem=False, threshold=-1, scaler=None, lamb=0.17, corr_thres=0.8, resamp=False, seed=0, do_trans=True):
    global n_cores
    n_cores = max_num_threads # change core as a global variable

    print("** Loading data")
    data= load_raw_data(file_name, seed=seed)

    # expand to 48 times
    print("** Expanding data")
    data=expand_data(data)
    # data_test=expand_data(data_test)

    # feature creation
    print("** Feature creating")
    data = feature_composition(data)
    # data_test = feature_composition(data_test)
    X,y=data.drop('time', axis=1), data['time']
    # X_test,y_test=data_test.drop('time', axis=1), data_test['time']#X,y,data
    cols = X.columns

    # Stratified train test split
    print("** Train test spliting")
    X, X_test, y, y_test = TTS(X, y, ratio=.3, num_bins=30)
    # print("Original", X_test.iloc[0, :])

    data_train_raw = None
    data_test_raw = None

    #resampling
    # if resamp:
    #     X, y = resampling(X, y, False)

    # feature transformation: include no info from test set, please note
    print("** Feature transforming")
    trans = None
    if do_trans:
        X, trans = Yeo_Johnson(X)
        X_test, _ = Yeo_Johnson(X_test, transformer= trans)
    # print("After Yeo", X_test[0, :])

    # feature scale # change to standardization
    print("** Feature Scaling")
    scaler,X = feature_scale(X)
    scaler,X_test = feature_scale(X_test, scaler)
    # print("After Scale", X_test.iloc[0, :])

    # order of the outlier removal is important
    print("** Removing outliers")
    if outlier_rem:
        original_len=X.shape[0]
        X,y=outlier_removal(X, y, threshold=threshold)
        now_len=X.shape[0]
        print('** '+str(original_len-now_len)+' Outliers removed.')
    else:
        print('** No outlier removal applied.')


    #Name features
    X.columns = cols
    X_test.columns = cols

    #feature selection
    print("** Selecting feature using feature importance")
    to_drop, X = feature_selection(X, y, corr_thres)
    X_test = X_test.drop(to_drop, axis=1)
    # if num_of_features < 17:
    #     sfs,X = feature_selection(X,y,num_of_features, direction='backward')
    #     print(sfs.support_)
    #     X_test=sfs.transform(X_test)   
    # print("After Drop", X_test.iloc[0, :])


    X = pd.DataFrame(X)                  
    X_test = pd.DataFrame(X_test)        

    cols = X_test.columns                
    X.columns = cols                     
    return X,y,X_test,y_test,scaler,trans,data_train_raw, data_test_raw, cols
                                         
def test_data_preprocess(file_name, max_num_threads = 48, trans = None, scaler=None, cols = None, seed=0):
    global n_cores
    n_cores = max_num_threads # change core as a global variable

    print("** Loading data")
    data= pd.read_csv(file_name)
    
    # expand to 48 times
    print("** Expanding data")
    data_nd = np.array(data)
    ##Raw data to regression data
    data_new_nd = np.zeros((n_cores*data_nd.shape[0], 4))
    for rid in range(0, data_nd.shape[0]):
        for cid in range(3, n_cores+3):
            point = np.array([[data_nd[rid, 0], data_nd[rid, 1], data_nd[rid, 2], cid-2]])# number of cores starting from 1
            data_new_nd[(rid)*n_cores + cid-3] = point
    data_new_nd = data_new_nd.astype(int)
    data_new = pd.DataFrame(data_new_nd)
    data_new.columns = ['m', 'k', 'n', 'ncores']
    data = data_new

    # feature creation
    print("** Feature creating")
    data_new = data
    data_new['m*k*n']=data_new['m']*data_new['k']*data_new['n']
    data_new['mfootprint']=4*(data_new['m']*data_new['k']+data_new['m']*data_new['n']+data_new['k']*data_new['n'])
    data_new['m*k']=data_new['m']*data_new['k']#6
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
    data = data_new
    cols_all = data.columns

    # feature transformation: include no info from test set, please note
    print("** Feature transforming")
    X_test, _ = Yeo_Johnson(data, transformer= trans)
    # print("After Yeo", X_test[0, :])

    # feature scale # change to standardization
    print("** Feature Scaling")
    scaler,X_test = feature_scale(X_test, scaler)
    # print("After Scale", X_test.iloc[0, :])

    #Name features
    X_test.columns = cols_all

    #feature selection
    print("** Selecting feature")
    # X_test = X_test.drop(to_drop, axis=1)
    X_test = pd.DataFrame(X_test, columns = cols)

    cols = X_test.columns
    return X_test











                                         
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
                            
                            
