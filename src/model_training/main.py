import sys
sys.path.append(r"resources")
from load_data import *
import pandas as pd
import numpy as np
from sklearn.metrics import mean_squared_error
import matplotlib.pyplot as plt
from math import sqrt
from sklearn.metrics import r2_score
import time
from xgboost import XGBRegressor as XGBR
from sklearn.model_selection import RandomizedSearchCV
from scipy.stats import uniform
import configparser

#Flags
domain_file = sys.argv[1]
# Gadi_sdot_500M, extract sdot 
subroutine = domain_file.split('_')[1]
num_threads = int(sys.argv[2])


Hyper_tune=True
n_iter=500
Save_model_file=True
Learning_curve=False
Write_config=True
GPU=False
np.random.seed(1234)

#Read and proprocess data
X,y,X_test,y_test, scaler, trans, data_train, data_test, cols, whole_cols = data_pipe(domain_file, max_num_threads = num_threads, outlier_rem=False, threshold=-1.5, lamb=0.17, resamp=False, corr_thres=0.85, seed=1000)


# print stat of data
print( "*************Train set stat: ****************")
print("Train set size: " + str(X.shape[0]/num_threads))
print("Test set size: " + str(X_test.shape[0]/num_threads))
print("Number of features: " + str(X.shape[1]))
print("") 

#Declare booster
booster = None
if GPU == True:
    booster = XGBR(learning_rate=0.1, n_estimators=100, max_depth=7, min_child_weight=0.2, seed=0,
                             subsample=0.7, colsample_bytree=0.7, gamma=7.84, reg_alpha=9.63, reg_lambda=5.36, tree_method='gpu_hist', gpu_id=0)
else:
    booster = XGBR(learning_rate=0.1, n_estimators=100, max_depth=7, min_child_weight=0.2, seed=0,
                             subsample=0.7, colsample_bytree=0.7, gamma=7.84, reg_alpha=9.63, reg_lambda=5.36)

# Save Config to File
if Write_config:
    # xgb_subroutine.ini
    config_file =  "xgb_" + subroutine + ".ini"
    config = configparser.ConfigParser()
    
    data_mean_= scaler.mean_.tolist()
    data_std_= scaler.scale_.tolist()
    yeo_lambda = trans.lambdas_.tolist()
    
    # # keep only selected features for the above three lists
    # data_mean_ = [data_mean_[i] for i in range(len(data_mean_)) if cols[i] == 1]
    # data_std_ = [data_std_[i] for i in range(len(data_std_)) if cols[i] == 1]
    # yeo_lambda = [yeo_lambda[i] for i in range(len(yeo_lambda)) if cols[i] == 1]

    # whole_cols=['m', 'k', 'n', 'ncores', 'm*k*n', 'mfootprint', 'm*k', 'k*n', 'm*n',
    #    'm/ncores', 'k/ncores', 'n/ncores', 'm*k/ncores', 'k*n/ncores',
    #    'm*n/ncores', 'm*k*n/ncores', 'mfootprint/ncores']

    print(whole_cols)
    feature_list = whole_cols[:] #save full list, then get mask using current list
    for i in range(len(whole_cols)):
        if whole_cols[i] in cols:
            whole_cols[i] = 1
        else:
            whole_cols[i] = 0

    config['DEFAULT'] = {'data_mean_': data_mean_,
                        'data_std_': data_std_,
                        'yeo_lambda': yeo_lambda,
                        'feature_num': len(cols),
                        'feature_mask': whole_cols,
                        'feature_list': feature_list
}
    with open(config_file, 'w') as configfile:
        config.write(configfile) # write config file
    print("** Config created")

#Hyper training
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

if Hyper_tune:
    cv = cv_stratified(n_splits=6, n_bins=50)
    clf = RandomizedSearchCV(booster, distributions, random_state=10,n_iter=n_iter,cv=cv, n_jobs=-2, verbose=10)
    search = clf.fit(X, y)
    print(search.best_params_)
    booster=clf.best_estimator_

#Learning Curve
if Learning_curve:
    plot_learning_curve(DT, 'Learning Curve', X, y, ylim=None, cv=cv_stratified(n_splits=6, n_bins=50),
                        train_sizes=np.linspace(.1, 1.0, 10), n_jobs=-2)




# Training and Evaluation
print("** Train test spliting")

# Fit the booster to the training data
t = time.time_ns()
booster.fit(X, y)
t = time.time_ns() - t

# Print training time
print("Data set size: " + str((X.shape[0] + X_test.shape[0]) / 480))
print("Training time: " + str(t / 1e6) + "ms")

# Predict on test and train data
t = time.time_ns()
y_predict_test = booster.predict(X_test)
y_predict_train = booster.predict(X)
t = time.time_ns() - t

# Print testing time
print("Testing time: " + str(t / 1e6) + "ms")
print("")

# Print r2 score and RMSE score
print("Train set r2 Score: " + str(r2_score(y, y_predict_train)))
print("Test set r2 Score: " + str(r2_score(y_test, y_predict_test)))
print("RMSE Score Train: " + str(sqrt(mean_squared_error(y_predict_train, y))))
print("RMSE Score Test: " + str(sqrt(mean_squared_error(y_predict_test, y_test))))
print(y_predict_test)
print(y_test)
# correct prediction rate and blind guess rate
# print(y.describe())
# print(y_test.describe())
print( "Speedup")
speedup_plot(y_test, y_predict_test)

# Save the model to file
if Save_model_file:
    # xgb_subroutine.model
    booster.save_model("xgb_" + subroutine + ".model")
    print("** Model saved to file")
