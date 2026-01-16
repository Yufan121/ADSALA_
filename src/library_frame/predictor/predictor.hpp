#ifndef PREDICTOR_HPP
#define PREDICTOR_HPP

#include <iostream>
#include <string.h>
#include <vector>
#include <mlpack.hpp>

extern "C" {
    #include "predictor.c"//include predictor
}


using namespace std;
using namespace mlpack;
using namespace mlpack::tree; // RandomForest is located in this namespace.

/* Call Order */
/* Predictor -> Model -> specific Model */

/* Model Parent Class */


class Model
{
    public: 
        Model() {};
        virtual ~Model() {};

        virtual int predict_num_cores(int m, int k, int n, string sub_routine) = 0;
        virtual int predict_num_cores(int m, int n, string sub_routine) = 0;
        virtual int predict_num_cores(int n, string sub_routine) = 0;

        // virtual void register_sub_routines(string[] sub_routines_names);
        virtual bool is_ml_needed(int* dims, string sub_routine) = 0;

        //variables 
        int num_of_duplicate;
        // string sub_routines[];
};

/* Function Pointer */

// XGB evaluation unit is in predictor.c
// pass

// LR evaluation unit
void lr_model(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result) {
    // Convert your data_input to Armadillo matrix format.
    arma::Row<float> X(data_input, max_num_threads, false, true);

    // TODO: You need to provide the 'y' vector (the labels).
    arma::Row<float> y; 

    // Solve the normal equations.
    arma::Row<float> beta = arma::solve(X, y.t());

    // Make predictions.
    arma::Row<float> predictions = X * beta.t();

    // Set the output length and result.
    *out_len = predictions.n_elem;
    *out_result = predictions.memptr();
}

// Bayesian LR evaluation unit
void bayesian_lr_model(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result) {
    // Convert your data_input to Armadillo matrix format.
    arma::Row<float> X(data_input, max_num_threads, false, true);

    // TODO: You need to provide the 'y' vector (the labels).
    arma::Row<float> y; 

    // TODO: You need to provide the prior covariance matrix 'S0' and the noise variance 'sigma2'.
    arma::mat S0;
    double sigma2;

    // Compute the posterior covariance matrix and mean vector.
    arma::mat S_n = arma::inv(arma::inv(S0) + (1 / sigma2) * X.t() * X);
    arma::Row<float> m_n = arma::conv_to<arma::Row<float>>::from((S_n * ((1 / sigma2) * X.t() * y.t())));

    // Make predictions.
    arma::Row<float> predictions = (X * m_n.t()).t();

    // Set the output length and result.
    *out_len = predictions.n_elem;
    *out_result = predictions.memptr();
}

// Random Forest evaluation unit
void random_forest_model(float* data_input, int max_num_threads, int feature_num, int* out_len, const float** out_result) {
    // Convert your data_input to a double array.
    double* data_input_double = new double[max_num_threads * feature_num];
    for(int i = 0; i < max_num_threads * feature_num; i++)
        data_input_double[i] = static_cast<double>(data_input[i]);

    // Now you can create your Armadillo row.
    arma::Row<double> X(data_input_double, max_num_threads * feature_num, false, true);

    // Initialize an empty random forest model.
    RandomForest<> rf;

    // Predict the labels of the test points.
    arma::Row<size_t> output;
    rf.Classify(X, output); 

    // Convert size_t predictions to float.
    arma::Row<float> output_float = arma::conv_to<arma::Row<float>>::from(output);

    // Set the output length and result.
    *out_len = max_num_threads;
    *out_result = output_float.memptr();

    // Print the result.
    for (int i = 0; i < *out_len; i++) {
        std::cout << (*out_result)[i] << " ";
    }

    delete[] data_input_double;
}





/* Function Pointer END */


/* XGB Model Class */

class XGB_Model: public Model {
    public:
        XGB_Model(string routine);
         ~XGB_Model();
        int predict_num_cores(int m, int k, int n, string sub_routine);
        int predict_num_cores(int m, int n, string sub_routine);
        int predict_num_cores(int n, string sub_routine);

        void register_num_of_duplicate(int num_of_duplicate);
        bool is_ml_needed(int* dims, string sub_routine);

    private:
        char* model_name;
        char* config_name;
};

XGB_Model::XGB_Model(string routine) {
    // get model and config name
    model_name = new char[15 + routine.length()];
    strcpy(model_name, ("xgb_" + routine + ".model").c_str());
    config_name = new char[15 + routine.length()];
    strcpy(config_name, ("xgb_" + routine + ".ini").c_str());

    // load model and config file
    load_model(model_name);
    load_config(config_name);

    // register subroutines
    // this-> sub_routines = {"dgemm", "sgemm", "dsymm", "ssymm", "dgemv", "sgemv", "dtrsv", "strsv", "ddot", "sdot"};
}

XGB_Model::~XGB_Model() {
    // safe_xgboost(XGBoosterFree(booster));
    delete[] model_name;
    delete[] config_name;
}

int XGB_Model::predict_num_cores(int m, int k, int n, string sub_routine) {

    return get_optimum_num_cores_d3(m,k,n,xgb_model);
}

int XGB_Model::predict_num_cores(int m, int n, string sub_routine) {

    // return get_optimum_num_cores_d2(m,n,xgb_model);
    return get_optimum_num_cores_d2(m,n,random_forest_model);
}

int XGB_Model::predict_num_cores(int n, string sub_routine) {

    return get_optimum_num_cores_d1(n,xgb_model);
}

void XGB_Model::register_num_of_duplicate(int num_of_duplicate) {
    // TODO: implement
}

bool XGB_Model::is_ml_needed(int* dims, string sub_routine) {
    // TODO: implement
    return false;
}






