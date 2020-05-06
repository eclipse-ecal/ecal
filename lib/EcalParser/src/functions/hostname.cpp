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

#include "hostname.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <WinSock2.h>
#else // WIN32
#include <unistd.h>
#endif // WIN32

namespace EcalParser
{
  std::string FunctionHostname::Evaluate(const std::string& /*parameters*/, std::chrono::system_clock::time_point /*time*/) const
  {
    static std::string hostname;
    
    if (hostname.empty())
    {
#ifdef WIN32
      WORD wVersionRequested = MAKEWORD(2, 2);

      WSADATA wsaData;
      int err = WSAStartup(wVersionRequested, &wsaData);
      if (err != 0)
      {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
      }
#endif // WIN32

      char hostname_char[1024] = { 0 };
      if (gethostname(hostname_char, 1024) == 0)
      {
        hostname = hostname_char;
      }
    }
    
    return hostname;
  }

  std::string FunctionHostname::ParameterUsage   () const { return ""; }

  std::string FunctionHostname::ParameterExample() const { return ""; }
  std::string FunctionHostname::Description     () const { return "Hostname"; }
  std::string FunctionHostname::HtmlDocumentation        () const { return "<p>Evaluates to the hostname of this machine / the target machine.</p>"; }
}