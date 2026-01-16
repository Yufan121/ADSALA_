#include <cstdlib>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <limits>

using namespace std;

#include "library-manager.h"
// #include "domain-files.h"
#include "rapidcsv.h"
#include "../library_frame/BLAS/include/blas_exceptions.h"

// Helper function to read CSV domain values
template<typename T>
std::vector<T> read_csv_domain(rapidcsv::Document& csv_file, int line_num, int num_dims) {
  std::vector<T> domain;
  domain.reserve(num_dims);
  for (int i = 0; i < num_dims; i++) {
    try {
      domain.push_back(csv_file.GetCell<T>(i, line_num));
    } catch (const std::out_of_range& oor) {
      throw blas_exceptions::CSVException("read", 
        "cell (" + std::to_string(i) + ", " + std::to_string(line_num) + ") out of range: " + oor.what());
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("read", 
        "error reading cell (" + std::to_string(i) + ", " + std::to_string(line_num) + "): " + e.what());
    }
  }
  return domain;
}

// Helper function to write CSV results
void write_csv_results(rapidcsv::Document& csv_file, const std::vector<long long>& results, 
                       int line_num, int result_column, int num_of_duplicate) {
  for (int i = 0; i < num_of_duplicate; i++) {
    try {
      csv_file.SetCell<long long>(result_column, line_num + i, results[i]);
    } catch (const std::out_of_range& oor) {
      throw blas_exceptions::CSVException("write", 
        "cell (" + std::to_string(result_column) + ", " + std::to_string(line_num + i) + ") out of range: " + oor.what());
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("write", 
        "error writing cell (" + std::to_string(result_column) + ", " + std::to_string(line_num + i) + "): " + e.what());
    }
  }
}

// Handler function type for Level 3 routines
using RoutineHandler = std::function<int(test_class*, rapidcsv::Document&, const std::string&, int, bool, int, int)>;

// Handler for GEMM (3 domain parameters)
int handle_gemm(test_class* lib, rapidcsv::Document& csv_file, const std::string& domain_file_name, 
                int line_num, bool isDouble, int nt, int num_of_duplicate) {
  auto domain = read_csv_domain<int>(csv_file, line_num, 3);
  int dom1 = domain[0], dom2 = domain[1], dom3 = domain[2];
  
  std::cout << dom1 << std::endl;
  std::cout << dom2 << std::endl;
  std::cout << dom3 << std::endl;
  
  std::vector<long long> result = lib->test_gemm(dom1, dom2, dom3, nt, isDouble, num_of_duplicate);
  write_csv_results(csv_file, result, line_num, nt + 3 - 1, num_of_duplicate);
  csv_file.Save(domain_file_name);
  return 0;
}

// Handler for SYMM, SYRK, SYR2K, TRMM, TRSM (2 domain parameters)
int handle_symm(test_class* lib, rapidcsv::Document& csv_file, const std::string& domain_file_name, 
                int line_num, bool isDouble, int nt, int num_of_duplicate) {
  auto domain = read_csv_domain<int>(csv_file, line_num, 2);
  int dom1 = domain[0], dom2 = domain[1];
  
  std::cout << dom1 << std::endl;
  std::cout << dom2 << std::endl;
  
  std::vector<long long> result = lib->test_symm(dom1, dom2, nt, isDouble, num_of_duplicate);
  write_csv_results(csv_file, result, line_num, nt + 2 - 1, num_of_duplicate);
  csv_file.Save(domain_file_name);
  return 0;
}

int handle_syrk(test_class* lib, rapidcsv::Document& csv_file, const std::string& domain_file_name, 
                int line_num, bool isDouble, int nt, int num_of_duplicate) {
  auto domain = read_csv_domain<int>(csv_file, line_num, 2);
  int dom1 = domain[0], dom2 = domain[1];
  
  std::cout << dom1 << std::endl;
  std::cout << dom2 << std::endl;
  
  std::vector<long long> result = lib->test_syrk(dom1, dom2, nt, isDouble, num_of_duplicate);
  write_csv_results(csv_file, result, line_num, nt + 2 - 1, num_of_duplicate);
  csv_file.Save(domain_file_name);
  return 0;
}

int handle_syr2k(test_class* lib, rapidcsv::Document& csv_file, const std::string& domain_file_name, 
                 int line_num, bool isDouble, int nt, int num_of_duplicate) {
  auto domain = read_csv_domain<int>(csv_file, line_num, 2);
  int dom1 = domain[0], dom2 = domain[1];
  
  std::cout << dom1 << std::endl;
  std::cout << dom2 << std::endl;
  
  std::vector<long long> result = lib->test_syr2k(dom1, dom2, nt, isDouble, num_of_duplicate);
  write_csv_results(csv_file, result, line_num, nt + 2 - 1, num_of_duplicate);
  csv_file.Save(domain_file_name);
  return 0;
}

