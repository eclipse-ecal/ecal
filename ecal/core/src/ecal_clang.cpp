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
#include <ecal/ecal_clang.h>
#include <ecal/msg/protobuf/dynamic_json_subscriber.h>

#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include <mutex>

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
ECAL_API const char* ecal_getversion()
{
  return(ECAL_VERSION);
}

/****************************************/
/*      ecal_getdate                    */
/****************************************/
ECAL_API const char* ecal_getdate()
{
  return(ECAL_DATE);
}

/****************************************/
/*      ecal_initialize                 */
/****************************************/
ECAL_API int ecal_initialize(int argc_, char **argv_, const char* unit_name_)
{
  return(eCAL::Initialize(argc_, argv_, unit_name_));
}

/****************************************/
/*      ecal_finalize                   */
/****************************************/
ECAL_API int ecal_finalize()
{
  //* @return Zero if succeeded, 1 if still initialized, -1 if failed.
  return(eCAL::Finalize());
}

/****************************************/
/*      ecal_set_process_state          */
/****************************************/
ECAL_API void ecal_set_process_state(const int severity_, const int level_, const char* info_)
{
  return(eCAL::Process::SetState(eCAL_Process_eSeverity(severity_), eCAL_Process_eSeverity_Level(level_), info_));
}

/****************************************/
/*      ecal_ok                         */
/****************************************/
ECAL_API bool ecal_ok()
{
  return(eCAL::Ok());
}

/****************************************/
/*      ecal_free_mem                    */
/****************************************/
ECAL_API void ecal_free_mem(void* mem_)
{
  free(mem_);
}

/****************************************/
/*      ecal_sleep_ms                   */
/****************************************/
ECAL_API void ecal_sleep_ms(const long time_ms_)
{
  eCAL::Process::SleepMS(time_ms_);
}

/****************************************/
/*     ecal_shutdown_process_uname      */
/****************************************/
ECAL_API void ecal_shutdown_process_uname(const char* unit_name_)
{
  eCAL::Util::ShutdownProcess(unit_name_);
}

/****************************************/
/*     ecal_shutdown_process_id         */
/****************************************/
ECAL_API void ecal_shutdown_process_id(const int process_id_)
{
  eCAL::Util::ShutdownProcess(process_id_);
}

/****************************************/
/*      ecal_shutdown_processes         */
/****************************************/
ECAL_API void ecal_shutdown_processes()
{
  eCAL::Util::ShutdownProcesses();
}

/****************************************/
/*      ecal_shutdown_core              */
/****************************************/
ECAL_API void ecal_shutdown_core()
{
  eCAL::Util::ShutdownCore();
}

/****************************************/
/*      ecal_enable_loopback            */
/****************************************/
ECAL_API void ecal_enable_loopback(const int state_)
{
  eCAL::Util::EnableLoopback(state_ != 0);
}

/****************************************/
/*      get_type_name                   */
/****************************************/
ECAL_API bool ecal_get_type_name(const char* topic_name_, const char** topic_type_, int* topic_type_len_)
{
  std::string topic_type_s;
  bool ret = eCAL::Util::GetTypeName(topic_name_, topic_type_s);
  if(ret)
  {
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(topic_type_s);
    if(cbuf == nullptr) return(false);

    if(topic_type_)     *topic_type_     = cbuf;
    if(topic_type_len_) *topic_type_len_ = static_cast<int>(topic_type_s.size());
  }
  return(ret);
}

/****************************************/
/*      get_description                 */
/****************************************/
ECAL_API bool ecal_get_description(const char* topic_name_, const char** topic_desc_, int* topic_desc_len_)
{
  std::string topic_desc_s;
  bool ret = eCAL::Util::GetDescription(topic_name_, topic_desc_s);
  if(ret)
  {
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(topic_desc_s);
    if(cbuf == nullptr) return(false);

    if(topic_desc_)     *topic_desc_     = cbuf;
    if(topic_desc_len_) *topic_desc_len_ = static_cast<int>(topic_desc_s.size());
  }
  return(ret);
}

/****************************************/
/*      log_setlevel                    */
/****************************************/
ECAL_API void log_setlevel(const int level_)
{
  eCAL::Logging::SetLogLevel(eCAL_Logging_eLogLevel(level_));
}

/****************************************/
/*      log_setcoretime                 */
/****************************************/
ECAL_API void log_setcoretime(const double time_)
{
  eCAL::Logging::SetCoreTime(time_);
}

/****************************************/
/*      log_message                     */
/****************************************/
ECAL_API void log_message(const char* message_)
{
  eCAL::Logging::Log(message_);
}


