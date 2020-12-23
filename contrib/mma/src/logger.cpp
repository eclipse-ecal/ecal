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

#include "../include/logger.h"

#include <string.h>

Logger* Logger::this_ = nullptr;
std::ofstream Logger::log_file_;

Logger::Logger()
  :logging_active_(false)
{
}

Logger::~Logger()
{
}

Logger* Logger::getLogger()
{
  if (this_ == nullptr)
    this_ = new Logger();

  return this_;
}


void Logger::PauseLogging()
{
  logging_active_ = false;
}


void Logger::ResumeLogging()
{
  logging_active_ = true;

  if (log_file_.is_open() == false)
  {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%Y%m%d_%H%M%S", timeinfo);

    std::string filename = std::string("Log_MMA_") + std::string(buffer) + std::string(".txt");
    log_file_.open(filename.c_str(), std::ios::out | std::ios::app);
  }
}

void Logger::Log(const std::string& message)
{
  if (logging_active_ == true && log_file_.is_open() == true)
  {
    log_file_ << "\n" << GetDateTime() << ": ";
    log_file_ << message;
  }
}

void Logger::Log(const char* message)
{
  if (logging_active_ == true && log_file_.is_open() == true)
  {
    log_file_ << "\n" << GetDateTime() << ": ";
    log_file_ << message;
  }
}

Logger& Logger::operator<<(const std::string& message)
{
  if (logging_active_ == true && log_file_.is_open() == true)
  {
    log_file_ << "\n" << GetDateTime() << ": ";
    log_file_ << message;
  }

  return *this;
}

std::string Logger::GetDateTime() const
{
  // current date/time based on current system
  time_t now = time(0);
  // convert now to string form
  char* ctime_no_newline;
  ctime_no_newline = strtok(ctime(&now), "\n");

  return std::string(ctime_no_newline);
}
