/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <ecal_c/auxiliaries.h>
#include "common.h"

extern "C"
{
  ECALC_API void eCAL_STopicId_Free(struct eCAL_STopicId* topic_id_)
  {
    if (topic_id_ != NULL)
    {
      Free_STopicId(topic_id_);
      std::free(reinterpret_cast<void*>(topic_id_));
    }
  }

  ECALC_API void eCAL_SDataTypeInformation_Free(struct eCAL_SDataTypeInformation* data_type_information_)
  {
    if (data_type_information_ != NULL)
    {
      Free_SDataTypeInformation(data_type_information_);
      std::free(reinterpret_cast<void*>(data_type_information_));
    }
  }

  ECALC_API void eCAL_Publisher_Configuration_Free(struct eCAL_Publisher_Configuration* publisher_configuration_)
  {
    std::free(reinterpret_cast<void*>(publisher_configuration_->layer_priority_local));
    std::free(reinterpret_cast<void*>(publisher_configuration_->layer_priority_remote));
    std::free(reinterpret_cast<void*>(publisher_configuration_));
  }

  ECALC_API void eCAL_Subscriber_Configuration_Free(struct eCAL_Subscriber_Configuration* subscriber_configuration_)
  {
  }

  ECALC_API void eCAL_SServiceId_Free(struct eCAL_SServiceId* service_id_)
  {
    if (service_id_ != NULL)
    {
      Free_SServiceId(service_id_);
      std::free(service_id_);
    }
  }

  ECALC_API void eCAL_SEntityId_Free(struct eCAL_SEntityId* entity_id_)
  {
    if (entity_id_ != NULL)
    {
      Free_SEntityId(entity_id_);
      std::free(reinterpret_cast<void*>(entity_id_));
    }
  }
}