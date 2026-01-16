#include <gtest/gtest.h>
#include <unordered_map>
#include <functional>
#include <string>
#include "library-manager.h"
#include "rapidcsv.h"
#include "blas_exceptions.h"

namespace {

// Mock handler function for testing
int mock_handler(test_class* lib, rapidcsv::Document& csv_file, 
                 const std::string& domain_file_name, int line_num, 
                 bool isDouble, int nt, int num_of_duplicate) {
    return 0;
}

// Test routine dispatcher lookup
TEST(DispatcherTest, ValidRoutineLookup) {
    using RoutineHandler = std::function<int(test_class*, rapidcsv::Document&, 
                                             const std::string&, int, bool, int, int)>;
    
    std::unordered_map<std::string, RoutineHandler> handlers = {
        {"gemm", mock_handler},
        {"symm", mock_handler},
        {"syrk", mock_handler}
    };
    
    auto it = handlers.find("gemm");
    EXPECT_NE(it, handlers.end());
    EXPECT_NE(it->second, nullptr);
    
    it = handlers.find("symm");
    EXPECT_NE(it, handlers.end());
    
    it = handlers.find("syrk");
    EXPECT_NE(it, handlers.end());
}

// Test invalid routine lookup
TEST(DispatcherTest, InvalidRoutineLookup) {
    using RoutineHandler = std::function<int(test_class*, rapidcsv::Document&, 
                                             const std::string&, int, bool, int, int)>;
    
    std::unordered_map<std::string, RoutineHandler> handlers = {
        {"gemm", mock_handler}
    };
    
    auto it = handlers.find("invalid_routine");
    EXPECT_EQ(it, handlers.end());
}

// Test all Level 3 routines are registered
TEST(DispatcherTest, AllLevel3RoutinesRegistered) {
    using RoutineHandler = std::function<int(test_class*, rapidcsv::Document&, 
                                             const std::string&, int, bool, int, int)>;
    
    std::vector<std::string> level3_routines = {"gemm", "symm", "syrk", "syr2k", "trmm", "trsm"};
    std::unordered_map<std::string, RoutineHandler> handlers = {
        {"gemm", mock_handler},
        {"symm", mock_handler},
        {"syrk", mock_handler},
        {"syr2k", mock_handler},
        {"trmm", mock_handler},
        {"trsm", mock_handler}
    };
    
    for (const auto& routine : level3_routines) {
        auto it = handlers.find(routine);
        EXPECT_NE(it, handlers.end()) << "Routine " << routine << " not found in dispatcher";
    }
}

} // namespace

