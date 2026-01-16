#ifndef __LIBRARY_MANAGER_H_INCLUDED__
#define __LIBRARY_MANAGER_H_INCLUDED__


#include "interfaces.h"
#include <vector>

class library_manager
{
  public:
    library_manager();
    ~library_manager();

    std::vector<test_class*> blas_libs();
    std::vector<std::string> library_names();
    library* get_library(std::string name);
  private:
    std::vector<library*> libs;
};

#endif