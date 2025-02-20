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

/**
 * @file   ecal_c/pubsub/publisher.h
 * @brief  eCAL publisher c interface
**/

#ifndef ecal_c_pubsub_publisher_h_included
#define ecal_c_pubsub_publisher_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  typedef struct eCAL_Publisher eCAL_Publisher;
  typedef struct eCAL_PayloadWriter eCAL_PayloadWriter;

  enum eCAL_TransportLayer_eType
  {
    eCAL_TransportLayer_eType_none,
    eCAL_TransportLayer_eType_udp_mc,
    eCAL_TransportLayer_eType_shm,
    eCAL_TransportLayer_eType_tcp,
  };

  struct eCAL_Publisher_Layer_SHM_Configuration
  {
    int enable;
    int zero_copy_mode;
    unsigned int acknowledge_timeout_ms;
    unsigned int memfile_buffer_count;
    unsigned int memfile_min_size_bytes;
    unsigned int memfile_reserve_percent;
  };

  struct eCAL_Publisher_Layer_UDP_Configuration
  {
    int enable;
  };

  struct eCAL_Publisher_Layer_TCP_Configuration
  {
    int enable;
  };

  struct eCAL_Publisher_Layer_Configuration
  {
    eCAL_Publisher_Layer_SHM_Configuration shm;
    eCAL_Publisher_Layer_UDP_Configuration udp;
    eCAL_Publisher_Layer_TCP_Configuration tcp;
  };

  struct eCAL_Publisher_Configuration
  {
    eCAL_Publisher_Layer_Configuration layer;

    eCAL_TransportLayer_eType* layer_priority_local;
    size_t layer_priority_local_length;
    eCAL_TransportLayer_eType* layer_priority_remote;
    size_t layer_priority_remote_length;
  };

  enum eCAL_ePublisherEvent
  {
    eCAL_ePublisherEvent_none,
    eCAL_ePublisherEvent_connected,
    eCAL_ePublisherEvent_disconnected,
    eCAL_ePublisherEvent_dropped
  };

  struct eCAL_SPubEventCallbackData
  {
    eCAL_ePublisherEvent event_type;  
    long long event_time;
    struct eCAL_SDataTypeInformation subscriber_datatype;
  };

  typedef void (*eCAL_PubEventCallbackT)(const struct eCAL_STopicId*, const struct eCAL_SPubEventCallbackData*);


  struct eCAL_PayloadWriter
  {
    int (*WriteFull)(void*, size_t);
    int (*WriteModified)(void*, size_t);
    size_t (*GetSize)();
  };

  ECALC_API eCAL_Publisher* eCAL_Publisher_New(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const struct eCAL_Publisher_Configuration* publisher_configuration_);
  ECALC_API eCAL_Publisher* eCAL_Publisher_New2(const char* topic_name_, const struct eCAL_SDataTypeInformation* data_type_information_, const eCAL_PubEventCallbackT pub_event_callback, const struct eCAL_Publisher_Configuration* publisher_configuration_);

  ECALC_API void eCAL_Publisher_Delete(eCAL_Publisher* publisher_);

  ECALC_API int eCAL_Publisher_Send(eCAL_Publisher* publisher_, const void* buffer_, size_t buffer_len_, long long timestamp_);
  ECALC_API int eCAL_Publisher_Send2(eCAL_Publisher* publisher_, struct eCAL_PayloadWriter* payload_writer_, long long timestamp_);

  ECALC_API size_t eCAL_Publisher_GetSubscriberCount(eCAL_Publisher* publisher_);

  ECALC_API char* eCAL_Publisher_GetTopicName(eCAL_Publisher* publisher_);

  ECALC_API struct eCAL_STopicId* eCAL_Publisher_GetTopicId(eCAL_Publisher* publisher_);
  ECALC_API void eCAL_STopicId_Free(struct eCAL_STopicId* topic_id_);


  ECALC_API struct eCAL_SDataTypeInformation* eCAL_Publisher_GetDataTypeInformation(eCAL_Publisher* publisher_);
  ECALC_API void eCAL_SDataTypeInformation_Free(struct eCAL_SDataTypeInformation* data_type_information_);

  ECALC_API struct eCAL_Publisher_Configuration* eCAL_GetPublisherConfiguration();
  ECALC_API void eCAL_Publisher_Configuration_Free(eCAL_Publisher_Configuration* publisher_configuration_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_pubsub_publisher_h_included*/
