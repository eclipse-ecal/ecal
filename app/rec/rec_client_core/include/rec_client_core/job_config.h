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
#include <chrono>

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
      void SetJobId(int64_t job_id);
      int64_t GetJobId() const;
      void GenerateNewJobId();

      void SetMeasRootDir(const std::string& meas_root_dir);
      std::string GetMeasRootDir() const;

      void SetMeasName(const std::string& meas_name);
      std::string GetMeasName() const;

      void SetMaxFileSize(unsigned int max_file_size_mb);
      unsigned int GetMaxFileSize() const;

      void SetDescription(const std::string& description);
      std::string GetDescription() const;

    //////////////////////////////
    // Evaluation
    //////////////////////////////
    public:
      JobConfig CreateEvaluatedJobConfig(bool eval_target_functions, std::chrono::system_clock::time_point time = std::chrono::system_clock::now()) const;

      std::string GetCompleteMeasurementPath() const;

    //////////////////////////////
    // Member Variables
    //////////////////////////////
    private:
      int64_t      job_id_;

      std::string  meas_root_dir_;
      std::string  meas_name_;
      unsigned int max_file_size_mb_;
      std::string  description_;
    };
  }
}