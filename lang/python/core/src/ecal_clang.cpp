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
 * @brief  eCAL C language interface
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_client.h>
#include <ecal/v5/ecal_server.h>
#include <ecal/v5/ecal_publisher.h>
#include <ecal/v5/ecal_subscriber.h>

#include "ecal_clang.h"

#include <mutex>
#include <string>
#include <vector>
#include <functional>

namespace
{
  char* str_malloc(const std::string& buf_s_)
  {
    void* cbuf = malloc(buf_s_.size());
    if (cbuf != nullptr)
    {
      memcpy(cbuf, buf_s_.data(), buf_s_.size());
    }
    return(static_cast<char*>(cbuf));
  }

  bool GetTopicDataTypeInformation(const char* topic_name_, eCAL::SDataTypeInformation& topic_info_)
  {
    std::set<eCAL::STopicId> pub_ids;
    eCAL::Registration::GetPublisherIDs(pub_ids);
    // try to find topic name in publisher set
    for (const auto& pub_id : pub_ids)
    {
      if (pub_id.topic_name == topic_name_)
      {
        return eCAL::Registration::GetPublisherInfo(pub_id, topic_info_);
      }
    }
    // try to find topic name in subscriber set
    std::set<eCAL::STopicId> sub_ids;
    eCAL::Registration::GetSubscriberIDs(sub_ids);
    for (const auto& sub_id : sub_ids)
    {
      if (sub_id.topic_name == topic_name_)
      {
        return eCAL::Registration::GetSubscriberInfo(sub_id, topic_info_);
      }
    }
    return false;
  }

  /**********************************************************************/
  /* This is duplicated code from ecal_core_c                           */
  /* We need to use the C API here instead of reimplementing everything */
  /* But will be obsolete once we move to nanobind                      */
  /**********************************************************************/

  eCAL_Publisher_Event enum_class_to_enum(eCAL::ePublisherEvent event_) {
    switch (event_) {
    case eCAL::ePublisherEvent::none:                   return pub_event_none;
    case eCAL::ePublisherEvent::connected:              return pub_event_connected;
    case eCAL::ePublisherEvent::disconnected:           return pub_event_disconnected;
    case eCAL::ePublisherEvent::dropped:                return pub_event_dropped;
    default:                                            return pub_event_none;
    }
  }

  eCAL::ePublisherEvent enum_to_enum_class(eCAL_Publisher_Event event_) {
    switch (event_) {
    case pub_event_none:                   return eCAL::ePublisherEvent::none;
    case pub_event_connected:              return eCAL::ePublisherEvent::connected;
    case pub_event_disconnected:           return eCAL::ePublisherEvent::disconnected;
    case pub_event_dropped:                return eCAL::ePublisherEvent::dropped;
    default:                               return eCAL::ePublisherEvent::none;
    }
  }

  eCAL_Subscriber_Event enum_class_to_enum(eCAL::eSubscriberEvent cpp_event_) {
    switch (cpp_event_) {
    case eCAL::eSubscriberEvent::none:                   return sub_event_none;
    case eCAL::eSubscriberEvent::connected:              return sub_event_connected;
    case eCAL::eSubscriberEvent::disconnected:           return sub_event_disconnected;
    case eCAL::eSubscriberEvent::dropped:                return sub_event_dropped;
    default:                                             return sub_event_none;
    }
  }

  eCAL::eSubscriberEvent enum_to_enum_class(eCAL_Subscriber_Event c_event_) {
    switch (c_event_) {
    case sub_event_none:                   return eCAL::eSubscriberEvent::none;
    case sub_event_connected:              return eCAL::eSubscriberEvent::connected;
    case sub_event_disconnected:           return eCAL::eSubscriberEvent::disconnected;
    case sub_event_dropped:                return eCAL::eSubscriberEvent::dropped;
    default:                               return eCAL::eSubscriberEvent::none;
    }
  }
}

/****************************************/
/*      ecal_getversion                 */
/****************************************/
const char* ecal_getversion()
{
  return(ECAL_VERSION);
}

/****************************************/
/*      ecal_getversion_components      */
/****************************************/
int ecal_getversion_components(int* major_, int* minor_, int* patch_)
{
  if ((major_ == nullptr) || (minor_ == nullptr) || (patch_ == nullptr) )
      return 0;
  *major_ = eCAL::GetVersion().major;
  *minor_ = eCAL::GetVersion().minor;
  *minor_ = eCAL::GetVersion().patch;
  return 1;
}

