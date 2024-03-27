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

/**
 * @brief  eCAL C language interface
**/

#include <ecal/ecal.h>

#include "ecal_clang.h"

#include <mutex>
#include <string>
#include <vector>
#include <functional>

static char* str_malloc(const std::string& buf_s_)
{
  void* cbuf = malloc(buf_s_.size());
  if(cbuf != nullptr)
  {
    memcpy(cbuf, buf_s_.data(), buf_s_.size());
  }
  return(static_cast<char*>(cbuf));
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
  return eCAL::GetVersion(major_, minor_, patch_);
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
int ecal_initialize(int argc_, char **argv_, const char* unit_name_)
{
  return(eCAL::Initialize(argc_, argv_, unit_name_));
}

/****************************************/
/*      ecal_finalize                   */
/****************************************/
int ecal_finalize()
{
  //* @return Zero if succeeded, 1 if still initialized, -1 if failed.
  return(eCAL::Finalize());
}

/****************************************/
/*      ecal_is_initialized             */
/****************************************/
int ecal_is_initialized()
{
  //* @return 1 if eCAL is initialized.
  return(eCAL::IsInitialized());
}

/****************************************/
/*      ecal_set_unit_name              */
/****************************************/
int ecal_set_unit_name(const char* unit_name_)
{
  return(eCAL::SetUnitName(unit_name_));
}

/****************************************/
/*      ecal_set_process_state          */
/****************************************/
void ecal_set_process_state(const int severity_, const int level_, const char* info_)
{
  return(eCAL::Process::SetState(eCAL_Process_eSeverity(severity_), eCAL_Process_eSeverity_Level(level_), info_));
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
/*      ecal_shutdown_core              */
/****************************************/
void ecal_shutdown_core()
{
  eCAL::Util::ShutdownCore();
}

/****************************************/
/*      ecal_enable_loopback            */
/****************************************/
void ecal_enable_loopback(const int state_)
{
  eCAL::Util::EnableLoopback(state_ != 0);
}

/****************************************/
/*      get_type_name                   */
/****************************************/
bool ecal_get_type_name(const char* topic_name_, const char** topic_type_, int* topic_type_len_)
{
  eCAL::SDataTypeInformation topic_info;
  bool ret = eCAL::Util::GetTopicDataTypeInformation(topic_name_, topic_info);
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
  bool ret = eCAL::Util::GetTopicDataTypeInformation(topic_name_, topic_info);
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
  bool ret = eCAL::Util::GetTopicDataTypeInformation(topic_name_, topic_info);
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
/*      log_setlevel                    */
/****************************************/
void log_setlevel(const int level_)
{
  eCAL::Logging::SetLogLevel(eCAL_Logging_eLogLevel(level_));
}

/****************************************/
/*      log_message                     */
/****************************************/
void log_message(const char* message_)
{
  eCAL::Logging::Log(message_);
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

  auto* pub = new eCAL::CPublisher;
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
  auto* pub = static_cast<eCAL::CPublisher*>(handle_);
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
  auto* pub = static_cast<eCAL::CPublisher*>(handle_);
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
static void g_pub_event_callback(const char* topic_name_, const struct eCAL::SPubEventCallbackData* data_, const PubEventCallbackCT callback_, void* par_)
{
  const std::lock_guard<std::mutex> lock(g_pub_event_callback_mtx);
  SPubEventCallbackDataC data{};
  data.type      = data_->type;
  data.time      = data_->time;
  data.clock     = data_->clock;
  data.tid       = data_->tid.c_str();
  data.tname     = data_->tdatatype.name.c_str();
  data.tencoding = data_->tdatatype.encoding.c_str();
  data.tdesc     = data_->tdatatype.descriptor.c_str();
  callback_(topic_name_, &data, par_);
}

bool pub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, const PubEventCallbackCT callback_, void* par_)
{
  auto* pub = static_cast<eCAL::CPublisher*>(handle_);

  auto callback = std::bind(g_pub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(pub->AddEventCallback(type_, callback));
}

/****************************************/
/*      pub_rem_event_callback          */
/****************************************/
bool pub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_)
{
  auto* pub = static_cast<eCAL::CPublisher*>(handle_);

  return(pub->RemEventCallback(type_));
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

  auto* sub = new eCAL::CSubscriber;
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
  auto* sub = static_cast<eCAL::CSubscriber*>(handle_);
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
  auto* sub = static_cast<eCAL::CSubscriber*>(handle_);
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
  auto* sub = static_cast<eCAL::CSubscriber*>(handle_);
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
static void g_sub_receive_callback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_, const ReceiveCallbackCT callback_, void* par_)
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
  auto* sub = static_cast<eCAL::CSubscriber*>(handle_);

  auto callback = std::bind(g_sub_receive_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(sub->AddReceiveCallback(callback));
}

/****************************************/
/*      sub_rem_receive_callback        */
/****************************************/
bool sub_rem_receive_callback(ECAL_HANDLE handle_)
{
  auto* sub = static_cast<eCAL::CSubscriber*>(handle_);

  return(sub->RemReceiveCallback());
}

/****************************************/
/*      sub_add_event_callback          */
/****************************************/
static std::mutex g_sub_event_callback_mtx;
static void g_sub_event_callback(const char* topic_name_, const struct eCAL::SSubEventCallbackData* data_, const SubEventCallbackCT callback_, void* par_)
{
  const std::lock_guard<std::mutex> lock(g_sub_event_callback_mtx);
  SSubEventCallbackDataC data{};
  data.type      = data_->type;
  data.time      = data_->time;
  data.clock     = data_->clock;
  data.tid       = data_->tid.c_str();
  data.tname     = data_->tdatatype.name.c_str();
  data.tencoding = data_->tdatatype.encoding.c_str();
  data.tdesc     = data_->tdatatype.descriptor.c_str();
  callback_(topic_name_, &data, par_);
}

bool sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const SubEventCallbackCT callback_, void* par_)
{
  auto* sub = static_cast<eCAL::CSubscriber*>(handle_);

  auto callback = std::bind(g_sub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(sub->AddEventCallback(type_, callback));
}

/****************************************/
/*      sub_rem_event_callback          */
/****************************************/
bool sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_)
{
  auto* sub = static_cast<eCAL::CSubscriber*>(handle_);

  return(sub->RemEventCallback(type_));
}


/****************************************/
/*      server_create                   */
/****************************************/
ECAL_HANDLE server_create(const char* service_name_)
{
  auto* server = new eCAL::CServiceServer;
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
  auto* server = static_cast<eCAL::CServiceServer*>(handle_);
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
  auto* server = static_cast<eCAL::CServiceServer*>(handle_);
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
  auto* server = static_cast<eCAL::CServiceServer*>(handle_);
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
  auto* client = new eCAL::CServiceClient;
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
  auto* client = static_cast<eCAL::CServiceClient*>(handle_);
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
  auto* client = static_cast<eCAL::CServiceClient*>(handle_);
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
  auto* client = static_cast<eCAL::CServiceClient*>(handle_);
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
  auto* client = static_cast<eCAL::CServiceClient*>(handle_);
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
/*      client_add_response_callback    */
/****************************************/

/****************************************/
/*      client_rem_response_callback    */
/****************************************/


/****************************************/
/*      mon_initialize                  */
/****************************************/
int mon_initialize()
{
  return(eCAL::Initialize(0, nullptr, "", eCAL::Init::Monitoring));
}

/****************************************/
/*      mon_finalize                    */
/****************************************/
int mon_finalize()
{
  return(ecal_finalize());
}

/****************************************/
/*      mon_set_excl_filter             */
/****************************************/
int mon_set_excl_filter(const char* filter_)
{
  return(eCAL::Monitoring::SetExclFilter(filter_));
}

/****************************************/
/*      mon_set_incl_filter             */
/****************************************/
int mon_set_incl_filter(const char* filter_)
{
  return(eCAL::Monitoring::SetInclFilter(filter_));
}

/****************************************/
/*      mon_set_filter_state            */
/****************************************/
int mon_set_filter_state(const bool state_)
{
  return(eCAL::Monitoring::SetFilterState(state_));
}

/****************************************/
/*      mon_get_monitoring              */
/****************************************/
int mon_get_monitoring(const char** mon_buf_, int* mon_buf_len_)
{
  std::string mon_s;
  const int size = eCAL::Monitoring::GetMonitoring(mon_s);
  if(size > 0)
  {
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(mon_s);
    if(cbuf == nullptr) return(0);

    if (mon_buf_ != nullptr) {
      *mon_buf_ = cbuf;
      if (mon_buf_len_ != nullptr) *mon_buf_len_ = static_cast<int>(mon_s.size());
    }
    else
    {
      // free allocated memory:
      ecal_free_mem(cbuf);
      if (mon_buf_len_ != nullptr) *mon_buf_len_ = 0;
      // operation could't be completed successfully
      return(0);
    }
    return(static_cast<int>(mon_s.size()));
  }
  else
  {
    return(0);
  }
}

/****************************************/
/*      mon_get_logging                 */
/****************************************/
int mon_get_logging(const char** log_buf_, int* log_buf_len_)
{
  std::string log_s;
  const int size = eCAL::Logging::GetLogging(log_s);
  if(size > 0)
  {
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(log_s);
    if(cbuf == nullptr) return(0);

    if (log_buf_ != nullptr) {
      *log_buf_ = cbuf;
      if (log_buf_len_ != nullptr) *log_buf_len_ = static_cast<int>(log_s.size());
    }
    else {
      // free allocated memory:
      ecal_free_mem(cbuf);
      if (log_buf_len_ != nullptr) *log_buf_len_ = 0;
      // operation couldn't be completed successfullly.
      return(0);
    }
    return(static_cast<int>(log_s.size()));
  }
  else
  {
    return(0);
  }
}
