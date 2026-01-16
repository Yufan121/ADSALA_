/*!
 * Copyright 2019 XGBoost contributors
 *
 * \file c-api-demo.c
 * \brief A simple example of using xgboost C API.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <xgboost/c_api.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "config-read.h"
#include <time.h>

/**
 * @brief safe guard for xgboost related calls
 * 
 */
#define safe_xgboost(call) {                                            \
int err = (call);                                                       \
if (err != 0) {                                                         \
  fprintf(stderr, "%s:%d: error in %s: %s\n", __FILE__, __LINE__, #call, XGBGetLastError()); \
  exit(1);                                                              \
}                                                                       \
}

/**
 * @brief maximum number of threads, feature numbers
 * 
 */
int max_num_threads = 48*2; // to be read from config file
// const int max_num_threads = 128*2;
int max_feature = 17; // to be read from config file
int feature_num = 5; // to be read from config file
// #define feature_num 5
// #define max_feature 17

/**
 * @brief struct for info of data preprocessing
 * 
 */
struct PreprocessInfo
{
  /* data */
  double* data_mean_;
  double* data_std_;
  double* range;
  double* yeo_lambda;
  int* feature_mask;
};

/**
 * @brief struct for memory of last prediction
 * 
 */
struct MemoryStruct
{
  /* data */
  int m;
  int k;
  int n;
  int n_core_choice;
};

//Public variables
static BoosterHandle booster;
// char feature_list[feature_num][15]; //check if the list in python and c match
static struct PreprocessInfo ConfigInfo;
static int* list_feature_index;
static struct MemoryStruct memory;


/**
 * @brief create new feature value from existing m,k,n,nthreads
 * possible upgrade: reuse existing result: e.g. mkn on top of mk and n
 * 
 * @param feature_index No. of feature index
 * @param m 
 * @param k 
 * @param n 
 * @param ncores 
 * @return double 
 */
double feature_create_d3(int feature_index, double m, double k, double n, double ncores) {
  switch (feature_index) // a total of 17 features
  {
  case 0:
    return m;
  case 1:
    return k;
  case 2:
    return n;
  case 3:
    return ncores;
  case 4:
    return m*k*n;
  case 5:
    return 4*(m*k+m*n+k*n);//mfootprint
  case 6:
    return m*k;
  case 7:
    return k*n;
  case 8:
    return m*n;
  case 9:
    return m/ncores;
  case 10:
    return k/ncores;
  case 11:
    return n/ncores;
  case 12:
    return m*k/ncores;//kn/ncores
  case 13:
    return k*n/ncores;//mn/ncores
  case 14:
    return m*n/ncores;//kn/ncores
  case 15:
    return (m*k*n)/ncores;//mkn/ncores
  case 16:
    return (4*(m*k+m*n+k*n))/ncores;//mfootprint/ncores
  default:
    break;
  }
  return 1.0;
}



/**
 * @brief create new feature value from existing m,k,n,nthreads
 * possible upgrade: reuse existing result: e.g. mkn on top of mk and n
 * 
 * @param feature_index No. of feature index
 * @param m 
 * @param n 
 * @param ncores 
 * @return double 
 */
double feature_create_d2(int feature_index, double m, double n, double ncores) {
  switch (feature_index) // a total of 7 features
  {
  case 0:
    return m;
  case 1:
    return n;
  case 2:
    return ncores;
  case 3:
    return m*n;
  case 4:
    return m/ncores;
  case 5:
    return n/ncores;
  case 6:
    return m*n/ncores;//kn/ncores
  default:
    break;
  }
  return 1.0;
}




/**
 * @brief create new feature value from existing m,k,n,nthreads
 * possible upgrade: reuse existing result: e.g. mkn on top of mk and n
 * 
 * @param feature_index No. of feature index
 * @param n 
 * @param ncores 
 * @return double 
 */
double feature_create_d1(int feature_index, double n, double ncores) {
  switch (feature_index) // a total of 3 features
  {
  case 0:
    return n;
  case 1:
    return ncores;
  case 2:
    return n/ncores;
  default:
    break;
  }
  return 1.0;
}




/**
 * @brief get value after Yeo-Johnson transformation (** currently neglecting x<0 senario **)
 * 
 * @param x input
 * @return double output
 */
double yeo_johnson(int f_index, double x) {
  return ConfigInfo.yeo_lambda[f_index] == 0? log(x+1): (pow(x+1, ConfigInfo.yeo_lambda[f_index]) - 1) / ConfigInfo.yeo_lambda[f_index];// log and pow return double precision
}




