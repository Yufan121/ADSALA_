#include "blas_config_c.h"
#include "blas_config.h"

extern "C" {

int blas_config_get_predictor_max_threads(void) {
    return blas_config::Config::getInstance().getPredictorMaxThreads();
}

int blas_config_get_max_feature(void) {
    return blas_config::Config::getInstance().getMaxFeature();
}

int blas_config_get_feature_num(void) {
    return blas_config::Config::getInstance().getFeatureNum();
}

} // extern "C"

