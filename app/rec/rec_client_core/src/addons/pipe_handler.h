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

#include <string>

class PipeHandler
{
public:
  PipeHandler();
  ~PipeHandler();

  bool StartProcess(const std::string& executable_path);
  bool StartProcess(const std::wstring& executable_path);

  bool WriteLine(const std::string& line);
  std::string ReadLine();

  bool IsProcessAlive() const;
  bool StopProcess();

  //bool isMessageAvailable();

private:
  std::string NativeReadSome();
  
private:
  std::string input_residue_;
#ifdef WIN32
  void* child_stdin_rd_;
  void* child_stdin_wr_;
  void* child_stdout_rd_;
  void* child_stdout_wr_;

  mutable void *process_;
#else
  static const int PIPE_READ  = 0;
  static const int PIPE_WRITE = 1;
  
  int child_stdin_pipe_fd_[2];
  int child_stdout_pipe_fd_[2];

  mutable int pid_;
#endif
};

