/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "registration/ecal_registration_sample_applier_user.h"
#include "serialization/ecal_serialize_sample_registration.h"

namespace eCAL
{
  namespace Registration
  {
    bool CSampleApplierUser::AddRegistrationCallback(eCAL_Registration_Event event_, const RegistrationCallbackT& callback_)
    {
      switch (event_)
      {
      case reg_event_publisher:
        m_callback_pub = callback_;
        return true;
      case reg_event_subscriber:
        m_callback_sub = callback_;
        return true;
      case reg_event_service:
        m_callback_service = callback_;
        return true;
      case reg_event_client:
        m_callback_client = callback_;
        return true;
      case reg_event_process:
        m_callback_process = callback_;
        return true;
      default:
        return false;
      }
    }

    bool CSampleApplierUser::RemRegistrationCallback(eCAL_Registration_Event event_)
    {
      switch (event_)
      {
      case reg_event_publisher:
        m_callback_pub = nullptr;
        return true;
      case reg_event_subscriber:
        m_callback_sub = nullptr;
        return true;
      case reg_event_service:
        m_callback_service = nullptr;
        return true;
      case reg_event_client:
        m_callback_client = nullptr;
        return true;
      case reg_event_process:
        m_callback_process = nullptr;
        return true;
      default:
        return false;
      }
    }

    void CSampleApplierUser::ApplySample(const eCAL::Registration::Sample& sample_)
    {
      RegistrationCallbackT reg_callback(nullptr);
      switch (sample_.cmd_type)
      {
      case bct_none:
      case bct_set_sample:
        break;
      case bct_reg_process:
      case bct_unreg_process:
        // unregistration event not implemented currently
        reg_callback = m_callback_process;
        break;
      case bct_reg_service:
      case bct_unreg_service:
        reg_callback = m_callback_service;
        break;
      case bct_reg_client:
      case bct_unreg_client:
        // current client implementation doesn't need that information
        reg_callback = m_callback_client;
        break;
      case bct_reg_subscriber:
      case bct_unreg_subscriber:
        reg_callback = m_callback_sub;
        break;
      case bct_reg_publisher:
      case bct_unreg_publisher:
        reg_callback = m_callback_pub;
        break;
      default:
        break;
      }

      // call user registration callback
      if (reg_callback)
      {
        std::string reg_sample;
        if (SerializeToBuffer(sample_, reg_sample))
        {
          reg_callback(reg_sample.c_str(), static_cast<int>(reg_sample.size()));
        }
      }
    }
  }
}