int handle_trmm(test_class* lib, rapidcsv::Document& csv_file, const std::string& domain_file_name, 
                int line_num, bool isDouble, int nt, int num_of_duplicate) {
  auto domain = read_csv_domain<int>(csv_file, line_num, 2);
  int dom1 = domain[0], dom2 = domain[1];
  
  std::cout << dom1 << std::endl;
  std::cout << dom2 << std::endl;
  
  std::vector<long long> result = lib->test_trmm(dom1, dom2, nt, isDouble, num_of_duplicate);
  write_csv_results(csv_file, result, line_num, nt + 2 - 1, num_of_duplicate);
  csv_file.Save(domain_file_name);
  return 0;
}

int handle_trsm(test_class* lib, rapidcsv::Document& csv_file, const std::string& domain_file_name, 
                int line_num, bool isDouble, int nt, int num_of_duplicate) {
  auto domain = read_csv_domain<int>(csv_file, line_num, 2);
  int dom1 = domain[0], dom2 = domain[1];
  
  std::cout << dom1 << std::endl;
  std::cout << dom2 << std::endl;
  
  std::vector<long long> result = lib->test_trsm(dom1, dom2, nt, isDouble, num_of_duplicate);
  write_csv_results(csv_file, result, line_num, nt + 2 - 1, num_of_duplicate);
  csv_file.Save(domain_file_name);
  return 0;
}





