# %matplotlib notebook                    
### Get evaluation time by test data, apply on new test data to get estimated speedup
import sys                                
sys.path.append(r"resources")             
from load_data import *                   
import pandas as pd                       
import numpy as np
# %matplotlib inline
import matplotlib.pyplot as plt
from sklearn.metrics import mean_squared_error
from math import sqrt
from sklearn.metrics import r2_score      
import time                               
# import lightgbm as lgb                    
from sklearn.linear_model import LinearRegression
from sklearn.svm import SVR
from sklearn import linear_model
from sklearn import tree
from sklearn.ensemble import RandomForestRegressor
from xgboost import XGBRegressor as XGBR
from sklearn.ensemble import AdaBoostRegressor
from sklearn.preprocessing import PolynomialFeatures
from sklearn.linear_model import ElasticNet
from sklearn.neighbors import KNeighborsRegressor

from sklearn.model_selection import RandomizedSearchCV
from scipy.stats import uniform
from sklearn.model_selection import cross_validate
import joblib



np.random.seed(150)
domain_file = sys.argv[1]
# extract no-directory name
domain_file_nodir = domain_file.split('/')[-1]
# extract subroutine name
subroutine = domain_file.split('_')[1]
num_threads = int(sys.argv[2])

# define metrics
metrics = []
# metrics.append(['train_rmse', 'test_rmse', 'ideal_speedup_mean', 'ideal_speedup_overall', 'train_time(e-6)', 'eval_time(e-6)', 'estimated_speedup_mean', 'estimated_speedup_overall'])

# hyper-tune parameters
n_iter = 200
verbose = 0

# get training data and all
# X,y,X_test,y_test, scaler, trans, data_train, data_test, cols=data_pipe(domain_file, max_num_threads = num_threads, outlier_rem=True, threshold=-1.5, lamb=0.17, resamp=False, corr_thres=0.80, seed=1000, do_trans=True)
X,y,X_test,y_test, scaler, trans, data_train, data_test, cols, whole_cols = data_pipe(domain_file, max_num_threads = num_threads, outlier_rem=False, threshold=-1.5, lamb=0.17, resamp=False, corr_thres=0.85, seed=1000)

