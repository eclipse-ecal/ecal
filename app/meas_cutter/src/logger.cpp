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

#include "logger.h"

Logger Logger::instance;
Logger::Logger(){}

void Logger::openFile(const std::string& log_file_path) 
{
  instance.fileStream.open(log_file_path.c_str());
}
void Logger::close() 
{
  instance.fileStream.close();
}
void Logger::append(const std::string& log_message)
{
  std::ostream& stream = instance.fileStream;
  stream << log_message;
}