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

#include <rec_client_core/job_config.h>

#include <EcalParser/EcalParser.h>
#include <ecal_utils/filesystem.h>

namespace eCAL
{
  namespace rec
  {
    //////////////////////////////
    // Constructor & Destructor
    //////////////////////////////

    JobConfig::JobConfig()
      : job_id_(0)
      , max_file_size_mb_(50)
    {}

    JobConfig::~JobConfig()
    {}

    //////////////////////////////
    // Getter & Setter
    //////////////////////////////

    void            JobConfig::SetJobId         (int64_t job_id)                   { job_id_ = job_id; }
    int64_t         JobConfig::GetJobId         () const                           { return job_id_; }

    void JobConfig::GenerateNewJobId()
    {
      // The system clock gives us a nice timestamp that may however lack precision on some systems
      auto system_clock_now = std::chrono::system_clock::now();
      int64_t system_clock_nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(system_clock_now.time_since_epoch()).count();

      // The steady clock gives us a (hopefully) monotonic timestamp. But it will start over when rebooting.
      auto steady_clock_now = std::chrono::steady_clock::now();
      int64_t steady_clock_nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(steady_clock_now.time_since_epoch()).count();

      // A static counter will just increase when creating new IDs
      static uint32_t counter = 0;
      counter++;

      job_id_ = (((int64_t)counter) << 32) | (int64_t(0xFFFFF000) & system_clock_nsecs) | (int64_t(0x00000FFF) & steady_clock_nsecs);
    }

    void            JobConfig::SetMeasRootDir   (const std::string& meas_root_dir) { meas_root_dir_ = meas_root_dir; }
    std::string     JobConfig::GetMeasRootDir   () const                           { return meas_root_dir_; }

    void            JobConfig::SetMeasName      (const std::string& meas_name)     { meas_name_ = meas_name; }
    std::string     JobConfig::GetMeasName      () const                           { return meas_name_; }

    void            JobConfig::SetMaxFileSize   (unsigned int max_file_size_mb)    { max_file_size_mb_ = max_file_size_mb; }
    unsigned int    JobConfig::GetMaxFileSize   () const                           { return max_file_size_mb_; }

    void            JobConfig::SetDescription   (const std::string& description)   { description_ = description; }
    std::string     JobConfig::GetDescription   () const                           { return description_; }

    //////////////////////////////
    // Evaluation
    //////////////////////////////
    JobConfig JobConfig::CreateEvaluatedJobConfig(bool eval_target_functions, std::chrono::system_clock::time_point time) const
    {
      JobConfig evaluated_config(*this);
      evaluated_config.SetMeasRootDir(EcalParser::Evaluate(evaluated_config.GetMeasRootDir(), eval_target_functions, time));
      evaluated_config.SetMeasName   (EcalParser::Evaluate(evaluated_config.GetMeasName(),    eval_target_functions, time));
      evaluated_config.SetDescription(EcalParser::Evaluate(evaluated_config.GetDescription(), eval_target_functions, time));
      return evaluated_config;
    }


    std::string JobConfig::GetCompleteMeasurementPath() const
    {
      std::string joined_meas_path = GetMeasRootDir();

      if ((!joined_meas_path.empty() && (joined_meas_path[joined_meas_path.size() - 1] != '\\') && (joined_meas_path[joined_meas_path.size() - 1] != '/'))
        && (!GetMeasName().empty() && (GetMeasName()[0] != '\\') && (GetMeasName()[0] != '/')))
      {
        joined_meas_path += "/";
      }
      joined_meas_path += GetMeasName();

      return EcalUtils::Filesystem::AbsolutePath(joined_meas_path);
    }
  }
}