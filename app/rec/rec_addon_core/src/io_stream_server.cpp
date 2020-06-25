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

#include "io_stream_server.h"

IOStreamServer::IOStreamServer(std::istream& istream, std::ostream& ostream) :
  istream_(istream), ostream_(ostream)
{
}

void IOStreamServer::SetCallback(std::function<std::vector<std::string>(const std::string&)> callback)
{
  callback_ = callback;
}

void IOStreamServer::Listen()
{
  while (!istream_.eof() && !istream_.fail())
  {
    std::string request_line;
    std::getline(istream_, request_line);
    if (request_line.size())
    {
      for (const auto& response_line : callback_(request_line))
        ostream_ << response_line << std::endl;
    }
  }
}