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

#include "username.h"

#ifdef WIN32
  #include <windows.h>
  #include <Lmcons.h>
#elif __linux__
  #include <unistd.h>
  #include <pwd.h>
#else
  #include <cstdlib>
#endif


namespace EcalParser
{
  std::string FunctionUsername::Evaluate(const std::string& /*parameters*/, std::chrono::system_clock::time_point /*time*/) const
  {
#if defined _WIN32 || defined _WIN64
    char username[UNLEN+1];
    DWORD username_len = UNLEN+1;

    if (GetUserName(username, &username_len))
      return std::string(username);
    else
      return "";

#elif __linux__ 
    uid_t uid = geteuid ();
    struct passwd *pw = getpwuid (uid);

    if (pw)
      return std::string(pw->pw_name);
    else
      return "";

#else // Let's hope that the USER variable is set. Should work for many Unix Style OSs
    const char* env_var = std::getenv("USER");
    if (env_var)
    {
      return std::string(env_var);
    }
    else
    {
      return "";
    }
#endif 
  }

  std::string FunctionUsername::ParameterUsage   () const { return ""; }

  std::string FunctionUsername::ParameterExample() const { return ""; }

  std::string FunctionUsername::Description     () const { return "Username"; }
  std::string FunctionUsername::HtmlDocumentation        () const
  {
    return R"(
<p>
Evaluates to the username of the user owning the process.
</p>
)";
  }
}