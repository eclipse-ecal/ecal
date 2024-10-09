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

#include "registration/ecal_registration_sample_applier_gates.h"
#include "ecal_global_accessors.h"
#include "pubsub/ecal_subgate.h"
#include "pubsub/ecal_pubgate.h"
#include "service/ecal_clientgate.h"

#include <ecal/ecal_log.h>

namespace eCAL
{
  namespace Registration
  {
    void CSampleApplierGates::ApplySample(const eCAL::Registration::Sample& sample_)
    {
      switch (sample_.cmd_type)
      {
      case bct_none:
      case bct_set_sample:
      case bct_reg_process:
      case bct_unreg_process:
        break;
#if ECAL_CORE_SERVICE
      case bct_reg_service:
        if (g_clientgate() != nullptr) g_clientgate()->ApplyServiceRegistration(sample_);
        break;
#endif
      case bct_unreg_service:
        break;
      case bct_reg_client:
      case bct_unreg_client:
        // current client implementation doesn't need that information
        break;
#if ECAL_CORE_PUBLISHER
      case bct_reg_subscriber:
        if (g_pubgate() != nullptr) g_pubgate()->ApplySubRegistration(sample_);
        break;
      case bct_unreg_subscriber:
        if (g_pubgate() != nullptr) g_pubgate()->ApplySubUnregistration(sample_);
        break;
#endif
#if ECAL_CORE_SUBSCRIBER
      case bct_reg_publisher:
        if (g_subgate() != nullptr) g_subgate()->ApplyPubRegistration(sample_);
        break;
      case bct_unreg_publisher:
        if (g_subgate() != nullptr) g_subgate()->ApplyPubUnregistration(sample_);
        break;
#endif
      default:
        Logging::Log(log_level_debug1, "CGatesApplier::ApplySample : unknown sample type");
        break;
      }
    }
  }
}