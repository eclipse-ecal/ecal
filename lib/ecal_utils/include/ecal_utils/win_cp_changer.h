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

#ifdef WIN32

namespace EcalUtils
{
  // RAII Class for changing the Console Codepage
  class WinCpChanger
  {
  public:
    // Constructor
    WinCpChanger(const int target_cp);

    // Copy (Delted -> no copy constructor)
    WinCpChanger(const WinCpChanger&)            = delete;
    WinCpChanger& operator=(const WinCpChanger&) = delete;

    // Move
    WinCpChanger& operator=(WinCpChanger&&)      = default;
    WinCpChanger(WinCpChanger&&)                 = default;

    // Destructor
    ~WinCpChanger();

  private:
    int initial_cp;
  };
}

#endif