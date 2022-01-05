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

#include <string>

namespace EcalUtils
{
  namespace StrConvert
  {
#ifdef WIN32
    std::string WideToAnsi(const std::wstring& wstr, unsigned int ansi_code_page);

    std::wstring AnsiToWide(const std::string& str, unsigned int ansi_code_page);

    std::string WideToUtf8(const std::wstring& wstr);

    std::wstring Utf8ToWide(const std::string& str);

    std::string AnsiToUtf8(const std::string& str, unsigned int ansi_code_page);

    std::string Utf8ToAnsi(const std::string& str, unsigned int ansi_code_page);
#endif // WIN32
  }
}