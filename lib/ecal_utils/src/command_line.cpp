/* ========================= eCAL LICENSE =================================
*
* Copyright (C) 2021 Continental Corporation
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

#include "ecal_utils/command_line.h"

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <Windows.h>
  #include <shellapi.h>
#endif // WIN32

#include <ecal_utils/str_convert.h>
#include <iostream>

namespace EcalUtils
{
  namespace CommandLine
  {

#ifdef WIN32
    std::vector<std::string> GetUtf8Argv()
    {
      int commandline_w_argc(0);

      std::vector<std::string> utf8_argv_vector;

      LPWSTR* wstring_arg_list_ptr = CommandLineToArgvW(GetCommandLineW(), &commandline_w_argc);
      if (wstring_arg_list_ptr == nullptr)
      {
        std::cerr << "CommandLineToArgvW failed" << std::endl;
      }
      else
      {
        // Fill the vector with UTF8 conversions of Windows wide-strings
        utf8_argv_vector.reserve(commandline_w_argc);
        for (int i = 0; i < commandline_w_argc; i++)
        {
          utf8_argv_vector.push_back(EcalUtils::StrConvert::WideToUtf8(std::wstring(wstring_arg_list_ptr[i])));
        }
        LocalFree(wstring_arg_list_ptr);
      }

      return utf8_argv_vector;
    }

    std::string GetUtf8CommandLine()
    {
      return EcalUtils::StrConvert::WideToUtf8(GetCommandLineW());
    }
#endif

  }
}