/****************************************/
/*      ecal_getdate                    */
/****************************************/
const char* ecal_getdate()
{
  return(ECAL_DATE);
}

/****************************************/
/*      ecal_initialize                 */
/****************************************/
int ecal_initialize(const char* unit_name_)
{
  std::string unit_name = (unit_name_ != nullptr) ? std::string(unit_name_) : std::string("");
  return static_cast<int>(!eCAL::Initialize(unit_name));
}

/****************************************/
/*      ecal_finalize                   */
/****************************************/
int ecal_finalize()
{
  //* @return Zero if succeeded
  return static_cast<int>(!eCAL::Finalize());
}

/****************************************/
/*      ecal_is_initialized             */
/****************************************/
int ecal_is_initialized()
{
  //* @return 1 if eCAL is initialized.
  return static_cast<int>(eCAL::IsInitialized());
}

/****************************************/
/*      ecal_set_process_state          */
/****************************************/
void ecal_set_process_state(const int severity_, const int level_, const char* info_)
{
  return(eCAL::Process::SetState(eCAL::Process::eSeverity(severity_), eCAL::Process::eSeverityLevel(level_), info_));
}

/****************************************/
/*      ecal_ok                         */
/****************************************/
bool ecal_ok()
{
  return(eCAL::Ok());
}

/****************************************/
/*      ecal_free_mem                    */
/****************************************/
void ecal_free_mem(void* mem_)
{
  free(mem_);
}

/****************************************/
/*      ecal_sleep_ms                   */
/****************************************/
void ecal_sleep_ms(const long time_ms_)
{
  eCAL::Process::SleepMS(time_ms_);
}

/****************************************/
/*     ecal_shutdown_process_uname      */
/****************************************/
void ecal_shutdown_process_uname(const char* unit_name_)
{
  eCAL::Util::ShutdownProcess(unit_name_);
}

/****************************************/
/*     ecal_shutdown_process_id         */
/****************************************/
void ecal_shutdown_process_id(const int process_id_)
{
  eCAL::Util::ShutdownProcess(process_id_);
}

/****************************************/
/*      ecal_shutdown_processes         */
/****************************************/
void ecal_shutdown_processes()
{
  eCAL::Util::ShutdownProcesses();
}

/****************************************/
/*      get_type_name                   */
/****************************************/
bool ecal_get_type_name(const char* topic_name_, const char** topic_type_, int* topic_type_len_)
{
  eCAL::SDataTypeInformation topic_info;
  // get the first matching topic type information for the given topic name found in either the publisher or subscriber id set !
  bool ret = GetTopicDataTypeInformation(topic_name_, topic_info);
  if(ret)
  {
    std::string topic_type_s = topic_info.name;
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(topic_type_s);
    if(cbuf == nullptr) return(false);

    if (topic_type_ != nullptr) {
      *topic_type_ = cbuf;
      if(topic_type_len_ != nullptr) *topic_type_len_ = static_cast<int>(topic_type_s.size());
    }
    else {
      // free allocated memory:
      ecal_free_mem(cbuf);
      if (topic_type_len_ != nullptr) *topic_type_len_ = 0;
      ret = false;
    }
  }
  return(ret);
}

/****************************************/
/*      get_type_encoding               */
/****************************************/
bool ecal_get_type_encoding(const char* topic_name_, const char** topic_encoding_, int* topic_encoding_len_)
{
  eCAL::SDataTypeInformation topic_info;
  // get the first matching topic type information for the given topic name found in either the publisher or subscriber id set !
  bool ret = GetTopicDataTypeInformation(topic_name_, topic_info);
  if (ret)
  {
    std::string topic_encoding_s = topic_info.encoding;
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(topic_encoding_s);
    if (cbuf == nullptr) return(false);

    if (topic_encoding_ != nullptr) {
      *topic_encoding_ = cbuf;
      if (topic_encoding_len_ != nullptr) *topic_encoding_len_ = static_cast<int>(topic_encoding_s.size());
    }
    else {
      // free allocated memory:
      ecal_free_mem(cbuf);
      if (topic_encoding_len_ != nullptr) *topic_encoding_len_ = 0;
      ret = false;
    }
  }
  return(ret);
}