# define average csv reader
def average_file_read(csv_path):
    data = pd.read_csv(csv_path)
    
    # drop any rows with zero entry
    data = data.loc[~(data==0).any(axis=1)]
    data = data.dropna()
    
    data = np.array(data)
    average = np.zeros((data.shape[0]//10, data.shape[1]))
    
    # get average
    for i in range(10):
        # print(data.shape)
        average += data[i::10]  
    average = average / 10
    return average

# define test data preprocess
def test(model=None):
    # ******************************** measure eval time
    
    # print model name with obvious separation
    print('********************************')
    print(model.__class__.__name__)
    
    train_list =[]
    test_list=[]

    
    # this loop evaluates average evaluation time
    for i in range(5):
    # timing, training and predicting
        # model training
        t=time.time_ns()
        model.fit(X,y)
        t=time.time_ns()-t
        
        train_list.append(t)

        # model evaluation
        t=time.time_ns()
        y_predict_test = model.predict(X_test)
        t=time.time_ns()-t

        test_list.append(t)

        # prediction of y
        y_predict_train = model.predict(X)

        # RMSE
        # print y and y_predict_train
        # print("y and y_predict_train is: ")
        # print(y[:10])
        # print(y_predict_train[:10])
        RMSE_train = sqrt(mean_squared_error(y, y_predict_train))
        RMSE_test = sqrt(mean_squared_error(y_predict_test, y_test))
        
        # show RMSE
        if i==0:
            print("Train RMSE Score: "+ str(RMSE_train))
            print("Test RMSE Score: "+ str(RMSE_test))
    
    # get average time
    train_time = (np.array(train_list)/1e3).mean()
    eval_time = (np.array(test_list)/1e3).mean()/(X_test.shape[0]/num_threads) # for one raw point 
    print("eval_time", eval_time)

    # ******************************** get estimated speedup

    # testing domain name
    # domain_file_ = './Setonix_domains.csv' # use new domain for testing
    # domain_file_ = "/home/659/yx7184/Project ADSALA/Barca repo/ADSALA/build/library_frame/test/domain500MB-122points_symm_False.d"
    domain_file_ = domain_file
    
    # get testing features
    X_test_extern = test_data_preprocess(domain_file_, trans = trans, scaler = scaler, cols = cols, max_num_threads = num_threads, seed=1000)
    # X_test_extern = X_test
    # print(X_test_extern[:num_threads])
    # print(X_test_extern.shape)
    
    # get actual time from training data
    time_actual = average_file_read(domain_file)[:, -num_threads:] # all time without features, shape (n_points, num_threads)
    time_predicted = model.predict(X_test_extern)
    # get best num_thread
    time_predicted_by_domain = time_predicted.reshape(-1, num_threads) # all time predicted
    index_predicted = np.array(time_predicted_by_domain)[:].argmin(axis=1)  # get all best num_thread
    # print(index_predicted)
    
    # get estimated time duration
    time_ADSALA = np.zeros((index_predicted.shape[0], 1))
    time_ADSALA_ideal = np.zeros((index_predicted.shape[0], 1))
    # add eval time to get estimated time duration
    for i in range(time_actual.shape[0]):
        time_ADSALA[i] = time_actual[i, index_predicted[i]] + eval_time # get estimated time duration
    for i in range(time_actual.shape[0]):
        time_ADSALA_ideal[i] = time_actual[i, index_predicted[i]] # get estimated time duration
        
    time_original = time_actual[:, -1].reshape(-1,1) # get no-ADSALA time duration
    
    # get speedup
    speedup = time_original / time_ADSALA
    speedup_ideal = time_original / time_ADSALA_ideal
    speedup_o = time_original.sum() / time_ADSALA.sum()
    speedup_o_ideal = time_original.sum() / time_ADSALA_ideal.sum()

    # mean speedup and mean speedup_ideal
    mean_speedup = speedup.mean()
    mean_speedup_ideal = speedup_ideal.mean()

    print(mean_speedup)
    # ********************************


    return [RMSE_train, RMSE_test, mean_speedup_ideal, speedup_o_ideal, train_time, eval_time, mean_speedup, speedup_o]


def save_partial_metrics(metrics, file_name=None):
    
    # need only once, save whole metrics list to csv file
    df = pd.DataFrame(metrics)
    # keep only 1 decimal
    df = df.round(2)
    # set column names
    df.columns = ['model_name', 'train_rmse', 'test_rmse', 'ideal_speedup_mean', 'ideal_speedup_overall', 'train_time(e-6)', 'eval_time(e-6)', 'estimated_speedup_mean', 'estimated_speedup_overall']

    # append if already exists
    df.to_csv(domain_file_nodir+'_metrics.csv', header=True, index=False)




################################################################################################################################################
################################################################################################################################################

# # # LR 1   # ********************************

DT = LinearRegression(fit_intercept=True)

new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
print(domain_file_nodir)
# joblib.dump(DT, './gadi_models/LinearRegression_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)



# # ElasticNet 2   # ********************************
DT = ElasticNet(alpha=0, l1_ratio=0.1,fit_intercept=True, max_iter=int(1e5))
distributions = dict(alpha=uniform(loc=1, scale=0.9),
                     l1_ratio=uniform(loc=0.5, scale=.49)
                    )

clf = RandomizedSearchCV(DT, distributions, random_state=1324,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2, verbose=verbose)
search = clf.fit(X, y)
print(search.best_params_)
DT=clf.best_estimator_

new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
# joblib.dump(DT, './gadi_models/ElasticNet_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)



# Bayesian 3   # ********************************
DT = linear_model.BayesianRidge()
distributions = dict(alpha_1=uniform(loc=.5, scale=.49),
                     alpha_2=uniform(loc=.5, scale=.49),
                     lambda_1=uniform(loc=.5, scale=.49),
                     lambda_2=uniform(loc=.5, scale=.49)
                    )

clf = RandomizedSearchCV(DT, distributions, random_state=1324,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2)
search = clf.fit(X, y)
print(search.best_params_)
DT=clf.best_estimator_

new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
# joblib.dump(DT, './gadi_models/BayesianRidge_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)


# DT  4  # ********************************
DT = tree.DecisionTreeRegressor(max_depth=7)
distributions = dict(max_depth=[5,6,7,8,9,10,11,12,13,14,15],
                    # min_samples_split=uniform(loc=.5, scale=.4),
                    min_samples_leaf=[2,3,4,5,6,7,8,9,10],
                    # ccp_alpha=uniform(loc=5, scale=4),
                     max_leaf_nodes=[2,3,4,5,6,7,8,9,10,11,12,15,20,25,30]
                    )

clf = RandomizedSearchCV(DT, distributions, random_state=1231,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2)
search = clf.fit(X, y)
print(search.best_params_)
DT=clf.best_estimator_

new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
# joblib.dump(DT, './gadi_models/DecisionTreeRegressor_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)



#RandomForestRegressor  5  # ********************************
DT = RandomForestRegressor(max_depth=5,n_estimators=100, n_jobs=1)

from sklearn.model_selection import RandomizedSearchCV
# Number of trees in random forest
n_estimators = [20,30,40,50,60,70,80,90,100,150,200,300,400]
# Number of features to consider at every split
max_features = ['auto', 'sqrt']
# Maximum number of levels in tree
max_depth = [int(x) for x in range(3, 15)]
max_depth.append(None)
# Minimum number of samples required to split a node
min_samples_split = [2, 5, 10]
# Minimum number of samples required at each leaf node
min_samples_leaf = [1, 2, 4]
# Method of selecting samples for training each tree
bootstrap = [True, False]
# Create the random grid
distributions = {'n_estimators': n_estimators,
               'max_features': max_features,
               'max_depth': max_depth,
               'min_samples_split': min_samples_split,
               'min_samples_leaf': min_samples_leaf,
               'bootstrap': bootstrap}

clf = RandomizedSearchCV(DT, distributions, random_state=1324,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2)
# Fit the random search model
search = clf.fit(X, y)
print(search.best_params_)
DT=clf.best_estimator_

new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
# joblib.dump(DT, './gadi_models/RandomForestRegressor_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)

#AdaBoost  6  # ********************************       
DT= AdaBoostRegressor(n_estimators=100)#AdaBoost回归
distributions = dict(learning_rate=uniform(loc=.25, scale=.24),
                     n_estimators=[20,30,40,50,60,70,80,90,100,150,200,300,400,600],
                    )

clf = RandomizedSearchCV(DT, distributions, random_state=1324,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2)
search = clf.fit(X, y)    
print(search.best_params_)
DT=clf.best_estimator_    

new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
# joblib.dump(DT, './gadi_models/AdaBoostRegressor_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)


#KNN     7  # ********************************    
DT = KNeighborsRegressor(n_neighbors=3, algorithm='ball_tree')
distributions = dict(weights=['uniform', 'distance'],
                     leaf_size=[10,20,25,30,35,40,50],
#                      algorithm=['auto', 'ball_tree', 'kd_tree', 'brute'],
                     n_neighbors = [2,3,4,5,8,10,15,20,30]
                    )

clf = RandomizedSearchCV(DT, distributions, random_state=1324,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2)
search = clf.fit(X, y)
print(search.best_params_)
DT=clf.best_estimator_

new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
# joblib.dump(DT, './gadi_models/KNeighborsRegressor_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)

# # SVR    8  # ********************************    

# DT = SVR(kernel='linear',max_iter=1000) # convergence issue
# distributions = dict(coef0=uniform(loc=10, scale=9.9),
# #                      shrinking=[True, False],
#                     # weights=['scale', 'auto'],
#                     kernel=['linear', 'poly', 'rbf', 'sigmoid'],
#                      C=uniform(loc=100, scale=99.9),
#                     epsilon=uniform(loc=5, scale=4.9)
#                     )

# clf = RandomizedSearchCV(DT, distributions, random_state=1324,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2)
# search = clf.fit(X, y)
# print(search.best_params_)
# DT=clf.best_estimator_


# new_m = test(DT)
# # prepend model name to new_m
# new_m.insert(0, DT.__class__.__name__)
# metrics.append(new_m)
# save partial metrics
# save_partial_metrics(metrics)

# # XGBoost    9  # ********************************    
# DT = XGBR()
# DT.load_model('0001.model')
DT = XGBR(learning_rate=0.1, n_estimators=100, max_depth=7, min_child_weight=0.2, seed=0,
                             subsample=0.7, colsample_bytree=0.7, gamma=7.84, reg_alpha=9.63, reg_lambda=5.36)
distributions = dict(learning_rate=uniform(loc=.15, scale=.14),
                     max_depth=[5,6,7,8,9,10,11,12,13,14,15],
                     min_child_weight=uniform(loc=.5, scale=.49),
                     subsample=uniform(loc=.5, scale=.49),
                     n_estimators=[50, 100, 150, 200, 250, 300],
                     reg_alpha=uniform(loc=30, scale=29.9),
                     reg_lambda=uniform(loc=30, scale=29.9),
                     colsample_bytree=uniform(loc=.5, scale=.49),
                     gamma=uniform(loc=3, scale=2.9)
                    )
clf = RandomizedSearchCV(DT, distributions, random_state=1324,n_iter=n_iter,cv=cv_stratified(n_splits=6, n_bins=30),n_jobs=-2)
search = clf.fit(X, y)
print(search.best_params_)
DT=clf.best_estimator_


new_m = test(DT)
# prepend model name to new_m
new_m.insert(0, DT.__class__.__name__)
metrics.append(new_m)
# save model with specific name
# joblib.dump(DT, './gadi_models/XGBRegressor_'+domain_file_nodir+'.pkl')
# save partial metrics
save_partial_metrics(metrics)









