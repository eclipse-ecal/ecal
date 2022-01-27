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

#include <ecal_utils/str_convert.h>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <Windows.h>
#endif // WIN32

#include <string>

namespace EcalUtils
{
  namespace StrConvert
  {
#ifdef WIN32
    std::string WideToAnsi(const std::wstring& wstr,  unsigned int ansi_code_page)
    {
        int count = WideCharToMultiByte(ansi_code_page, 0, wstr.c_str(), static_cast<int>(wstr.length()), NULL, 0, NULL, NULL);
        std::string str(count, 0);
        WideCharToMultiByte(ansi_code_page, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
        return str;
    }

    std::wstring AnsiToWide(const std::string& str,  unsigned int ansi_code_page)
    {
        int count = MultiByteToWideChar(ansi_code_page, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
        std::wstring wstr(count, 0);
        MultiByteToWideChar(ansi_code_page, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], count);
        return wstr;
    }

    std::string WideToUtf8(const std::wstring& wstr)
    {
        int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), NULL, 0, NULL, NULL);
        std::string str(count, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
        return str;
    }

    std::wstring Utf8ToWide(const std::string& str)
    {
        int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
        std::wstring wstr(count, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], count);
        return wstr;
    }

    std::string AnsiToUtf8(const std::string& str, unsigned int ansi_code_page)
    {
      return WideToUtf8(AnsiToWide(str, ansi_code_page));
    }

    std::string Utf8ToAnsi(const std::string& str, unsigned int ansi_code_page)
    {
      return WideToAnsi(Utf8ToWide(str), ansi_code_page);
    }
#endif
  }
}