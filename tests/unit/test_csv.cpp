#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "rapidcsv.h"
#include "blas_exceptions.h"

namespace {

class CSVTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test CSV file
        std::ofstream file("test_data.csv");
        file << "m,n,k,result\n";
        file << "100,200,300,0\n";
        file << "200,300,400,0\n";
        file.close();
    }
    
    void TearDown() override {
        std::remove("test_data.csv");
        std::remove("test_output.csv");
    }
};

// Test reading valid CSV file
TEST_F(CSVTest, ReadValidCSV) {
    rapidcsv::Document doc("test_data.csv");
    EXPECT_EQ(doc.GetCell<int>(0, 0), 100);
    EXPECT_EQ(doc.GetCell<int>(1, 0), 200);
    EXPECT_EQ(doc.GetCell<int>(2, 0), 300);
}

// Test reading invalid cell
TEST_F(CSVTest, ReadInvalidCell) {
    rapidcsv::Document doc("test_data.csv");
    EXPECT_THROW(
        doc.GetCell<int>(10, 0),
        std::exception
    );
}

// Test writing to CSV
TEST_F(CSVTest, WriteCSV) {
    rapidcsv::Document doc("test_data.csv");
    doc.SetCell<long long>(3, 0, 12345);
    doc.Save("test_output.csv");
    
    rapidcsv::Document doc2("test_output.csv");
    EXPECT_EQ(doc2.GetCell<long long>(3, 0), 12345);
}

// Test CSV file not found
TEST_F(CSVTest, FileNotFound) {
    EXPECT_THROW(
        rapidcsv::Document doc("nonexistent.csv"),
        std::exception
    );
}

} // namespace