/**
 * @brief Create the atrributes for predictions (size = (num_threads, num_features))
 * 
 * @param input_data pre-declared attribute data matrix
 * @param m 
 * @param k 
 * @param n 
 */
void data_load_preprocess_d3(float** input_data, int m, int k, int n) {

  // 1. Generate data for prediction (Data generation)(we can allocate this space beforhand)
  // 2. Combine features to match the training data (Feature engineering)
  // 3. Transform, normalize remaining features to match the training data (Data transformation)
  // static double input_data[max_num_threads * feature_num];//change to float to speedup

  *input_data = (float *)malloc(feature_num*max_num_threads*sizeof(float));//allocate space
  int index;

  for (int i = 0; i < max_num_threads; i++) {
    // printf("Index: %d \n", i);
      for (int j = 0; j < feature_num; j++) {
        index = list_feature_index[j];
        (*input_data)[feature_num*i+j] = (yeo_johnson(index, feature_create_d3(index, m, k, n, i+1) )-ConfigInfo.data_mean_[index]) / ConfigInfo.data_std_[index];//can speed up by cancelling parameter transfer// i+1 prevents n_cores be 0
      }
      // printf("\n");
  }
  // return input_data;
}



/**
 * @brief Create the atrributes for predictions (size = (num_threads, num_features))
 * 
 * @param input_data pre-declared attribute data matrix
 * @param m 
 * @param n 
 */
void data_load_preprocess_d2(float** input_data, int m, int n) {

  // 1. Generate data for prediction (Data generation)(we can allocate this space beforhand)
  // 2. Combine features to match the training data (Feature engineering)
  // 3. Transform, normalize remaining features to match the training data (Data transformation)
  // static double input_data[max_num_threads * feature_num];//change to float to speedup

  *input_data = (float *)malloc(feature_num*max_num_threads*sizeof(float));//allocate space
  int index;

  

  for (int i = 0; i < max_num_threads; i++) {
    // printf("Index: %d \n", i);
      for (int j = 0; j < feature_num; j++) {
        index = list_feature_index[j];
        (*input_data)[feature_num*i+j] = (yeo_johnson(index, feature_create_d2(index, m, n, i+1) )-ConfigInfo.data_mean_[index]) / ConfigInfo.data_std_[index];//can speed up by cancelling parameter transfer// i+1 prevents n_cores be 0
      }
      // printf("\n");
  }
  // return input_data;
}





/**
 * @brief Create the atrributes for predictions (size = (num_threads, num_features))
 * 
 * @param input_data pre-declared attribute data matrix
 * @param n 
 */
void data_load_preprocess_d1(float** input_data, int n) {

  // 1. Generate data for prediction (Data generation)(we can allocate this space beforhand)
  // 2. Combine features to match the training data (Feature engineering)
  // 3. Transform, normalize remaining features to match the training data (Data transformation)
  // static double input_data[max_num_threads * feature_num];//change to float to speedup

  *input_data = (float *)malloc(feature_num*max_num_threads*sizeof(float));//allocate space
  int index;

  for (int i = 0; i < max_num_threads; i++) {
    // printf("Index: %d \n", i);
      for (int j = 0; j < feature_num; j++) {
        index = list_feature_index[j];
        (*input_data)[feature_num*i+j] = (yeo_johnson(index, feature_create_d1(index, n, i+1) )-ConfigInfo.data_mean_[index]) / ConfigInfo.data_std_[index];//can speed up by cancelling parameter transfer// i+1 prevents n_cores be 0
        // print the input data
        // printf("(yeo_johnson(index, feature_create_d3(index, 0, 0, n, i+1) )-ConfigInfo.data_mean_[index]) = %f\n", (yeo_johnson(index, feature_create_d3(index, 0, 0, n, i+1) )-ConfigInfo.data_mean_[index]));
        // printf("ConfigInfo.data_std_[index] = %f\n", ConfigInfo.data_std_[index]);
        // printf("CHECK:  %f\n", (*input_data)[feature_num*i+j]);

      }
      // printf("\n");
  }
  // return input_data;
}



/**
 * @brief Load the config for data-preprocessiong (e.g. min-max scaler, box-cox lambda .etc)
 * 
 * @return int Config is save to the global variable, a struct
 */