int main(int argc, char *argv[]) {
  try {
  library_manager lib_man; //library manager

  if (argc < 7) {
      std::cerr << "Usage: " << argv[0] << " <routine> <domain_file> <library> <precision> <num_threads> <line_num>" << std::endl;
      std::cerr << "Please provide a domain file, a library and a precision" << std::endl;
      std::cerr << "Available libraries:" << std::endl;
    for (std::string s : lib_man.library_names()) 
        std::cerr << "\t- " << s << std::endl;
      return 1;
  }

  //parameters
  string routine_todo = argv[1];
  std::string domain_file_name = argv[2];
  std::string library_name = argv[3];
  std::string precision = argv[4];
    
    // Validate and parse num_threads
    char* endptr;
    long nt_long = std::strtol(argv[5], &endptr, 10);
    if (*endptr != '\0' || nt_long < 1 || nt_long > std::numeric_limits<int>::max()) {
      throw blas_exceptions::InvalidArgumentException("num_threads", "must be a positive integer");
    }
    int nt = static_cast<int>(nt_long);
    
    // Validate and parse line_num
    long line_num_long = std::strtol(argv[6], &endptr, 10);
    if (*endptr != '\0' || line_num_long < 1 || line_num_long > std::numeric_limits<int>::max()) {
      throw blas_exceptions::InvalidArgumentException("line_num", "must be a positive integer");
    }
    int line_num = static_cast<int>(line_num_long);

    // parse precision
    bool isDouble = true;
    if (precision == "double") {
      isDouble = true;
    } else if (precision == "single") {
      isDouble = false;
    } else {
      throw blas_exceptions::InvalidArgumentException("precision", "must be 'double' or 'single'");
    }
    
    line_num -= 1; //line number starts from 0, csv might ignore heading line
    if (line_num < 0) {
      throw blas_exceptions::InvalidArgumentException("line_num", "must be >= 1");
    }

    // Validate routine name
    vector<string> routine_names = {"gemm", "symm", "syrk", "syr2k", "trmm", "trsm", "gemv", "syr", "trsv", "dot", "axpy"};
    bool routine_valid = false;
    for (const auto& name : routine_names) {
      if (routine_todo == name) {
        routine_valid = true;
        break;
      }
    }
    if (!routine_valid) {
      throw blas_exceptions::InvalidArgumentException("routine", "unknown routine: " + routine_todo);
    }

    // get lib from lib name
    test_class* lib = dynamic_cast<test_class*>(lib_man.get_library(library_name));
    if (lib == nullptr) {
      throw blas_exceptions::InvalidArgumentException("library", "library '" + library_name + "' not found or invalid");
    }

    // Read CSV file with exception handling
    rapidcsv::Document csv_file;
    try {
      csv_file = rapidcsv::Document(domain_file_name);
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("read", "failed to open file '" + domain_file_name + "': " + e.what());
    }

    int num_of_duplicate = 10;

    // Create routine dispatcher map for Level 3 routines
    std::unordered_map<std::string, RoutineHandler> level3_handlers = {
      {"gemm", handle_gemm},
      {"symm", handle_symm},
      {"syrk", handle_syrk},
      {"syr2k", handle_syr2k},
      {"trmm", handle_trmm},
      {"trsm", handle_trsm}
    };

    // Try Level 3 routine dispatcher first
    auto handler_it = level3_handlers.find(routine_todo);
    if (handler_it != level3_handlers.end()) {
      return handler_it->second(lib, csv_file, domain_file_name, line_num, isDouble, nt, num_of_duplicate);
    }

    // Fall back to Level 2/1 routines (keep existing if-else chain for now)
    vector<string> routine_names = {"gemm", "symm", "syrk", "syr2k", "trmm", "trsm", "gemv", "syr", "trsv", "dot", "axpy"};
    if (routine_todo == routine_names[6]) { /*GEMV*/ 


    //get domain
    int dom1, dom2; 
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
    } catch (const std::out_of_range& oor) {
      throw blas_exceptions::CSVException("read", 
        "cell (0, " + std::to_string(line_num) + ") or (1, " + std::to_string(line_num) + ") out of range: " + oor.what());
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("read", "error reading domain values: " + std::string(e.what()));
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_gemv(dom1, dom2, nt, isDouble, num_of_duplicate);
    for(int i=0; i < num_of_duplicate; i++){

      try {
        csv_file.SetCell<long long>(nt+2-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        throw blas_exceptions::CSVException("write", 
          "cell (" + std::to_string(nt+2-1) + ", " + std::to_string(line_num+i) + ") out of range: " + oor.what());
      } catch (const std::exception& e) {
        throw blas_exceptions::CSVException("write", "error writing result: " + std::string(e.what()));
      }
    }
    // std::cout<< result <<std::endl;
    try {
      csv_file.Save(domain_file_name);//write file
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("save", "failed to save file '" + domain_file_name + "': " + e.what());
    }
    
    return 0;

  } else if (routine_todo == routine_names[7]) { // /*SYR*/
  
      //get domain
      int dom1;
      try {
        dom1 = csv_file.GetCell<int>(0, line_num);
      } catch (const std::out_of_range& oor) {
        throw blas_exceptions::CSVException("read", 
          "cell (0, " + std::to_string(line_num) + ") out of range: " + oor.what());
      } catch (const std::exception& e) {
        throw blas_exceptions::CSVException("read", "error reading domain value: " + std::string(e.what()));
      }
  
      std::cout<< dom1 <<std::endl;
  
      //Do the testing and measurement
      std::vector<long long> result =  lib->test_syr(dom1, nt, isDouble, num_of_duplicate);
  
      
      for(int i=0; i < num_of_duplicate; i++){
        try {
          csv_file.SetCell<long long>(nt+1-1, line_num+i, result[i]);//set each time result
        } catch (const std::out_of_range& oor) {
          std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
          return -1;
        }
        std::cout<< result[i] <<std::endl;
      }
      // std::cout<< result <<std::endl;
      csv_file.Save(domain_file_name);//write file
      
      return 0;
  } else if (routine_todo == routine_names[8]) { // /*TRSV*/


    //get domain
    int dom1; 
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
    } catch (const std::out_of_range& oor) {
      throw blas_exceptions::CSVException("read", 
        "cell (0, " + std::to_string(line_num) + ") out of range: " + oor.what());
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("read", "error reading domain value: " + std::string(e.what()));
    }

    std::cout<< dom1 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_trsv(dom1, nt, isDouble, num_of_duplicate);
    for(int i=0; i < num_of_duplicate; i++){

      try {
        csv_file.SetCell<long long>(nt+1-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        throw blas_exceptions::CSVException("write", 
          "cell (" + std::to_string(nt+1-1) + ", " + std::to_string(line_num+i) + ") out of range: " + oor.what());
      } catch (const std::exception& e) {
        throw blas_exceptions::CSVException("write", "error writing result: " + std::string(e.what()));
      }
    }
    // std::cout<< result <<std::endl;
    try {
      csv_file.Save(domain_file_name);//write file
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("save", "failed to save file '" + domain_file_name + "': " + e.what());
    }
    
    return 0;

  } else if (routine_todo == routine_names[9]) { // /*DOT*/

    //get domain
    int dom1;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_dot(dom1, nt, isDouble, num_of_duplicate);

    
    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+1-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        throw blas_exceptions::CSVException("write", 
          "cell (" + std::to_string(nt+1-1) + ", " + std::to_string(line_num+i) + ") out of range: " + oor.what());
      } catch (const std::exception& e) {
        throw blas_exceptions::CSVException("write", "error writing result: " + std::string(e.what()));
      }
      std::cout<< result[i] <<std::endl;
    }
    // std::cout<< result <<std::endl;
    try {
      csv_file.Save(domain_file_name);//write file
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("save", "failed to save file '" + domain_file_name + "': " + e.what());
    }
    
    return 0;


  } else if (routine_todo == routine_names[10]) { // /*AXPY*/
    //get domain
    int dom1;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_axpy(dom1, nt, isDouble, num_of_duplicate);

    
    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+1-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        throw blas_exceptions::CSVException("write", 
          "cell (" + std::to_string(nt+1-1) + ", " + std::to_string(line_num+i) + ") out of range: " + oor.what());
      } catch (const std::exception& e) {
        throw blas_exceptions::CSVException("write", "error writing result: " + std::string(e.what()));
      }
      std::cout<< result[i] <<std::endl;
    }
    // std::cout<< result <<std::endl;
    try {
      csv_file.Save(domain_file_name);//write file
    } catch (const std::exception& e) {
      throw blas_exceptions::CSVException("save", "failed to save file '" + domain_file_name + "': " + e.what());
    }
    
    return 0;

  
  } else {
      throw blas_exceptions::InvalidArgumentException("routine", "routine '" + routine_todo + "' not implemented");
    }

  } catch (const blas_exceptions::BLASException& e) {
    std::cerr << "BLAS Error: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown error occurred" << std::endl;
    return 1;
  }
}