/****************************************/
/*      get_description                 */
/****************************************/
bool ecal_get_description(const char* topic_name_, const char** topic_desc_, int* topic_desc_len_)
{
  eCAL::SDataTypeInformation topic_info;
  // get the first matching topic type information for the given topic name found in either the publisher or subscriber id set !
  bool ret = GetTopicDataTypeInformation(topic_name_, topic_info);
  if(ret)
  {
    std::string topic_desc_s = topic_info.descriptor;
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(topic_desc_s);
    if(cbuf == nullptr) return(false);

    if (topic_desc_ != nullptr) {
      *topic_desc_ = cbuf;
      if (topic_desc_len_ != nullptr) *topic_desc_len_ = static_cast<int>(topic_desc_s.size());
    }
    else {
      // free allocated memory:
      ecal_free_mem(cbuf);
      if (topic_desc_len_ != nullptr) *topic_desc_len_ = 0;
      ret = false;
    }
  }
  return(ret);
}

/****************************************/
/*      log_message                     */
/****************************************/
void log_message(const eCAL_Logging_eLogLevel& log_level_, const char* message_)
{
  // this is potentially dangerous, but we will hopefully remove this soon anyways
  eCAL::Logging::Log(static_cast<eCAL::Logging::eLogLevel>(log_level_), message_);
}


/****************************************/
/*      pub_create                      */
/****************************************/
ECAL_HANDLE pub_create(const char* topic_name_, const char* topic_type_, const char* topic_enc_, const char* topic_desc_, const int topic_desc_length_)
{
  eCAL::SDataTypeInformation topic_info;
  topic_info.name       = topic_type_;
  topic_info.encoding   = topic_enc_;
  topic_info.descriptor = std::string(topic_desc_, static_cast<size_t>(topic_desc_length_));

  auto* pub = new eCAL::v5::CPublisher;
  if (!pub->Create(topic_name_, topic_info))
  {
    delete pub;
    return(nullptr);
  }
  return(pub);
}