int load_config(char* config_name){ //NOTICE: double precision scale info is cut to float precision, change to double if needed
  
  // initialize memory
  ConfigInfo.data_mean_ = (double*)malloc(max_feature * sizeof(double));
  ConfigInfo.data_std_ = (double*)malloc(max_feature * sizeof(double));
  ConfigInfo.range = (double*)malloc(max_feature * sizeof(double));
  ConfigInfo.yeo_lambda = (double*)malloc(max_feature * sizeof(double));
  ConfigInfo.feature_mask = (int*)malloc(max_feature * sizeof(int));

  // Don't forget to check if malloc was successful
  if (ConfigInfo.data_mean_ == NULL || ConfigInfo.data_std_ == NULL || ConfigInfo.range == NULL || ConfigInfo.yeo_lambda == NULL || ConfigInfo.feature_mask == NULL) {
      // Handle allocation failure...
      printf("Error: malloc failed in load_config\n");
      return 1;
  }

  printf("config_name: %s\n", config_name);

  // read max_num_threads, max_feature, feature_num
  max_num_threads = (int)GetIniKeyInt("DEFAULT", "max_num_threads", config_name);
  max_feature = (int)GetIniKeyInt("DEFAULT", "max_feature", config_name);
  feature_num = (int)GetIniKeyInt("DEFAULT", "feature_num", config_name);

  // printf("max_num_threads: %d\n", max_num_threads);
  // printf("max_feature: %d\n", max_feature);
  // printf("feature_num: %d\n", feature_num);

  int* intpointer = String2IntAarray(GetIniKeyString("DEFAULT", "feature_mask", config_name));
  for(int loop = 0; loop < max_feature; loop++) {
      ConfigInfo.feature_mask[loop] = intpointer[loop];
  }
  // printf("feature_mask: %d\n", ConfigInfo.feature_mask[0]);

  double* floatpointer = String2DoubleAarray(GetIniKeyString("DEFAULT", "data_mean_", config_name));
  for(int loop = 0; loop < max_feature; loop++) {
      ConfigInfo.data_mean_[loop] = floatpointer[loop];
  }
  // printf("data_mean_: %f\n", ConfigInfo.data_mean_[0]);

  floatpointer = String2DoubleAarray(GetIniKeyString("DEFAULT", "data_std_", config_name));
  for(int loop = 0; loop < max_feature; loop++) {
      ConfigInfo.data_std_[loop] = floatpointer[loop];
  }
  // printf("data_std_: %f\n", ConfigInfo.data_std_[0]);

  floatpointer = String2DoubleAarray(GetIniKeyString("DEFAULT", "yeo_lambda", config_name));
  for(int loop = 0; loop < max_feature; loop++) {
      ConfigInfo.yeo_lambda[loop] = floatpointer[loop];
  }
  // printf("yeo_lambda: %f\n", ConfigInfo.yeo_lambda[0]);

  //compute the feature_index_list
  int index = 0;
  list_feature_index = (int*)malloc(feature_num * sizeof(int));
  for(int i = 0; i < max_feature; i++) {
    if (ConfigInfo.feature_mask[i] == 1) {
      list_feature_index[index++] = i;
    } 
    // printf("i list_feature_index: %d %d\n", i, index);
  }

  //compute the range list
  for(int i = 0; i < max_feature; i++) {
    ConfigInfo.range[i] = ConfigInfo.data_std_[i] - ConfigInfo.data_mean_[i];
  }



  // //check correctness for feature_mask
  // for(int i = 0; i < max_feature; i++) {
  //   printf("feature_mask: %d\n", ConfigInfo.feature_mask[i]);
  // }

  // // check datamean and datastd
  // for(int i = 0; i < max_feature; i++) {
  //   printf("data_mean_: %f\n", ConfigInfo.data_mean_[i]);
  // }

  // for(int i = 0; i < max_feature; i++) {
  //   printf("data_std_: %f\n", ConfigInfo.data_std_[i]);
  // }

  // // // check correctness
  // // for (int i = 0; i < feature_num; i++) {
  // //   printf("OK: %d %d\n", i, list_feature_index[i]);
  // // }

  // // For check correctness
  // printf("Read from Config: %f \n", ConfigInfo.yeo_lambda);
  // for (int i = 0; i < max_feature; i++) {
  //   printf("Read from Config: %f \n", ConfigInfo.data_std_[i]);
  // }

	// printf("%s\n", GetIniKeyString("DEFAULT", "feature_list", config_name));
  return 0;
}




/**
 * @brief Load the model from model file of xgboost, save to global variable
 * 
 * @return int 
 */
