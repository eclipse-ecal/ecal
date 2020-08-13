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

#include "osselect.h"

#include <vector>
#include <ecal_utils/string.h>

namespace EcalParser
{
  std::string FunctionOsselect::Evaluate(const std::string& parameters, std::chrono::system_clock::time_point /*time*/) const
  {
    std::vector<std::string> parameter_vector;
    EcalUtils::String::SplitQuotedString(parameters, parameter_vector, '\0', true, true);
    
    int default_param_index = -1; // Index of the DEFAULT element
    int os_index = 0;
    
    while (os_index < ((int)parameter_vector.size() - 1))
    {
      std::string os = parameter_vector[os_index];
      std::transform(os.begin(), os.end(), os.begin(), [](char c) {return static_cast<char>(::tolower(c)); });

      if (os == "default")
      {
        // If we find a DEFAULT element, we store the index, just in case we  don't find a correct os element for this machine
        default_param_index = os_index;
      }
      else
      {
#if defined _WIN32 || defined _WIN64
        if (os == "win" || os == "windows")
          break;
#elif __ANDROID__ 
        if (os == "android")
          break;
#elif __linux__ 
        if (os == "linux")
          break;
#elif __APPLE__ && __MACH__
        if (os == "mac" || os == "macos")
          break;
#elif __FreeBSD__
        if (os == "freebsd")
          break;
#elif __NetBSD__
        if (os == "netbsd")
          break;
#elif __OpenBSD__
        if (os == "openbsd")
          break;
#endif
      }
    
      os_index += 2;
    }
    
    if (os_index < ((int)parameter_vector.size() - 1))
    {
      return parameter_vector[os_index + 1];
    }
    else if (default_param_index > 0)
    {
      return parameter_vector[default_param_index + 1];
    }
    else
    {
      return "";
    }
  }

  std::string FunctionOsselect::ParameterUsage   () const { return "<OS_1> \"<val1>\" <OS_2> \"<val2>\" ... DEFAULT \"<default>\""; }

  std::string FunctionOsselect::ParameterExample() const { return "WIN \"C:\\Windows\\System32\\cmd.exe\" LINUX \"/usr/bin/xterm\" MAC \"/Applications/Utilities/Terminal.app\" DEFAULT \"Where am I running on?\""; }
  std::string FunctionOsselect::Description     () const { return "OS depending output"; }
  std::string FunctionOsselect::HtmlDocumentation        () const
  {
    return R"(
<p>
Evaluates to the string given for the specific OS. The OS specifiers are case-insensitive. Supported OS specifiers are:
</p>
<p>
 <ul>
  <li>WIN</li>
  <li>WINDOWS</li>
  <li>LINUX</li>
  <li>MAC</li>
  <li>MACOS</li>
  <li>ANDROID</li>
  <li>FREEBSD</li>
  <li>NETBSD</li>
  <li>OPENBSD</li>
  <li>DEFAULT</li>
</ul> 
</p>
<p>
Note that you should always enclose the output in quotation marks (" "). The quotation marks will be automatically removed for evaluation.
</p>
<p>
All elements are optional (including the DEFAULT element). If the OS does not match any given OS specifiers, this function will evaluate to the DEFAULT element, if present. It will otherwise evaluate to nothing.
</p>
)";
  }
}
