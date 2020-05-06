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

#include "os.h"

#include <ecal_utils/string.h>

namespace EcalParser
{
  std::string FunctionOs::Evaluate(const std::string& /*parameters*/, std::chrono::system_clock::time_point /*time*/) const
  {
#if defined _WIN32 || defined _WIN64
    return "Windows";
#elif __ANDROID__ 
    return "Android";
#elif __linux__ 
    return "Linux";
#elif __APPLE__ && __MACH__
    return "MacOS";
#elif __FreeBSD__
    return "FreeBSD";
#elif __NetBSD__
    return "NetBSD";
#elif __OpenBSD__
    return "OpenBSD";
#endif
  }

  std::string FunctionOs::ParameterUsage   () const { return ""; }

  std::string FunctionOs::ParameterExample() const { return ""; }

  std::string FunctionOs::Description     () const { return "Operating system"; }
  std::string FunctionOs::HtmlDocumentation        () const
  {
    return R"(
<p>
Evaluates to the operating system of the host / target. Possible outputs are:
</p>
<p>
 <ul>
  <li>Windows</li>
  <li>Linux</li>
  <li>MacOS</li>
  <li>Android</li>
  <li>FreeBSD</li>
  <li>NetBSD</li>
  <li>OpenBSD</li>
</ul> 
</p>
)";
  }
}