/****************************************/
/*      pub_destroy                     */
/****************************************/
bool pub_destroy(ECAL_HANDLE handle_)
{
  auto* pub = static_cast<eCAL::v5::CPublisher*>(handle_);
  if(pub != nullptr)
  {
    delete pub;
    return(true);
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      pub_send                        */
/****************************************/
int pub_send(ECAL_HANDLE handle_, const char* payload_, const int length_, const long long time_)
{
  auto* pub = static_cast<eCAL::v5::CPublisher*>(handle_);
  if(pub != nullptr)
  {
    const size_t ret = pub->Send(payload_, static_cast<size_t>(length_), time_);
    if(static_cast<int>(ret) == length_)
    {
      return(length_);
    }
  }
  return(0);
}

/****************************************/
/*      pub_add_event_callback          */
/****************************************/
static std::mutex g_pub_event_callback_mtx;
static void g_pub_event_callback(const char* topic_name_, const struct eCAL::v5::SPubEventCallbackData* data_, const PubEventCallbackCT callback_, void* par_)
{
  const std::lock_guard<std::mutex> lock(g_pub_event_callback_mtx);
  SPubEventCallbackDataC data{};
  data.type       = enum_class_to_enum(data_->type);
  data.time       = data_->time;
  data.clock      = data_->clock;
  data.topic_id   = data_->tid.c_str();
  data.topic_name = data_->tdatatype.name.c_str();
  data.tencoding  = data_->tdatatype.encoding.c_str();
  data.tdesc      = data_->tdatatype.descriptor.c_str();
  callback_(topic_name_, &data, par_);
}

bool pub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, const PubEventCallbackCT callback_, void* par_)
{
  auto* pub = static_cast<eCAL::v5::CPublisher*>(handle_);

  auto callback = std::bind(g_pub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(pub->AddEventCallback(enum_to_enum_class(type_), callback));
}

/****************************************/
/*      pub_rem_event_callback          */
/****************************************/
bool pub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_)
{
  auto* pub = static_cast<eCAL::v5::CPublisher*>(handle_);

  return(pub->RemEventCallback(enum_to_enum_class(type_)));
}


/****************************************/
/*      sub_create                      */
/****************************************/
ECAL_HANDLE sub_create(const char* topic_name_, const char* topic_type_, const char* topic_enc_, const char* topic_desc_, const int topic_desc_length_)
{
  eCAL::SDataTypeInformation topic_info;
  topic_info.name       = topic_type_;
  topic_info.encoding   = topic_enc_;
  topic_info.descriptor = std::string(topic_desc_, static_cast<size_t>(topic_desc_length_));

  auto* sub = new eCAL::v5::CSubscriber;
  if (!sub->Create(topic_name_, topic_info))
  {
    delete sub;
    return(nullptr);
  }
  return(sub);
}

/****************************************/
/*      sub_destroy                     */
/****************************************/
bool sub_destroy(ECAL_HANDLE handle_)
{
  auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
  if(sub != nullptr)
  {
    delete sub;
    return(true);
  }
  return(false);
}

/****************************************/
/*      sub_receive                     */
/****************************************/
int sub_receive(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, const int timeout_)
{
  auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
  if(sub != nullptr)
  {
    std::string rcv_buf;
    long long rcv_time  = 0;
    sub->ReceiveBuffer(rcv_buf, &rcv_time, timeout_);

    if(!rcv_buf.empty())
    {
      // this has to be freed by caller (ecal_free_mem)
      char* cbuf = str_malloc(rcv_buf);
      if(cbuf == nullptr) return(0);

      if (rcv_buf_ != nullptr) {
        *rcv_buf_ = cbuf;
        if (rcv_buf_len_ != nullptr) *rcv_buf_len_ = static_cast<int>(rcv_buf.size());
      }
      else {
        // free allocated memory:
        ecal_free_mem(cbuf);
        if (rcv_buf_len_ != nullptr) *rcv_buf_len_ = 0;
        // operation could't be completed successfully
        return(0);
      }

      if(rcv_time_ != nullptr)    *rcv_time_    = rcv_time;

      return(static_cast<int>(rcv_buf.size()));
    }
  }
  return(0);
}

/****************************************/
/*      sub_receive_buffer              */
/****************************************/
bool sub_receive_buffer(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, const int timeout_)
{
  auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);
  if (sub != nullptr)
  {
    std::string rcv_buf;
    long long rcv_time = 0;

    if (sub->ReceiveBuffer(rcv_buf, &rcv_time, timeout_))
    {
      // this has to be freed by caller (ecal_free_mem)
      char* cbuf = str_malloc(rcv_buf);
      if (cbuf == nullptr) return(false);

      if (rcv_buf_ != nullptr) {
        *rcv_buf_ = cbuf;
        if (rcv_buf_len_ != nullptr) *rcv_buf_len_ = static_cast<int>(rcv_buf.size());
      }
      else {
        // free allocated memory:
        ecal_free_mem(cbuf);
        if (rcv_buf_len_ != nullptr) *rcv_buf_len_ = 0;
        // operation couldn't be completed successfullly.
        return(false);
      }
      if (rcv_time_ != nullptr)    *rcv_time_ = rcv_time;

      return(true);
    }
  }
  return(false);
}

/****************************************/
/*      sub_add_receive_callback        */
/****************************************/
static std::mutex g_sub_receive_callback_mtx;
static void g_sub_receive_callback(const char* topic_name_, const struct eCAL::v5::SReceiveCallbackData* data_, const ReceiveCallbackCT callback_, void* par_)
{
  const std::lock_guard<std::mutex> lock(g_sub_receive_callback_mtx);
  SReceiveCallbackDataC data{};
  data.buf   = data_->buf;
  data.size  = data_->size;
  data.id    = data_->id;
  data.time  = data_->time;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

bool sub_add_receive_callback(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_)
{
  auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);

  auto callback = std::bind(g_sub_receive_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(sub->AddReceiveCallback(callback));
}

/****************************************/
/*      sub_rem_receive_callback        */
/****************************************/
bool sub_rem_receive_callback(ECAL_HANDLE handle_)
{
  auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);

  return(sub->RemReceiveCallback());
}

/****************************************/
/*      sub_add_event_callback          */
/****************************************/
static std::mutex g_sub_event_callback_mtx;
static void g_sub_event_callback(const char* topic_name_, const struct eCAL::v5::SSubEventCallbackData* data_, const SubEventCallbackCT callback_, void* par_)
{
  const std::lock_guard<std::mutex> lock(g_sub_event_callback_mtx);
  SSubEventCallbackDataC data{};
  data.type       = enum_class_to_enum(data_->type);
  data.time       = data_->time;
  data.clock      = data_->clock;
  data.topic_id   = data_->tid.c_str();
  data.topic_name = data_->tdatatype.name.c_str();
  data.tencoding  = data_->tdatatype.encoding.c_str();
  data.tdesc      = data_->tdatatype.descriptor.c_str();
  callback_(topic_name_, &data, par_);
}

