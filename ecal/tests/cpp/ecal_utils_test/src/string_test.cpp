#include <gtest/gtest.h>

#include "ecal_utils/filesystem.h"
#include "ecal_utils/string.h"

#include <string>
#include <vector>

TEST(ecal_utils_cpp /*unused*/, join_paths /*unused*/)
{ 
  std::vector<std::string> string_vec;
  // Test the JoinPaths function
  
  string_vec.push_back("");
  if (EcalUtils::Filesystem::OsStyle::Current == EcalUtils::Filesystem::OsStyle::Windows)
  {
    string_vec[0] = "C:";
  }
  string_vec.push_back("Users");
  string_vec.push_back("User");
  string_vec.push_back("Documents");
  string_vec.push_back("ecal");
  string_vec.push_back("test");
  string_vec.push_back("src");
  string_vec.push_back("string_test.cpp");

  // path with trailing file
  std::string expected_path;
  char path_separator = EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current);
  
  expected_path = string_vec[0] + path_separator + string_vec[1] + path_separator + string_vec[2] + path_separator 
                + string_vec[3] + path_separator + string_vec[4] + path_separator + string_vec[5] + path_separator 
                + string_vec[6] + path_separator + string_vec[7];

  std::string joined_path = EcalUtils::String::Join(std::string(1, path_separator), string_vec);
  EXPECT_EQ(joined_path, expected_path);

  // path without trailing file
  string_vec[7] = "string";
  expected_path = string_vec[0] + path_separator + string_vec[1] + path_separator + string_vec[2] + path_separator 
                + string_vec[3] + path_separator + string_vec[4] + path_separator + string_vec[5] + path_separator 
                + string_vec[6] + path_separator + string_vec[7];
  
  joined_path = EcalUtils::String::Join(std::string(1, path_separator), string_vec);
  EXPECT_EQ(joined_path, expected_path);

}