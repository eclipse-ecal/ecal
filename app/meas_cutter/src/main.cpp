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

#include "ecal_meas_cutter.h"

#ifdef WIN32
int main()
{
  SetConsoleOutputCP(CP_UTF8);
#else
int main(int argc, char* argv[])
{
#endif // WIN32

  std::vector<std::string> cmd_arguments;
#ifdef WIN32
  cmd_arguments = EcalUtils::CommandLine::GetUtf8Argv();
#else
  cmd_arguments.insert(cmd_arguments.end(), argv, argv + argc);
#endif // WIN32

  eCALMeasCutter app(cmd_arguments);
  return app.run();
}
  

