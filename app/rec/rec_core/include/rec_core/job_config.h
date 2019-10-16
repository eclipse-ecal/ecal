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

namespace eCAL
{
  namespace rec
  {
    class JobConfig
    {
    //////////////////////////////
    // Constructor & Destructor
    //////////////////////////////
    public:
      JobConfig();
      ~JobConfig();

    //////////////////////////////
    // Getter & Setter
    //////////////////////////////
    public:
      void SetMeasRootDir(const std::string& meas_root_dir);
      std::string GetMeasRootDir() const;

      void SetMeasName(const std::string& meas_name);
      std::string GetMeasName() const;

      void SetMaxFileSize(size_t max_file_size_mb);
      size_t GetMaxFileSize() const;

      void SetDescription(const std::string& description);
      std::string GetDescription() const;

      void SetPreRecCommand(const std::string& pre_rec_cmd);
      std::string GetPreRecCommand() const;

      void SetPostRecCommand(const std::string& post_rec_cmd);
      std::string GetPostRecCommand() const;

    //////////////////////////////
    // Member Variables
    //////////////////////////////
    private:
      std::string meas_root_dir_;
      std::string meas_name_;
      size_t      max_file_size_mb_;
      std::string description_;

      std::string pre_rec_cmd_;
      std::string post_rec_cmd_;
    };
  }
}