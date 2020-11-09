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

#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

class Logger
{
public:
  /**
  *   Logs a message
  *   @param message message to be logged.
  */
  void Log(const std::string& message);

  /**
  *   Variable Length Logger function
  *   @param format string for the message to be logged.
  */
  void Log(const char* message);

  /**
  *   << overloaded function to Logs a message
  *   @param sMessage message to be logged.
  */
  Logger& operator<<(const std::string& message);

  static Logger* getLogger();

  void PauseLogging();
  void ResumeLogging();

private:
  Logger();
  Logger(const Logger&);
  ~Logger();
  Logger& operator =(const Logger&);

  std::string GetDateTime() const;
  bool logging_active_;

  static Logger* this_;
  static std::ofstream log_file_;
};

