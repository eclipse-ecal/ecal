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

#include <rec_core/job_config.h>

namespace eCAL
{
  namespace rec
  {
    //////////////////////////////
    // Constructor & Destructor
    //////////////////////////////

    JobConfig::JobConfig()
      : max_file_size_mb_(50)
    {}

    JobConfig::~JobConfig()
    {}

    //////////////////////////////
    // Getter & Setter
    //////////////////////////////

    void            JobConfig::SetMeasRootDir   (const std::string& meas_root_dir) { meas_root_dir_ = meas_root_dir; }
    std::string     JobConfig::GetMeasRootDir   () const                           { return meas_root_dir_; }

    void            JobConfig::SetMeasName      (const std::string& meas_name)     { meas_name_ = meas_name; }
    std::string     JobConfig::GetMeasName      () const                           { return meas_name_; }

    void            JobConfig::SetMaxFileSize   (size_t max_file_size_mb)          { max_file_size_mb_ = max_file_size_mb; }
    size_t          JobConfig::GetMaxFileSize   () const                           { return max_file_size_mb_; }

    void            JobConfig::SetDescription   (const std::string& description)   { description_ = description; }
    std::string     JobConfig::GetDescription   () const                           { return description_; }

    void            JobConfig::SetPreRecCommand (const std::string& pre_rec_cmd)   { pre_rec_cmd_ = pre_rec_cmd; }
    std::string     JobConfig::GetPreRecCommand () const                           { return pre_rec_cmd_; }

    void            JobConfig::SetPostRecCommand(const std::string& post_rec_cmd)  { post_rec_cmd_ = post_rec_cmd; }
    std::string     JobConfig::GetPostRecCommand() const                           { return post_rec_cmd_; }
  }
}