bool sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const SubEventCallbackCT callback_, void* par_)
{
  auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);

  auto callback = std::bind(g_sub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(sub->AddEventCallback(enum_to_enum_class(type_), callback));
}

/****************************************/
/*      sub_rem_event_callback          */
/****************************************/
bool sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_)
{
  auto* sub = static_cast<eCAL::v5::CSubscriber*>(handle_);

  return(sub->RemEventCallback(enum_to_enum_class(type_)));
}


/****************************************/
/*      server_create                   */
/****************************************/
ECAL_HANDLE server_create(const char* service_name_)
{
  auto* server = new eCAL::v5::CServiceServer;
  if (!server->Create(service_name_))
  {
    delete server;
    return(nullptr);
  }
  return(server);
}

/****************************************/
/*      server_destroy                  */
/****************************************/
bool server_destroy(ECAL_HANDLE handle_)
{
  auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
  if (server != nullptr)
  {
    delete server;
    return(true);
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      server_add_method_callback      */
/****************************************/
static std::mutex g_server_add_method_callback_mtx;
static int g_server_method_callback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_, const MethodCallbackCT callback_, void* par_)
{
  const std::lock_guard<std::mutex> lock(g_server_add_method_callback_mtx);
  void* response(nullptr);
  int   response_len(0);
  const int ret = callback_(method_.data(), req_type_.data(), resp_type_.data(), request_.data(), static_cast<int>(request_.size()), &response, &response_len, par_);
  response_ = std::string(static_cast<char*>(response), static_cast<size_t>(response_len));
  return ret;
}

bool server_add_method_callback(ECAL_HANDLE handle_,  const char* method_name_, const char* req_type_, const char* resp_type_, const MethodCallbackCT callback_, void* par_)
{
  auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
  if (server != nullptr)
  {
    auto callback = std::bind(g_server_method_callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, callback_, par_);
    return(server->AddMethodCallback(method_name_, req_type_, resp_type_, callback));
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      server_rem_method_callback      */
/****************************************/
bool server_rem_method_callback(ECAL_HANDLE handle_, const char* method_name_)
{
  auto* server = static_cast<eCAL::v5::CServiceServer*>(handle_);
  if (server != nullptr)
  {
    return(server->RemMethodCallback(method_name_));
  }
  else
  {
    return(false);
  }
}


/****************************************/
/*      client_create                   */
/****************************************/
ECAL_HANDLE client_create(const char* service_name_)
{
  auto* client = new eCAL::v5::CServiceClient;
  if (!client->Create(service_name_))
  {
    delete client;
    return(nullptr);
  }
  return(client);
}

/****************************************/
/*      client_destroy                  */
/****************************************/
bool client_destroy(ECAL_HANDLE handle_)
{
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  if (client != nullptr)
  {
    delete client;
    return(true);
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      client_set_hostname             */
/****************************************/
bool client_set_hostname(ECAL_HANDLE handle_, const char* host_name_)
{
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  if (client != nullptr)
  {
    return(client->SetHostName(host_name_));
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      client_call_method              */
/****************************************/
bool client_call_method(ECAL_HANDLE handle_, const char* method_name_, const char* request_, const int request_len_, const int timeout_)
{
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  if (client != nullptr)
  {
    std::string request(request_, request_len_);
    return(client->Call(method_name_, request, timeout_));
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      client_call_method_async        */
/****************************************/
bool client_call_method_async(ECAL_HANDLE handle_, const char* method_name_, const char* request_, const int request_len_, const int timeout_)
{
  auto* client = static_cast<eCAL::v5::CServiceClient*>(handle_);
  if (client != nullptr)
  {
    std::string request(request_, request_len_);
    return(client->CallAsync(method_name_, request, timeout_));
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      mon_initialize                  */
/****************************************/
int mon_initialize()
{
  return(eCAL::Initialize("", eCAL::Init::Monitoring));
}

/****************************************/
/*      mon_finalize                    */
/****************************************/
int mon_finalize()
{
  return(ecal_finalize());
}
