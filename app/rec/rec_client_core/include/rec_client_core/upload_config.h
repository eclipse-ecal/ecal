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
    struct UploadConfig
    {
      enum class Type
      {
        FTP
      };

      UploadConfig()
        : protocol_()
        , meas_id_(0)
        , port_(0)
        , upload_metadata_files_(true)
        , delete_after_upload_(false)
      {}

      Type protocol_;

      int64_t     meas_id_;

      std::string host_;
      uint16_t    port_;
      std::string username_;
      std::string password_;
      std::string upload_path_;
      bool        upload_metadata_files_;
      bool        delete_after_upload_;
    };
  }
}