/****************************************/
/*      pub_create                      */
/****************************************/
ECAL_API ECAL_HANDLE pub_create(const char* topic_name_, const char* topic_type_)
{
  eCAL::CPublisher* pub = new eCAL::CPublisher;
  if (!pub->Create(topic_name_, topic_type_))
  {
    delete pub;
    return(nullptr);
  }
  return(pub);
}

/****************************************/
/*      pub_destroy                     */
/****************************************/
ECAL_API bool pub_destroy(ECAL_HANDLE handle_)
{
  eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
  if(pub)
  {
    delete pub;
    pub = nullptr;
    return(true);
  }
  else
  {
    return(false);
  }
}

/****************************************/
/*      pub_setdescription              */
/****************************************/
ECAL_API bool pub_set_description(ECAL_HANDLE handle_, const char* topic_desc_, const int topic_desc_length_)
{
  eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
  if(pub)
  {
    return(pub->SetDescription(std::string(topic_desc_, static_cast<size_t>(topic_desc_length_))));
  }
  return(false);
}

/****************************************/
/*      pub_set_qos                     */
/****************************************/
ECAL_API bool pub_set_qos(ECAL_HANDLE handle_, struct SWriterQOSC qos_) //-V813
{
  int ret = eCAL_Pub_SetQOS(handle_, qos_);
  return(ret == 0);
}

/****************************************/
/*      pub_get_qos                     */
/****************************************/
ECAL_API bool pub_get_qos(ECAL_HANDLE handle_, struct SWriterQOSC* qos_)
{
  int ret = eCAL_Pub_GetQOS(handle_, qos_);
  return(ret == 0);
}

/****************************************/
/*      pub_set_layer_mode              */
/****************************************/
ECAL_API bool pub_set_layer_mode(ECAL_HANDLE handle_, const int layer_, const int mode_)
{
  eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
  if(pub)
  {
    return(pub->SetLayerMode(eCAL::TLayer::eTransportLayer(layer_), eCAL::TLayer::eSendMode(mode_)));
  }
  return(false);
}

/****************************************/
/*      pub_set_max_bandwidth_udp       */
/****************************************/
ECAL_API bool pub_set_max_bandwidth_udp(ECAL_HANDLE handle_, long bandwidth_)
{
  eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
  if (pub)
  {
    return(pub->SetMaxBandwidthUDP(bandwidth_));
  }
  return(false);
}

/****************************************/
/*      pub_send                        */
/****************************************/
ECAL_API int pub_send(ECAL_HANDLE handle_, const char* payload_, const int length_, const long long time_)
{
  eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
  if(pub)
  {
    size_t ret = pub->Send(payload_, static_cast<size_t>(length_), time_);
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
  std::lock_guard<std::mutex> lock(g_pub_event_callback_mtx);
  SPubEventCallbackDataC data;
  data.type  = data_->type;
  data.time  = data_->time;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

ECAL_API bool pub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_, const PubEventCallbackCT callback_, void* par_)
{
  eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);

  auto callback = std::bind(g_pub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(pub->AddEventCallback(type_, callback));
}

/****************************************/
/*      pub_rem_event_callback          */
/****************************************/
ECAL_API bool pub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Publisher_Event type_)
{
  eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);

  return(pub->RemEventCallback(type_));
}


/****************************************/
/*      sub_create                      */
/****************************************/
ECAL_API ECAL_HANDLE sub_create(const char* topic_name_, const char* topic_type_)
{
  eCAL::CSubscriber* sub = new eCAL::CSubscriber;
  if (!sub->Create(topic_name_, topic_type_))
  {
    delete sub;
    return(nullptr);
  }
  return(sub);
}

/****************************************/
/*      sub_destroy                     */
/****************************************/
ECAL_API bool sub_destroy(ECAL_HANDLE handle_)
{
  eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
  if(sub)
  {
    delete sub;
    sub = nullptr;
    return(true);
  }
  return(false);
}

/****************************************/
/*      sub_set_qos                     */
/****************************************/
ECAL_API bool sub_set_qos(ECAL_HANDLE handle_, struct SReaderQOSC qos_) //-V813
{
  int ret = eCAL_Sub_SetQOS(handle_, qos_);
  return(ret == 0);
}

/****************************************/
/*      sub_get_qos                     */
/****************************************/
ECAL_API bool sub_get_qos(ECAL_HANDLE handle_, struct SReaderQOSC* qos_)
{
  int ret = eCAL_Sub_GetQOS(handle_, qos_);
  return(ret == 0);
}

