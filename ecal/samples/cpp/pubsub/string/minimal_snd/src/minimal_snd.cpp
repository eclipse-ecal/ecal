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

#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>

#include <iostream>
#include <chrono>

class CDescGate
{
public:
  CDescGate(const std::chrono::milliseconds& exp_timeout_)
  {
    std::cout << "Constructor" << std::endl;
  }
  void Foo()
  {
    std::cout << "Foo" << std::endl;
  }
};

int main(int argc, char **argv)
{
  long long time_ms(5000);
  CDescGate desc_gate(std::chrono::milliseconds(time_ms));
  desc_gate.Foo();
  return(0);
}
