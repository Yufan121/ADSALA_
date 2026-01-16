#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <iostream>
#include <sstream>

// Forward declarations of helper functions from interfaces-ml.cpp
void calculate_timing_phases(std::chrono::high_resolution_clock::time_point t_1,
                            std::chrono::high_resolution_clock::time_point t_2,
                            std::chrono::high_resolution_clock::time_point t_3,
                            std::chrono::high_resolution_clock::time_point t_4,
                            std::chrono::high_resolution_clock::time_point t_5,
                            int64_t& time1, int64_t& time2, int64_t& time3, int64_t& time4);

void print_timing_phases(int64_t time1, int64_t time2, int64_t time3, int64_t time4);

long long calculate_average_time(const std::vector<long long>& time_vect);

namespace {

// Test fixture for timing helper tests
class TimingHelperTest : public ::testing::Test {
protected:
    // Create test timestamps with known differences
    std::chrono::high_resolution_clock::time_point t1;
    std::chrono::high_resolution_clock::time_point t2;
    std::chrono::high_resolution_clock::time_point t3;
    std::chrono::high_resolution_clock::time_point t4;
    std::chrono::high_resolution_clock::time_point t5;

    void SetUp() override {
        // Create timestamps with known microsecond differences
        t1 = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1000));
        t2 = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1100)); // +100 μs
        t3 = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1200)); // +100 μs
        t4 = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1300)); // +100 μs
        t5 = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1400)); // +100 μs
    }
};

// Test calculate_timing_phases function
TEST_F(TimingHelperTest, CalculateTimingPhases) {
    int64_t time1, time2, time3, time4;

    calculate_timing_phases(t1, t2, t3, t4, t5, time1, time2, time3, time4);

    // Each phase should be 100 microseconds
    EXPECT_EQ(time1, 100); // t2 - t1
    EXPECT_EQ(time2, 100); // t3 - t2
    EXPECT_EQ(time3, 100); // t4 - t3
    EXPECT_EQ(time4, 100); // t5 - t4
}

// Test calculate_timing_phases with zero time intervals
TEST_F(TimingHelperTest, CalculateTimingPhasesZeroIntervals) {
    int64_t time1, time2, time3, time4;

    // All timestamps are the same
    auto t_same = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1000));

    calculate_timing_phases(t_same, t_same, t_same, t_same, t_same, time1, time2, time3, time4);

    EXPECT_EQ(time1, 0);
    EXPECT_EQ(time2, 0);
    EXPECT_EQ(time3, 0);
    EXPECT_EQ(time4, 0);
}

// Test calculate_timing_phases with larger time intervals
TEST_F(TimingHelperTest, CalculateTimingPhasesLargeIntervals) {
    int64_t time1, time2, time3, time4;

    // Create timestamps with 1ms intervals
    auto t1_large = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1000000));
    auto t2_large = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1001000)); // +1ms
    auto t3_large = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1002000)); // +1ms
    auto t4_large = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1003000)); // +1ms
    auto t5_large = std::chrono::high_resolution_clock::time_point(std::chrono::microseconds(1004000)); // +1ms

    calculate_timing_phases(t1_large, t2_large, t3_large, t4_large, t5_large, time1, time2, time3, time4);

    EXPECT_EQ(time1, 1000); // 1ms in microseconds
    EXPECT_EQ(time2, 1000);
    EXPECT_EQ(time3, 1000);
    EXPECT_EQ(time4, 1000);
}

// Test print_timing_phases function
TEST_F(TimingHelperTest, PrintTimingPhases) {
    // Capture cout output
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    print_timing_phases(100, 200, 300, 400);

    std::cout.rdbuf(old); // Restore original buffer

    std::string output = buffer.str();
    EXPECT_NE(output.find("time1~time4:"), std::string::npos);
    EXPECT_NE(output.find("100"), std::string::npos);
    EXPECT_NE(output.find("200"), std::string::npos);
    EXPECT_NE(output.find("300"), std::string::npos);
    EXPECT_NE(output.find("400"), std::string::npos);
    EXPECT_NE(output.find("End: time1~time4:"), std::string::npos);
}

// Test print_timing_phases with zero values
TEST_F(TimingHelperTest, PrintTimingPhasesZeroValues) {
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    print_timing_phases(0, 0, 0, 0);

    std::cout.rdbuf(old);

    std::string output = buffer.str();
    EXPECT_NE(output.find("time1~time4:"), std::string::npos);
    EXPECT_NE(output.find("0"), std::string::npos);
    EXPECT_NE(output.find("End: time1~time4:"), std::string::npos);
}

// Test calculate_average_time function
TEST_F(TimingHelperTest, CalculateAverageTime) {
    std::vector<long long> times = {100, 200, 300, 400, 500};
    long long average = calculate_average_time(times);
    EXPECT_EQ(average, 300); // (100+200+300+400+500)/5 = 300
}

// Test calculate_average_time with empty vector
TEST_F(TimingHelperTest, CalculateAverageTimeEmptyVector) {
    std::vector<long long> times;
    long long average = calculate_average_time(times);
    // Should return 0 for empty vector instead of crashing with division by zero
    EXPECT_EQ(average, 0);
}

// Test calculate_average_time with single element
TEST_F(TimingHelperTest, CalculateAverageTimeSingleElement) {
    std::vector<long long> times = {42};
    long long average = calculate_average_time(times);
    EXPECT_EQ(average, 42);
}

// Test calculate_average_time with large numbers
TEST_F(TimingHelperTest, CalculateAverageTimeLargeNumbers) {
    std::vector<long long> times = {1000000LL, 2000000LL, 3000000LL};
    long long average = calculate_average_time(times);
    EXPECT_EQ(average, 2000000LL);
}

// Test calculate_average_time with odd number of elements
TEST_F(TimingHelperTest, CalculateAverageTimeOddCount) {
    std::vector<long long> times = {1, 2, 3, 4, 5, 6, 7};
    long long average = calculate_average_time(times);
    EXPECT_EQ(average, 4); // (1+2+3+4+5+6+7)/7 = 28/7 = 4
}

} // namespace
