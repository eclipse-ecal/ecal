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

#include "env.h"

#include <ecal_utils/string.h>

namespace EcalParser
{
  std::string FunctionEnv::Evaluate(const std::string& parameters, std::chrono::system_clock::time_point /*time*/) const
  {
    const char* env_var = std::getenv(EcalUtils::String::Trim(parameters).c_str());
    if (env_var)
    {
      return std::string(env_var);
    }
    else
    {
      return "";
    }
  }

  std::string FunctionEnv::ParameterUsage   () const { return "<VariableName>"; }

  std::string FunctionEnv::ParameterExample() const
  {
#ifdef WIN32
    return "userprofile";
#else // WIN32
    return "HOME";
#endif // WIN32
  }

  std::string FunctionEnv::Description     () const { return "Environment variable"; }
  std::string FunctionEnv::HtmlDocumentation        () const
  {
    return "<p>Evaluates to the given environment variable. If there is no variable with that name, this function evaluates to nothing.</p>\n<p>The case-sensitiveness depends on the operating system.</p>";
  }
}




