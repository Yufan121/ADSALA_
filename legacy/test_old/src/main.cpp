#include <iostream>
#include <malloc.h>
#include <ctime>
#include <ratio>
#include <chrono>
#include "rapidcsv.h"
#include "library-manager.h"
#include "interfaces-ml.h"
#include "interfaces.h"
// #include <mlpack.hpp>


using namespace std;

int main(int argc, char *argv[]) { // read sizes from csv file version



  if (argc < 7) {
    std::cout << "Please provide: subroutine, line_id, isML, isDouble, domainFile, BLASPackage, num_duplicates" << std::endl;
    std::cout << "Available libraries:" << std::endl;
    return -1;
  }


  // extract args
  string routine_todo = argv[1];
  int testIndex = std::atoi(argv[2]);
  int useML = std::atoi(argv[3]);
  std::string precision = argv[4];
  bool isDouble=true;
  if (precision == "double") {
    isDouble = true;
  } else if (precision == "single") {
    isDouble = false;
  }
  std::string domain_file_name = argv[5];
  std::string library_name = argv[6];
  size_t num_of_duplicate = std::atoi(argv[7]);


  // get lib from lib name
  library_manager lib_man;
  test_class* lib = dynamic_cast<test_class*>(lib_man.get_library(library_name));
  test_class_ml* test_lib = new test_class_ml(); // create test class for ML, pass test_class to it

  //routine name as a vector
  vector<string> routine_names = {"gemm", "symm", "syrk", "syr2k", "trmm", "trsm", "gemv", "syr", "trsv", "dot", "axpy"};

  rapidcsv::Document csv_file(domain_file_name);//read domain file





  if (routine_todo == routine_names[0]) { /*GEMM*/


    int m = csv_file.GetCell<int>(0, testIndex);
    int k = csv_file.GetCell<int>(1, testIndex);
    int n = csv_file.GetCell<int>(2, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_gemm_(m,k,n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 3, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;

  } else if (routine_todo == routine_names[1]) { /*SYMM*/

    int m = csv_file.GetCell<int>(0, testIndex);
    int n = csv_file.GetCell<int>(1, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_symm_(m,n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 2, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
  
  } else if (routine_todo == routine_names[2]) { /*SYRK*/

    int n = csv_file.GetCell<int>(0, testIndex);
    int k = csv_file.GetCell<int>(1, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_syrk_(n, k, isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 2, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
  
  } else if (routine_todo == routine_names[3]) { /*SYR2K*/

    int n = csv_file.GetCell<int>(0, testIndex);
    int k = csv_file.GetCell<int>(1, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_syr2k_(n, k,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 2, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
  
  } else if (routine_todo == routine_names[4]) { /*TRMM*/

    int m = csv_file.GetCell<int>(0, testIndex);
    int n = csv_file.GetCell<int>(1, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_trmm_(m,n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 2, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
  
  } else if (routine_todo == routine_names[5]) { /*TRSM*/

    int m = csv_file.GetCell<int>(0, testIndex);
    int n = csv_file.GetCell<int>(1, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_trsm_(m,n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 2, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
  

  
  } else if (routine_todo == routine_names[6]) { /*GEMV*/ 


    int m = csv_file.GetCell<int>(0, testIndex);
    int n = csv_file.GetCell<int>(1, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_gemv_(m,n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 2, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;

  } else if (routine_todo == routine_names[7]) { // /*SYR*/
  
    int n = csv_file.GetCell<int>(0, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_syr_(n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 1, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
      
  } else if (routine_todo == routine_names[8]) { // /*TRSV*/


    int n = csv_file.GetCell<int>(0, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_trsv_(n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 1, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
  


  } else if (routine_todo == routine_names[9]) { // /*DOT*/

    int n = csv_file.GetCell<int>(0, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_dot_(n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 1, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;


  } else if (routine_todo == routine_names[10]) { // /*AXPY*/


    int n = csv_file.GetCell<int>(0, testIndex);

    // run test
    long long time;
    time = test_lib->run_compare_axpy_(n,isDouble,useML,num_of_duplicate, lib);//
    
    // save time
    csv_file.SetCell<long long>(useML + 1, testIndex, time/num_of_duplicate);//set each time result

    csv_file.Save(domain_file_name);//write file

    std::cout << time << std::endl;

    return 0;
  
  } else {
    cout << "Routine name not found." << endl;
    return -1;
  }


}


