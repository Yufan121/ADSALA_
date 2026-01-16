#include <cstdlib>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdexcept>

using namespace std;

#include "library-manager.h"
// #include "domain-files.h"
#include "rapidcsv.h"





int main(int argc, char *argv[]) {

  library_manager lib_man; //library manager

  if (argc < 7) {
    std::cout << "Please provide a domain file, a library and a precision" << std::endl;
    std::cout << "Available libraries:" << std::endl;
    for (std::string s : lib_man.library_names()) 
      std:: cout << "\t- " << s << std::endl;
    return -1;
  }

  //parameters
  string routine_todo = argv[1];
  std::string domain_file_name = argv[2];
  std::string library_name = argv[3];
  std::string precision = argv[4];
  int nt = std::atoi(argv[5]);
  int line_num = std::atoi(argv[6]);

  // parse precision
  bool isDouble=true;
  if (precision == "double")  isDouble = true;
  else if (precision == "single") isDouble = false;
  else {
    cout << "Precision not found." << endl;
    return -1;
  }
  
  line_num -= 1; //line number starts from 0, csv might ignore heading line


  // get lib from lib name
  test_class* lib = dynamic_cast<test_class*>(lib_man.get_library(library_name));
  rapidcsv::Document csv_file(domain_file_name);//read domain file


  //routine name as a vector
  vector<string> routine_names = {"gemm", "symm", "syrk", "syr2k", "trmm", "trsm", "gemv", "syr", "trsv", "dot", "axpy"};
  int num_of_duplicate = 10;


  if (routine_todo == routine_names[0]) { /*GEMM*/


    //get domain
    // do not stop program if domain is not found
    int dom1, dom2, dom3;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
      dom3 = csv_file.GetCell<int>(2, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;
    std::cout<< dom3 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_gemm(dom1, dom2, dom3, nt, isDouble, num_of_duplicate);

    // // print result
    // std::cout << "********** Result: " << std::endl;
    // for(int i=0; i < num_of_duplicate; i++){
    //   std::cout << result[i] << std::endl;
    // }

    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+3-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file

    return 0;

  } else if (routine_todo == routine_names[1]) { /*SYMM*/
    //get domain
    // do not stop program if domain is not found
    int dom1, dom2;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_symm(dom1, dom2, nt, isDouble, num_of_duplicate);

    // // print result
    // std::cout << "********** Result: " << std::endl;
    // for(int i=0; i < num_of_duplicate; i++){
    //   std::cout << result[i] << std::endl;
    // }

    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+2-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file

    return 0;
    
  } else if (routine_todo == routine_names[2]) { /*SYRK*/
    //get domain
    // do not stop program if domain is not found
    int dom1, dom2;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_syrk(dom1, dom2, nt, isDouble, num_of_duplicate);

    // // print result
    // std::cout << "********** Result: " << std::endl;
    // for(int i=0; i < num_of_duplicate; i++){
    //   std::cout << result[i] << std::endl;
    // }

    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+2-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file

    return 0;
    
  } else if (routine_todo == routine_names[3]) { /*SYR2K*/
    //get domain
    // do not stop program if domain is not found
    int dom1, dom2;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_syr2k(dom1, dom2, nt, isDouble, num_of_duplicate);
    // // print result
    // std::cout << "********** Result: " << std::endl;
    // for(int i=0; i < num_of_duplicate; i++){
    //   std::cout << result[i] << std::endl;
    // }

    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+2-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file

    return 0;
    
  } else if (routine_todo == routine_names[4]) { /*TRMM*/
    //get domain
    // do not stop program if domain is not found
    int dom1, dom2;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_trmm(dom1, dom2, nt, isDouble, num_of_duplicate);

    // // print result
    // std::cout << "********** Result: " << std::endl;
    // for(int i=0; i < num_of_duplicate; i++){
    //   std::cout << result[i] << std::endl;
    // }

    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+2-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file

    return 0;

  } else if (routine_todo == routine_names[5]) { /*TRSM*/
    //get domain
    // do not stop program if domain is not found
    int dom1, dom2;
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_trsm(dom1, dom2, nt, isDouble, num_of_duplicate);

    // // print result
    // std::cout << "********** Result: " << std::endl;
    // for(int i=0; i < num_of_duplicate; i++){
    //   std::cout << result[i] << std::endl;
    // }

    for(int i=0; i < num_of_duplicate; i++){
      try {
        csv_file.SetCell<long long>(nt+2-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file

    return 0;

  
  } else if (routine_todo == routine_names[6]) { /*GEMV*/ 


    //get domain
    // do not stop program if domain is not found
    int dom1, dom2; 
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
      dom2 = csv_file.GetCell<int>(1, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;
    std::cout<< dom2 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_gemv(dom1, dom2, nt, isDouble, num_of_duplicate);
    for(int i=0; i < num_of_duplicate; i++){

      try {
        csv_file.SetCell<long long>(nt+2-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file
    
    return 0;

  } else if (routine_todo == routine_names[7]) { // /*SYR*/
  
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
    // do not stop program if domain is not found
    int dom1; 
    try {
      dom1 = csv_file.GetCell<int>(0, line_num);
    } catch (const std::out_of_range& oor) {
      std::cerr << "RapidCSV get cell error: " << oor.what() << '\n';
      return -1;
    }

    std::cout<< dom1 <<std::endl;

    //Do the testing and measurement
    std::vector<long long> result =  lib->test_trsv(dom1, nt, isDouble, num_of_duplicate);
    for(int i=0; i < num_of_duplicate; i++){

      try {
        csv_file.SetCell<long long>(nt+1-1, line_num+i, result[i]);//set each time result
      } catch (const std::out_of_range& oor) {
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file
    
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
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
      std::cout<< result[i] <<std::endl;
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file
    
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
        std::cerr << "RapidCSV set cell error: " << oor.what() << '\n';
        return -1;
      }
      std::cout<< result[i] <<std::endl;
    }
    // std::cout<< result <<std::endl;
    csv_file.Save(domain_file_name);//write file
    
    return 0;

  
  } else {
    cout << "Routine name not found." << endl;
    return -1;
  }




}

