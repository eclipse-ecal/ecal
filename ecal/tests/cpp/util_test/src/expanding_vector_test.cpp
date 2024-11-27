#include "util/expanding_vector.h" // Assuming the class is defined in this header file
#include <cstddef>
#include <gtest/gtest.h>
#include <ostream>
#include <stdexcept>

using namespace eCAL::Util;

// Define a simple class that has a clear function
class MyElement {
public:
  int value;

  MyElement(int v = 0) : value(v) {}

  void clear() {
    value = 0;  // Reset the value to zero when clear is called
  }

  bool operator==(const MyElement& other) const {
    return value == other.value;
  }

  friend std::ostream& operator<<(std::ostream& os, const MyElement& elem) {
    os << elem.value;
    return os;
  }
};

// Unit test class for CExpandingVector
class CExpandingVectorTest : public ::testing::Test {
protected:
  CExpandingVector<MyElement> vec;  // Create a vector of MyElement type

  void SetUp() override {
    // Add some initial values
    vec.push_back(MyElement(1));
    vec.push_back(MyElement(2));
    vec.push_back(MyElement(3));
  }
};

using core_cpp_util_expanding_vector = CExpandingVectorTest;

// Test push_back and size functionality
TEST_F(core_cpp_util_expanding_vector, PushBackAndSize) {
  EXPECT_EQ(vec.size(), 3);  // Size should be 3 after pushing 3 elements

  vec.push_back(MyElement(4));
  EXPECT_EQ(vec.size(), 4);  // Size should now be 4

  EXPECT_EQ(vec[0].value, 1);
  EXPECT_EQ(vec[1].value, 2);
  EXPECT_EQ(vec[2].value, 3);
  EXPECT_EQ(vec[3].value, 4);
}

// Test clear functionality
TEST_F(core_cpp_util_expanding_vector, ClearElements) {
  vec.clear();  // Call clear, which should reset individual elements

  EXPECT_EQ(vec.size(), 0);  // Internal size should be reset to 0

  // Ensure underlying data is still there and reset to 0
  EXPECT_EQ(vec.full_size(), 3);  // Underlying storage is still 3
  EXPECT_EQ(vec[0].value, 0);
  EXPECT_EQ(vec[1].value, 0);
  EXPECT_EQ(vec[2].value, 0);

  EXPECT_EQ(vec.push_back().value, 0);
  EXPECT_EQ(vec.push_back().value, 0);
}

// Test resize functionality
TEST_F(core_cpp_util_expanding_vector, ResizeVector) {
  vec.resize(5);
  EXPECT_EQ(vec.size(), 5);  // Size should be 5 after resizing

  vec[3].value = 99;
  vec[4].value = 100;

  EXPECT_EQ(vec[3].value, 99);
  EXPECT_EQ(vec[4].value, 100);

  vec.resize(2);  // Shrink the vector
  EXPECT_EQ(vec.size(), 2);  // Size should be 2 now
  EXPECT_THROW(vec.at(3), std::out_of_range);  // Accessing out of range should throw exception
}

// Test operator[] without bounds checking
TEST_F(core_cpp_util_expanding_vector, OperatorAccess) {
  EXPECT_NO_THROW(vec[1]);  // Access valid index without throwing exception
  EXPECT_EQ(vec[1].value, 2);  // Element at index 1 should have value 2

  vec[1].value = 42;
  EXPECT_EQ(vec[1].value, 42);  // After modification, value should be 42

  // No bounds checking in operator[], so accessing beyond size is allowed
  // but it's testing undefined behavior in C++
}

// Test at() function with bounds checking
TEST_F(core_cpp_util_expanding_vector, AtFunction) {
  EXPECT_NO_THROW(vec.at(0));
  EXPECT_EQ(vec.at(0).value, 1);

  EXPECT_THROW(vec.at(10), std::out_of_range);  // Out of bounds access should throw
}

// Test begin() and end() iterator functions
TEST_F(core_cpp_util_expanding_vector, Iterators) {
  auto it = vec.begin();
  EXPECT_EQ(it->value, 1);  // First element should have value 1
  ++it;
  EXPECT_EQ(it->value, 2);  // Second element should have value 2

  size_t count = 0;
  for (auto it2 = vec.begin(); it2 != vec.end(); ++it2) {
    ++count;
  }
  EXPECT_EQ(count, vec.size());  // Iterator loop should iterate over all valid elements
}

// Test const_iterator functionality
TEST_F(core_cpp_util_expanding_vector, ConstIterator) {
  const CExpandingVector<MyElement> constVec = vec;

  CExpandingVector<MyElement>::const_iterator it = constVec.begin();
  EXPECT_EQ(it->value, 1);  // First element in const vector should be 1
  ++it;
  EXPECT_EQ(it->value, 2);  // Second element in const vector should be 2

  size_t count = 0;
  for (auto it2 = constVec.begin(); it2 != constVec.end(); ++it2) {
    ++count;
  }
  EXPECT_EQ(count, constVec.size());  // Iterator loop should iterate over all valid elements
}

// Test empty() function
TEST_F(core_cpp_util_expanding_vector, EmptyFunction) {
  EXPECT_FALSE(vec.empty());  // Should not be empty initially

  vec.clear();
  EXPECT_TRUE(vec.empty());  // Should be empty after clear is called
}

// Test capacity and full_size functions
TEST_F(core_cpp_util_expanding_vector, FullSize) {
  EXPECT_EQ(vec.full_size(), 3);  // Full size is the size of the underlying vector

  vec.push_back(MyElement(4));
  EXPECT_EQ(vec.full_size(), 4);  // Full size is the new capacity

  vec.clear();
  EXPECT_EQ(vec.full_size(), 4);  // Full size is the new capacity
}


TEST_F(core_cpp_util_expanding_vector, Front) {
  EXPECT_EQ(vec.front().value, 1);  // The first element should have value 1

  vec.front().value = 10;  // Modify the first element
  EXPECT_EQ(vec.front().value, 10);  // Check if the modification worked
}

// Test back function
TEST_F(core_cpp_util_expanding_vector, Back) {
  EXPECT_EQ(vec.back().value, 3);  // The last element should have value 3

  vec.back().value = 20;  // Modify the last element
  EXPECT_EQ(vec.back().value, 20);  // Check if the modification worked

  vec.clear();
  vec.push_back(10);  // push back a new value
  EXPECT_EQ(vec.back().value, 10);  // Check if the modification worked
}

// Test front and back with an empty vector (should throw an exception)
TEST_F(core_cpp_util_expanding_vector, FrontAndBackEmptyVector) {
  CExpandingVector<MyElement> emptyVec;

  EXPECT_THROW(emptyVec.front(), std::out_of_range);  // Should throw because vector is empty
  EXPECT_THROW(emptyVec.back(), std::out_of_range);   // Should throw because vector is empty
}
