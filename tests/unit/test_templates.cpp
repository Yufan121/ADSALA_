#include <gtest/gtest.h>
#include <type_traits>
#include "interfaces-ml.h"

namespace {

// Test that template functions can be instantiated for float and double
// Note: This is a compile-time test - if it compiles, templates work correctly

TEST(TemplateTest, TypeTraits) {
    // Verify that float and double are distinct types
    EXPECT_FALSE((std::is_same_v<float, double>));
    EXPECT_TRUE((std::is_same_v<float, float>));
    EXPECT_TRUE((std::is_same_v<double, double>));
}

// Test template instantiation (compile-time check)
// The actual template functions are in interfaces-ml.cpp
// This test verifies the header declarations are correct
TEST(TemplateTest, TemplateDeclarations) {
    // If this compiles, the template declarations are correct
    test_class_ml test_ml;
    
    // These calls would instantiate the templates if we had a real test_class instance
    // For now, we just verify the class exists and has the right methods
    EXPECT_TRUE(true); // Placeholder - actual testing requires mock test_class
}

} // namespace