int load_model(char* model_name){

  int silent = 0;
  //initialize memory
  memory.m = 0; memory.k = 0; memory.n = 0; memory.n_core_choice=1;

  // load the data
  DMatrixHandle dtrain, dtest;
  safe_xgboost(XGDMatrixCreateFromFile("./data/agaricus.txt.train", silent, &dtrain));
  safe_xgboost(XGDMatrixCreateFromFile("./data/agaricus.txt.test", silent, &dtest));

  // create the booster
  DMatrixHandle eval_dmats[2] = {dtrain, dtest};
  safe_xgboost(XGBoosterCreate(eval_dmats, 2, &booster));

  //Yufan: save model to file
  // safe_xgboost(XGBoosterSaveModel	(	booster, "model.model" ));
  //Yufan: load the model from file
  safe_xgboost(XGBoosterLoadModel	(	booster, model_name ));

  // get feature num
  bst_ulong num_feature = 0;
  // safe_xgboost(XGBoosterGetNumFeature(booster, &num_feature));
  // printf("num_feature: %lu\n", (unsigned long)(num_feature));


  safe_xgboost(XGDMatrixFree(dtrain));
  safe_xgboost(XGDMatrixFree(dtest));
  
  // printf("model successfully loaded\n");

  return 0;
}



int find_min_index(const float* out_result, int size) {
    float min = 3.402823466e38;
    int best_core = 0;
    for (int i = 0; i < size; i++) {
        if (out_result[i] < min) {
            min = out_result[i];
            best_core = i;
        }
    }
    return best_core + 1;
}


// Define the function pointer
typedef void (*ModelFunc)(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result);



/* XGB evaluation unit */

void xgb_model(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result) {
  DMatrixHandle dmat;
  bst_ulong bst_out_len = 0;
  safe_xgboost(XGDMatrixCreateFromMat(data_input, max_num_threads, feature_num, 0.0, &dmat));
  safe_xgboost(XGBoosterPredict(booster, dmat, 0, 0, 0, &bst_out_len, out_result));
  safe_xgboost(XGDMatrixFree(dmat));
  *out_len = (int)(bst_out_len); // convert bst_ulong to int
}

// /* LR evaluation unit */

// void lr_model(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result) {
//   *out_len = max_num_threads;
//   //TODO
//   *out_result = data_input;
// }

// /* BayesianLR evaluation unit */
// void bayesian_lr_model(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result) {
//   *out_len = max_num_threads;
//   //TODO
//   *out_result = data_input;
// }

// /* Random Forest evaluation unit */
// void random_forest_model(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result) {
//   *out_len = max_num_threads;
//   //TODO
//   *out_result = data_input;
// }



/**
 * @brief Given m,k,n, Get the optimum num cores from the model and input attributes. Calls upon all the above functions
 * 
 * @param m 
 * @param k 
 * @param n 
 * @return int 
 */
int get_optimum_num_cores_d3(int m, int k, int n, ModelFunc model_func) {
  if (memory.m == m && memory.k == k && memory.n == n)
    return memory.n_core_choice;
  else {
    memory.m = m; memory.k = k; memory.n = n;
  }

  float* data_input;
  data_load_preprocess_d3(&data_input, m, k, n);

  int out_len = 0;
  const float* out_result = NULL;
  model_func(data_input, max_num_threads, feature_num, &out_len, &out_result);

  free(data_input);

  memory.n_core_choice = find_min_index(out_result, out_len);

  return memory.n_core_choice;
}



/**
 * @brief Given m, n, Get the optimum num cores from the model and input attributes. Calls upon all the above functions
 * 
 * @param m 
 * @param n 
 * @return int 
 */
int get_optimum_num_cores_d2(int m, int n, ModelFunc model_func) {
  //Model already loaded , no need to load each time

  //Load and save memory
  if (memory.m == m && memory.n == n)
    return memory.n_core_choice;
  else {
    memory.m = m; memory.n = n;
  }

  //create feature_index_list, put this part in load_model could be better
  float* data_input;
  data_load_preprocess_d2(&data_input, m, n); // pass reference, or undefined behaviour

  int out_len = 0;
  const float* out_result = NULL;
  model_func(data_input, max_num_threads, feature_num, &out_len, &out_result);

  free(data_input);

  memory.n_core_choice = find_min_index(out_result, out_len);

  return memory.n_core_choice;

}



/**
 * @brief Given m, n, Get the optimum num cores from the model and input attributes. Calls upon all the above functions
 * 
 * @param n 
 * @return int 
 */
int get_optimum_num_cores_d1(int n, ModelFunc model_func) {
  //Model already loaded , no need to load each time

  //Load and save memory
  if (memory.n == n)
    return memory.n_core_choice;
  else {
    memory.n = n;
  }

  //create feature_index_list, put this part in load_model could be better
  float* data_input;
  data_load_preprocess_d1(&data_input, n); // pass reference, or undefined behaviour

  int out_len = 0;
  const float* out_result = NULL;
  model_func(data_input, max_num_threads, feature_num, &out_len, &out_result);

  free(data_input);

  memory.n_core_choice = find_min_index(out_result, out_len);

  return memory.n_core_choice;

}