/****************************************/
/*      sub_receive                     */
/****************************************/
ECAL_API int sub_receive(ECAL_HANDLE handle_, const char** rcv_buf_, int* rcv_buf_len_, long long* rcv_time_, const int timeout_)
{
  eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
  if(sub)
  {
    std::string rcv_buf;
    long long rcv_time  = 0;
    sub->Receive(rcv_buf, &rcv_time, timeout_);

    if(!rcv_buf.empty())
    {
      // this has to be freed by caller (ecal_free_mem)
      char* cbuf = str_malloc(rcv_buf);
      if(cbuf == nullptr) return(0);

      if(rcv_buf_)     *rcv_buf_     = cbuf;
      if(rcv_buf_len_) *rcv_buf_len_ = static_cast<int>(rcv_buf.size());
      if(rcv_time_)    *rcv_time_    = rcv_time;

      return(static_cast<int>(rcv_buf.size()));
    }
  }
  return(0);
}

/****************************************/
/*      sub_add_receive_callback        */
/****************************************/
static std::mutex g_sub_receive_callback_mtx;
static void g_sub_receive_callback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_, const ReceiveCallbackCT callback_, void* par_)
{
  std::lock_guard<std::mutex> lock(g_sub_receive_callback_mtx);
  SReceiveCallbackDataC data;
  data.buf   = data_->buf;
  data.size  = data_->size;
  data.id    = data_->id;
  data.time  = data_->time;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

ECAL_API bool sub_add_receive_callback(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_)
{
  eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);

  auto callback = std::bind(g_sub_receive_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(sub->AddReceiveCallback(callback));
}

/****************************************/
/*      sub_rem_receive_callback        */
/****************************************/
ECAL_API bool sub_rem_receive_callback(ECAL_HANDLE handle_)
{
  eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);

  return(sub->RemReceiveCallback());
}

/****************************************/
/*      sub_add_event_callback          */
/****************************************/
static std::mutex g_sub_event_callback_mtx;
static void g_sub_event_callback(const char* topic_name_, const struct eCAL::SSubEventCallbackData* data_, const SubEventCallbackCT callback_, void* par_)
{
  std::lock_guard<std::mutex> lock(g_sub_event_callback_mtx);
  SSubEventCallbackDataC data;
  data.type  = data_->type;
  data.time  = data_->time;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

ECAL_API bool sub_add_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_, const SubEventCallbackCT callback_, void* par_)
{
  eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);

  auto callback = std::bind(g_sub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(sub->AddEventCallback(type_, callback));
}

/****************************************/
/*      sub_rem_event_callback          */
/****************************************/
ECAL_API bool sub_rem_event_callback(ECAL_HANDLE handle_, enum eCAL_Subscriber_Event type_)
{
  eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);

  return(sub->RemEventCallback(type_));
}

/****************************************/
/*      sub_set_timeout                 */
/****************************************/
ECAL_API bool sub_set_timeout(ECAL_HANDLE handle_, int timeout_)
{
  eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);

  return(sub->SetTimeout(timeout_));
}

/****************************************/
/*       dyn_json_sub_create            */
/****************************************/
ECAL_API ECAL_HANDLE dyn_json_sub_create(const char* topic_name_)
{
  eCAL::protobuf::CDynamicJSONSubscriber* sub = new eCAL::protobuf::CDynamicJSONSubscriber(topic_name_);
  if (!sub->IsCreated())
  {
    delete sub;
    return(nullptr);
  }
  return(sub);
}

/****************************************/
/*       dyn_json_sub_destroy           */
/****************************************/
ECAL_API bool dyn_json_sub_destroy(ECAL_HANDLE handle_)
{
  eCAL::protobuf::CDynamicJSONSubscriber* sub = static_cast<eCAL::protobuf::CDynamicJSONSubscriber*>(handle_);
  if (sub)
  {
    delete sub;
    sub = nullptr;
    return(true);
  }
  return(false);
}

