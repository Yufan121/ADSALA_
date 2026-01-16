#ifndef __BLAS_CONFIG_C_H_INCLUDED__
#define __BLAS_CONFIG_C_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief C interface for accessing BLAS configuration
 * These functions allow C code (like predictor.c) to access config values
 */

/**
 * @brief Get maximum number of threads for predictor
 * @return Maximum threads (default: 96)
 */
int blas_config_get_predictor_max_threads(void);

/**
 * @brief Get maximum number of features
 * @return Maximum features (default: 17)
 */
int blas_config_get_max_feature(void);

/**
 * @brief Get number of features used
 * @return Number of features (default: 5)
 */
int blas_config_get_feature_num(void);

#ifdef __cplusplus
}
#endif

#endif // __BLAS_CONFIG_C_H_INCLUDED__

