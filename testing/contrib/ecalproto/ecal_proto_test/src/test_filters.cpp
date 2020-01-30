/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * Tests eCALHDF5 reader w/o input file (single channel)
**/

#include <string>

#include "ecal/protobuf/ecal_proto_message_filter.h"

#include "gtest/gtest.h"

using namespace eCAL::protobuf;

TEST(filter, nofilter)
{
  NoFilter filter;
  bool result = filter.Filter("abc");
  EXPECT_EQ(true, result);
}

TEST(filter, include_filter_clear)
{
  SimpleIncludeFilter filter;
  std::string a("a");
  filter.Insert(a);
  EXPECT_EQ(true, filter.Filter(a));

  filter.Clear();
  EXPECT_EQ(false, filter.Filter(a));
}

void test_regular_includes(BaseIncludeFilter& filter)
{
  std::string abc("a.b.c");
  std::string ab("a.b");
  std::string a("a");
  std::string ad("a.d");
  std::string ade("a.d.e");
  std::string adf("a.d.f");
  EXPECT_EQ(false, filter.Filter(abc));
  filter.Insert(abc);
  EXPECT_EQ(true, filter.Filter(a));
  EXPECT_EQ(true, filter.Filter(ab));
  EXPECT_EQ(true, filter.Filter(abc));
  filter.Erase(abc);
  EXPECT_EQ(false, filter.Filter(a));
  EXPECT_EQ(false, filter.Filter(ab));
  EXPECT_EQ(false, filter.Filter(abc));
  filter.Insert(ade);
  filter.Insert(abc);
  filter.Insert(adf);
  filter.Erase(ade);
  EXPECT_EQ(true, filter.Filter(a));
  EXPECT_EQ(true, filter.Filter(ab));
  EXPECT_EQ(true, filter.Filter(abc));
  EXPECT_EQ(true, filter.Filter(ad));
  EXPECT_EQ(false, filter.Filter(ade));
  EXPECT_EQ(true, filter.Filter(adf));
  filter.Erase(abc);
  EXPECT_EQ(true, filter.Filter(a));
  EXPECT_EQ(false, filter.Filter(ab));
  EXPECT_EQ(false, filter.Filter(abc));
  EXPECT_EQ(true, filter.Filter(ad));
  EXPECT_EQ(false, filter.Filter(ade));
  EXPECT_EQ(true, filter.Filter(adf));
  filter.Erase(adf);
  EXPECT_EQ(false, filter.Filter(a));
  EXPECT_EQ(false, filter.Filter(ab));
  EXPECT_EQ(false, filter.Filter(abc));
  EXPECT_EQ(false, filter.Filter(ad));
  EXPECT_EQ(false, filter.Filter(ade));
  EXPECT_EQ(false, filter.Filter(adf));
}

TEST(filter, include_filter)
{
  SimpleIncludeFilter simple_filter;
  test_regular_includes(simple_filter);
  ComplexIncludeFilter complex_filter;
  test_regular_includes(complex_filter);
}

TEST(filter, include_filter_array_simple)
{
  const std::string a("a");
  const std::string a1("a[1]");
  const std::string a2("a[2]");
  const std::string a1b("a[1].b");
  const std::string a2b("a[2].b");
  const std::string a1bc1("a[1].b.c[1]");

  ComplexIncludeFilter filter;
  filter.Insert(a1);                   // Add a[1]
  EXPECT_EQ(true, filter.Filter(a));  // should accept a
  EXPECT_EQ(true, filter.Filter(a1)); // should accept a[1]
  EXPECT_EQ(false, filter.Filter(a2)); // should not accept a[2]

                                        // Test subsequently, all combinations to that point are included
  filter.Clear();
  filter.Insert(a1bc1);
  EXPECT_EQ(true, filter.Filter(a));
  EXPECT_EQ(true, filter.Filter(a1));
  EXPECT_EQ(true, filter.Filter(a1b));
  EXPECT_EQ(true, filter.Filter(a1bc1));

  filter.Clear();
  filter.Insert(a1b);                   // add a[1].b
  EXPECT_EQ(true, filter.Filter(a));   // should accept a
  EXPECT_EQ(true, filter.Filter(a1));  // should accept a[1]
  EXPECT_EQ(true, filter.Filter(a1b)); // should accept a[1].b
  EXPECT_EQ(false, filter.Filter(a2));  // should not accept a[2]
  EXPECT_EQ(false, filter.Filter(a2b)); // should not accept a[2].b
}

TEST(filter, include_filter_array_complex)
{
  const std::string a    ("a"          );
  const std::string a1   ("a[1]"       );
  const std::string a2   ("a[2]"       );
  const std::string a_   ("a[*]"       );
  const std::string a1b  ("a[1].b"     );
  const std::string a2b  ("a[2].b"     );
  const std::string a_b  ("a[*].b"     );
  const std::string a1bc_("a[1].b.c[*]");
  const std::string a1bc1("a[1].b.c[1]");

  ComplexIncludeFilter filter;
  filter.Insert(a1);                   // Add a[1]
  EXPECT_EQ(true,  filter.Filter(a));  // should accept a
  EXPECT_EQ(true,  filter.Filter(a1)); // should accept a[1]
  EXPECT_EQ(false, filter.Filter(a2)); // should not accept a[2]
  EXPECT_EQ(false, filter.Filter(a_)); // should not accept a[*]

  filter.Clear();
  filter.Insert(a_);                   // Add a[*]
  EXPECT_EQ(true, filter.Filter(a));   // should accept a
  EXPECT_EQ(true, filter.Filter(a1));  // should accept a[1]
  EXPECT_EQ(true, filter.Filter(a2));  // should accept a[2]
  EXPECT_EQ(true, filter.Filter(a_));  // should accept a[*]

  filter.Clear();
  filter.Insert(a_b);                   // Add a[*].b
  EXPECT_EQ(true, filter.Filter(a1b));  // should accept a[1].b
  EXPECT_EQ(true, filter.Filter(a_b));  // should accept a[*].b

  // Test subsequently, all combinations to that point are included
  filter.Clear();
  filter.Insert(a1bc1); 
  EXPECT_EQ(true, filter.Filter(a));
  EXPECT_EQ(true, filter.Filter(a1));
  EXPECT_EQ(true, filter.Filter(a1b));
  EXPECT_EQ(true, filter.Filter(a1bc1));

  filter.Clear();
  filter.Insert(a1bc_);
  EXPECT_EQ(true, filter.Filter(a));
  EXPECT_EQ(true, filter.Filter(a1));
  EXPECT_EQ(true, filter.Filter(a1b));
  EXPECT_EQ(true, filter.Filter(a1bc1));

  filter.Clear();
  filter.Insert(a_b);                  // add a[*].b
  EXPECT_EQ(true, filter.Filter(a1));  // should accept a[1]
  EXPECT_EQ(true, filter.Filter(a2));  // should accept a[2]
  EXPECT_EQ(true, filter.Filter(a_));  // should accept a[*]

  filter.Clear();
  filter.Insert(a1b);                   // add a[1].b
  EXPECT_EQ(true,  filter.Filter(a));   // should accept a
  EXPECT_EQ(true,  filter.Filter(a1));  // should accept a[1]
  EXPECT_EQ(true,  filter.Filter(a1b)); // should accept a[1].b
  EXPECT_EQ(false, filter.Filter(a2));  // should not accept a[2]
  EXPECT_EQ(false, filter.Filter(a2b)); // should not accept a[2].b
  EXPECT_EQ(false, filter.Filter(a_));  // should not accept a[*]
  EXPECT_EQ(false, filter.Filter(a_b)); // should not accept a[*].b
}