/****************************************/
/*   dyn_json_sub_add_receive_callback  */
/****************************************/
static std::mutex g_dyn_json_sub_receive_callback_mtx;
static void g_dyn_json_sub_receive_callback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_, const ReceiveCallbackCT callback_, void* par_)
{
  std::lock_guard<std::mutex> lock(g_dyn_json_sub_receive_callback_mtx);
  SReceiveCallbackDataC data;
  data.buf   = data_->buf;
  data.size  = data_->size;
  data.id    = data_->id;
  data.time  = data_->time;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

ECAL_API bool dyn_json_sub_add_receive_callback(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_)
{
  eCAL::protobuf::CDynamicJSONSubscriber* sub = static_cast<eCAL::protobuf::CDynamicJSONSubscriber*>(handle_);

  auto callback = std::bind(g_dyn_json_sub_receive_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return(sub->AddReceiveCallback(callback));
}

/****************************************/
/*  dyn_json_sub_rem_receive_callback   */
/****************************************/
ECAL_API bool dyn_json_sub_rem_receive_callback(ECAL_HANDLE handle_)
{
  eCAL::protobuf::CDynamicJSONSubscriber* sub = static_cast<eCAL::protobuf::CDynamicJSONSubscriber*>(handle_);

  return(sub->RemReceiveCallback());
}


/****************************************/
/*      server_create                   */
/****************************************/
ECAL_API ECAL_HANDLE server_create(const char* service_name_)
{
  eCAL::CServiceServer* server = new eCAL::CServiceServer;
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
ECAL_API bool server_destroy(ECAL_HANDLE handle_)
{
  eCAL::CServiceServer* server = static_cast<eCAL::CServiceServer*>(handle_);
  if (server)
  {
    delete server;
    server = nullptr;
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
  std::lock_guard<std::mutex> lock(g_server_add_method_callback_mtx);
  void* response(nullptr);
  int   response_len(0);
  int ret = callback_(method_.data(), req_type_.data(), resp_type_.data(), request_.data(), static_cast<int>(request_.size()), &response, &response_len, par_);
  response_ = std::string(static_cast<char*>(response), static_cast<size_t>(response_len));
  return ret;
}

ECAL_API bool server_add_method_callback(ECAL_HANDLE handle_,  const char* method_name_, const char* req_type_, const char* resp_type_, const MethodCallbackCT callback_, void* par_)
{
  eCAL::CServiceServer* server = static_cast<eCAL::CServiceServer*>(handle_);
  if (server)
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
ECAL_API bool server_rem_method_callback(ECAL_HANDLE handle_, const char* method_name_)
{
  eCAL::CServiceServer* server = static_cast<eCAL::CServiceServer*>(handle_);
  if (server)
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
ECAL_API ECAL_HANDLE client_create(const char* service_name_)
{
  eCAL::CServiceClient* client = new eCAL::CServiceClient;
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
ECAL_API bool client_destroy(ECAL_HANDLE handle_)
{
  eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
  if (client)
  {
    delete client;
    client = nullptr;
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
ECAL_API bool client_set_hostname(ECAL_HANDLE handle_, const char* host_name_)
{
  eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
  if (client)
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
ECAL_API bool client_call_method(ECAL_HANDLE handle_, const char* method_name_, const char* request_, const int request_len_)
{
  eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
  if (client)
  {
    std::string request(request_, request_len_);
    return(client->Call(method_name_, request));
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
ECAL_API int mon_initialize()
{
  return(eCAL::Initialize(0, nullptr, "", eCAL::Init::Monitoring));
}

/****************************************/
/*      mon_finalize                    */
/****************************************/
ECAL_API int mon_finalize()
{
  return(ecal_finalize());
}

/****************************************/
/*      mon_set_excl_filter             */
/****************************************/
ECAL_API int mon_set_excl_filter(const char* filter_)
{
  return(eCAL::Monitoring::SetExclFilter(filter_));
}

/****************************************/
/*      mon_set_incl_filter             */
/****************************************/
ECAL_API int mon_set_incl_filter(const char* filter_)
{
  return(eCAL::Monitoring::SetInclFilter(filter_));
}

/****************************************/
/*      mon_set_filter_state            */
/****************************************/
ECAL_API int mon_set_filter_state(const bool state_)
{
  return(eCAL::Monitoring::SetFilterState(state_));
}

/****************************************/
/*      mon_get_monitoring              */
/****************************************/
ECAL_API int mon_get_monitoring(const char** mon_buf_, int* mon_buf_len_)
{
  std::string mon_s;
  int size = eCAL::Monitoring::GetMonitoring(mon_s);
  if(size > 0)
  {
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(mon_s);
    if(cbuf == nullptr) return(0);

    if(mon_buf_)     *mon_buf_     = cbuf;
    if(mon_buf_len_) *mon_buf_len_ = static_cast<int>(mon_s.size());
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
ECAL_API int mon_get_logging(const char** log_buf_, int* log_buf_len_)
{
  std::string log_s;
  int size = eCAL::Monitoring::GetLogging(log_s);
  if(size > 0)
  {
    // this has to be freed by caller (ecal_free_mem)
    char* cbuf = str_malloc(log_s);
    if(cbuf == nullptr) return(0);

    if(log_buf_)     *log_buf_     = cbuf;
    if(log_buf_len_) *log_buf_len_ = static_cast<int>(log_s.size());
    return(static_cast<int>(log_s.size()));
  }
  else
  {
    return(0);
  }
}