/* LR Model Class */

class LR_Model: public Model {
    public:
        LR_Model(string routine);
         ~LR_Model();
        int predict_num_cores(int m, int k, int n, string sub_routine);
        int predict_num_cores(int m, int n, string sub_routine);
        int predict_num_cores(int n, string sub_routine);

        void register_num_of_duplicate(int num_of_duplicate);
        bool is_ml_needed(int* dims, string sub_routine);

    private:
        char* model_name;
        char* config_name;
};



LR_Model::LR_Model(string routine) {
    // load model config
    config_name = new char[15 + routine.length()];
    strcpy(config_name, ("lr_" + routine + ".ini").c_str());

    // TODO: load config file
}

LR_Model::~LR_Model() {
}

int LR_Model::predict_num_cores(int m, int k, int n, string sub_routine) {
    
    return get_optimum_num_cores_d3(m,k,n,lr_model);
}

int LR_Model::predict_num_cores(int m, int n, string sub_routine) {
    
    return get_optimum_num_cores_d2(m,n,lr_model);
}

int LR_Model::predict_num_cores(int n, string sub_routine) {
    
    return get_optimum_num_cores_d1(n,lr_model);
}

void LR_Model::register_num_of_duplicate(int num_of_duplicate) {
    // TODO: implement
}

bool LR_Model::is_ml_needed(int* dims, string sub_routine) {
    // TODO: implement
    return false;
}



/* Bayesian LR Model Class */

class BayesianLR_Model: public Model {
    public:
        BayesianLR_Model(string routine);
         ~BayesianLR_Model();
        int predict_num_cores(int m, int k, int n, string sub_routine);
        int predict_num_cores(int m, int n, string sub_routine);
        int predict_num_cores(int n, string sub_routine);

        void register_num_of_duplicate(int num_of_duplicate);
        bool is_ml_needed(int* dims, string sub_routine);

    private:
        char* model_name;
        char* config_name;
};

BayesianLR_Model::BayesianLR_Model(string routine) {
    // load model config
    config_name = new char[15 + routine.length()];
    strcpy(config_name, ("blr_" + routine + ".ini").c_str());
    
    // TODO: load config file
}

BayesianLR_Model::~BayesianLR_Model() {
}

int BayesianLR_Model::predict_num_cores(int m, int k, int n, string sub_routine) {

    return get_optimum_num_cores_d3(m,k,n,bayesian_lr_model);
}

int BayesianLR_Model::predict_num_cores(int m, int n, string sub_routine) {

    return get_optimum_num_cores_d2(m,n,bayesian_lr_model);
}

int BayesianLR_Model::predict_num_cores(int n, string sub_routine) {

    return get_optimum_num_cores_d1(n,bayesian_lr_model);
}

void BayesianLR_Model::register_num_of_duplicate(int num_of_duplicate) {
    // TODO: implement
}

bool BayesianLR_Model::is_ml_needed(int* dims, string sub_routine) {
    // TODO: implement
    return false;
}


/* Random Forest Model Class */

class RandomForest_Model: public Model {
    public:
        RandomForest_Model(string routine);
         ~RandomForest_Model();
        int predict_num_cores(int m, int k, int n, string sub_routine);
        int predict_num_cores(int m, int n, string sub_routine);
        int predict_num_cores(int n, string sub_routine);

        void register_num_of_duplicate(int num_of_duplicate);
        bool is_ml_needed(int* dims, string sub_routine);

    private:
        char* model_name;
        char* config_name;
};

RandomForest_Model::RandomForest_Model(string routine) {
    // load model config
    config_name = new char[15 + routine.length()];
    strcpy(config_name, ("rf_" + routine + ".ini").c_str());

    // TODO: load config file
}

RandomForest_Model::~RandomForest_Model() {
}

int RandomForest_Model::predict_num_cores(int m, int k, int n, string sub_routine) {

    return get_optimum_num_cores_d3(m,k,n,random_forest_model);
}

int RandomForest_Model::predict_num_cores(int m, int n, string sub_routine) {

    return get_optimum_num_cores_d2(m,n,random_forest_model);
}

int RandomForest_Model::predict_num_cores(int n, string sub_routine) {

    return get_optimum_num_cores_d1(n,random_forest_model);
}

void RandomForest_Model::register_num_of_duplicate(int num_of_duplicate) {
    // TODO: implement
}

bool RandomForest_Model::is_ml_needed(int* dims, string sub_routine) {
    // TODO: implement
    return false;
}





/* Predictor Class, the base class that is called during the runtime */


class Predictor
{
    public:
        // void set_model(Model* model) { this->model = model; };

        Predictor(string model, string sub_routine);
        ~Predictor();

        //variables 
        Model* model;
        int num_of_duplicate;
        int sub_routines;

        int predict_num_cores(int m, int k, int n, int model, string sub_routine) { return this->model -> predict_num_cores(m, k, n, sub_routine); };
        int predict_num_cores(int m, int n, int model, string sub_routine) { return this->model -> predict_num_cores(m, n, sub_routine); };
        int predict_num_cores(int n, int model, string sub_routine) { return this->model -> predict_num_cores(n, sub_routine); };

        void register_model(int model); // also need supported subroutines
        void register_num_of_duplicate(int num_of_duplicate);
        void is_ml_needed(int* dims, int sub_routine);


};


Predictor::Predictor(string model, string sub_routine) {
    if (model == "xgb") {
        this -> model = new XGB_Model(sub_routine);
    } else if (model == "lr") {
        this -> model = new LR_Model(sub_routine);
    } 
    else {
        cout << "Model not supported" << endl;
    }
}

Predictor::~Predictor() {
    delete this -> model;
}




#endif










