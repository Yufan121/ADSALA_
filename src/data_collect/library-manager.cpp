#include "library-manager.h"
#include <cstdlib>

#ifdef USE_BLAS
#include "BLAS-context.h"
#endif
#ifdef USE_OPENBLAS
#include "OpenBLAS-context.h"
#endif
#ifdef USE_CUBLAS
#include "cuBLAS-context.h"
#endif
#ifdef USE_MKL
#include "MKL-context.h"
#endif
#ifdef USE_BLIS
#include "BLIS-context.h"
#endif


library_manager::library_manager() {
#ifdef USE_BLAS
  libs.push_back(new BLAS_context());
#endif
#ifdef USE_CUBLAS
  libs.push_back(new cuBLAS_context());
#endif
#ifdef USE_OPENBLAS
  libs.push_back(new OpenBLAS_context());
#endif
#ifdef USE_MKL
  libs.push_back(new MKL_context());
#endif
#ifdef USE_BLIS
  // std::cout << "USE_BLIS is called in libman" << std::endl;
  libs.push_back(new BLIS_context());
#endif
}

library_manager::~library_manager() {
  for (library *lib : libs)
    delete lib;
}

std::vector<test_class*> library_manager::blas_libs() {
  std::vector<test_class*> ret;
  for (library* lib : libs) {
    test_class* i = dynamic_cast<test_class*>(lib);
    if (i)
      ret.push_back(i);
  }
  return ret;
}

std::vector<std::string> library_manager::library_names() {
  std::vector<std::string> ret;
  for (library* lib : libs) {
    ret.push_back(lib->name);
  }
  return ret;
}

library* library_manager::get_library(std::string name) {
  for (library* lib : libs) {
    if (lib->name == name)
      return lib;
  }
  return NULL;
}