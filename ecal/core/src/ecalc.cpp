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
 * @brief  Implementation of the eCAL dll interface
**/

#include <mutex>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/dynamic_json_subscriber.h>
#include <ecal/ecalc.h>

static int CopyBuffer(void* target_, int target_len_, const std::string& source_s_)
{
  if(target_ == nullptr) return(0);
  if(source_s_.empty())  return(0);
  if(target_len_ == ECAL_ALLOCATE_4ME)
  {
    void* buf_alloc = malloc(source_s_.size());
    if(buf_alloc == nullptr) return(0);
    int copied = CopyBuffer(buf_alloc, static_cast<int>(source_s_.size()), source_s_);
    if(copied > 0)
    {
      *((void**)target_) = buf_alloc; //-V206
      return(copied);
    }
  }
  else
  {
    if(target_len_ < static_cast<int>(source_s_.size())) return(0);
    memcpy(target_, source_s_.data(), source_s_.size());
    return(static_cast<int>(source_s_.size()));
  }
  return(0);
}

/////////////////////////////////////////////////////////
// Core
/////////////////////////////////////////////////////////
extern "C"
{
  ECALC_API const char* eCAL_GetVersionString()
  {
    return(ECAL_VERSION);
  }

  ECALC_API const char* eCAL_GetVersionDateString()
  {
    return(ECAL_DATE);
  }

  ECALC_API int eCAL_GetVersion(int* major_, int* minor_, int* patch_)
  {
    if((major_ == nullptr) && (minor_ == nullptr) && (patch_ == nullptr)) return(-1);
    if(major_) *major_ = ECAL_VERSION_MAJOR;
    if(minor_) *minor_ = ECAL_VERSION_MINOR;
    if(patch_) *patch_ = ECAL_VERSION_PATCH;
    return(0);
  }

  ECALC_API int eCAL_Initialize(int argc_, char **argv_, const char *unit_name_, unsigned int components_)
  {
    return(eCAL::Initialize(argc_, argv_, unit_name_, components_));
  }

  ECALC_API int eCAL_SetUnitName(const char *unit_name_)
  {
    return(eCAL::SetUnitName(unit_name_));
  }

  ECALC_API int eCAL_Finalize(unsigned int components_)
  {
    return(eCAL::Finalize(components_));
  }

  ECALC_API int eCAL_IsInitialized(unsigned int component_)
  {
    return(eCAL::IsInitialized(component_));
  }

  ECALC_API int eCAL_Ok()
  {
    return(eCAL::Ok());
  }

  ECALC_API void eCAL_FreeMem(void* mem_)
  {
    free(mem_);
  }
}

/////////////////////////////////////////////////////////
// Util
/////////////////////////////////////////////////////////
extern "C"
{
  ECALC_API void eCAL_Util_ShutdownUnitName(const char* unit_name_)
  {
    std::string unit_name = unit_name_;
    eCAL::Util::ShutdownProcess(unit_name);
  }

  ECALC_API void eCAL_Util_ShutdownProcessID(int process_id_)
  {
    eCAL::Util::ShutdownProcess(process_id_);
  }

  ECALC_API void eCAL_Util_ShutdownProcesses()
  {
    eCAL::Util::ShutdownProcesses();
  }

  ECALC_API void eCAL_Util_ShutdownCore()
  {
    eCAL::Util::ShutdownCore();
  }

  ECALC_API void eCAL_Util_EnableLoopback(int state_)
  {
    eCAL::Util::EnableLoopback(state_ != 0);
  }

  ECALC_API int eCAL_Util_GetTypeName(const char* topic_name_, void* topic_type_, int topic_type_len_)
  {
    if(!topic_name_) return(0);
    if(!topic_type_) return(0);
    std::string topic_type;
    if(eCAL::Util::GetTypeName(topic_name_, topic_type))
    {
      return(CopyBuffer(topic_type_, topic_type_len_, topic_type));
    }
    return(0);
  }

  ECALC_API int eCAL_Util_GetDescription(const char* topic_name_, void* topic_desc_, int topic_desc_len_)
  {
    if(!topic_name_) return(0);
    if(!topic_desc_) return(0);
    std::string topic_desc;
    if(eCAL::Util::GetDescription(topic_name_, topic_desc))
    {
      return(CopyBuffer(topic_desc_, topic_desc_len_, topic_desc));
    }
    return(0);
  }
}

/////////////////////////////////////////////////////////
// Process
/////////////////////////////////////////////////////////
extern "C"
{
  ECALC_API void eCAL_Process_DumpConfig()
  {
    eCAL::Process::DumpConfig();
  }

  ECALC_API int eCAL_Process_GetHostName(void* name_, int name_len_)
 {
    std::string name = eCAL::Process::GetHostName();
    if(!name.empty())
    {
      return(CopyBuffer(name_, name_len_, name));
    }
    return(0);
  }

  ECALC_API int eCAL_Process_GetHostID()
  {
    return(eCAL::Process::GetHostID());
  }

  ECALC_API int eCAL_Process_GetUnitName(void* name_, int name_len_)
  {
    std::string name = eCAL::Process::GetUnitName();
    if(!name.empty())
    {
      return(CopyBuffer(name_, name_len_, name));
    }
    return(0);
  }

  ECALC_API int eCAL_Process_GetTaskParameter(void* par_, int par_len_, const char* sep_)
  {
    std::string par = eCAL::Process::GetTaskParameter(sep_);
    if(!par.empty())
    {
      return(CopyBuffer(par_, par_len_, par));
    }
    return(0);
  }

  ECALC_API void eCAL_Process_SleepMS(long time_ms_)
  {
    eCAL::Process::SleepMS(time_ms_);
  }

  ECALC_API int eCAL_Process_GetProcessID()
  {
    return(eCAL::Process::GetProcessID());
  }

  ECALC_API int eCAL_Process_GetProcessName(void* name_, int name_len_)
  {
    std::string name = eCAL::Process::GetProcessName();
    if(!name.empty())
    {
      return(CopyBuffer(name_, name_len_, name));
    }
    return(0);
  }

  ECALC_API int eCAL_Process_GetProcessParameter(void* par_, int par_len_)
  {
    std::string par = eCAL::Process::GetProcessParameter();
    if(!par.empty())
    {
      return(CopyBuffer(par_, par_len_, par));
    }
    return(0);
  }

  ECALC_API float eCAL_Process_GetProcessCpuUsage()
  {
    return(eCAL::Process::GetProcessCpuUsage());
  }

  ECALC_API unsigned long eCAL_Process_GetProcessMemory()
  {
    return(eCAL::Process::GetProcessMemory());
  }

  ECALC_API long long eCAL_Process_GetSClock()
  {
    return(eCAL_Process_GetWClock());
  }

  ECALC_API long long eCAL_Process_GetSBytes()
  {
    return(eCAL_Process_GetWBytes());
  }

  ECALC_API long long eCAL_Process_GetWClock()
  {
    return(eCAL::Process::GetWClock());
  }

  ECALC_API long long eCAL_Process_GetWBytes()
  {
    return(eCAL::Process::GetWBytes());
  }

  ECALC_API long long eCAL_Process_GetRClock()
  {
    return(eCAL::Process::GetRClock());
  }

  ECALC_API long long eCAL_Process_GetRBytes()
  {
    return(eCAL::Process::GetRBytes());
  }

  ECALC_API void eCAL_Process_SetState(enum eCAL_Process_eSeverity severity_, enum eCAL_Process_eSeverity_Level level_, const char* info_)
  {
    eCAL::Process::SetState(severity_, level_, info_);
  }

  ECALC_API int eCAL_Process_StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, int create_console_, enum eCAL_Process_eStartMode process_mode_, int block_)
  {
    return(eCAL::Process::StartProcess(proc_name_, proc_args_, working_dir_, create_console_ != 0, process_mode_, block_ != 0));
  }

  ECALC_API int eCAL_Process_StopProcessName(const char* proc_name_)
  {
    return(eCAL::Process::StopProcess(proc_name_));
  }

  ECALC_API int eCAL_Process_StopProcessID(int proc_id_)
  {
    return(eCAL::Process::StopProcess(proc_id_));
  }
}

/////////////////////////////////////////////////////////
// Monitoring
/////////////////////////////////////////////////////////
extern "C"
{
  ECALC_API int eCAL_Monitoring_SetExclFilter(const char* filter_)
  {
    return(eCAL::Monitoring::SetExclFilter(std::string(filter_)));
  }

  ECALC_API int eCAL_Monitoring_SetInclFilter(const char* filter_)
  {
    return(eCAL::Monitoring::SetInclFilter(std::string(filter_)));
  }

  ECALC_API int eCAL_Monitoring_SetFilterState(int state_)
  {
    return(eCAL::Monitoring::SetFilterState(state_ != 0));
  }

  ECALC_API int eCAL_Monitoring_GetMonitoring(void* buf_, int buf_len_)
  {
    std::string buf;
    if(eCAL::Monitoring::GetMonitoring(buf))
    {
      return(CopyBuffer(buf_, buf_len_, buf));
    }
    return(0);
  }

  ECALC_API int eCAL_Monitoring_GetLogging(void* buf_, int buf_len_)
  {
    std::string buf;
    if(eCAL::Monitoring::GetLogging(buf))
    {
      return(CopyBuffer(buf_, buf_len_, buf));
    }
    return(0);
  }

  ECALC_API int eCAL_Monitoring_PubMonitoring(int state_, const char * name_)
  {
    return(eCAL::Monitoring::PubMonitoring(state_ != 0, name_));
  }

  ECALC_API int eCAL_Monitoring_PubLogging(int state_, const char * name_)
  {
    return(eCAL::Monitoring::PubLogging(state_ != 0, name_));
  }
}

/////////////////////////////////////////////////////////
// Logging
/////////////////////////////////////////////////////////
extern "C"
{
  ECALC_API void eCAL_Logging_SetLogLevel(enum eCAL_Logging_eLogLevel level_)
  {
    eCAL::Logging::SetLogLevel(level_);
  }

  ECALC_API enum eCAL_Logging_eLogLevel eCAL_Logging_GetLogLevel()
  {
    return(eCAL::Logging::GetLogLevel());
  }

  ECALC_API void eCAL_Logging_Log(const char* const msg_)
  {
    eCAL::Logging::Log(msg_);
  }

  ECALC_API void eCAL_Logging_StartCoreTimer()
  {
    eCAL::Logging::StartCoreTimer();
  }

  ECALC_API void eCAL_Logging_StopCoreTimer()
  {
    eCAL::Logging::StopCoreTimer();
  }

  ECALC_API void eCAL_Logging_SetCoreTime(double time_)
  {
    eCAL::Logging::SetCoreTime(time_);
  }

  ECALC_API double eCAL_Logging_GetCoreTime()
  {
    return(eCAL::Logging::GetCoreTime());
  }
}

/////////////////////////////////////////////////////////
// Event
/////////////////////////////////////////////////////////
extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Event_gOpenEvent(const char* event_name_)
  {
    eCAL::EventHandleT* event_handle = new eCAL::EventHandleT;
    bool success = eCAL::gOpenEvent(event_handle, event_name_);
    if (success)
    {
      return(event_handle);
    }
    else
    {
      delete event_handle;
      return(nullptr);
    }
  }

  ECALC_API int eCAL_Event_gCloseEvent(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    eCAL::EventHandleT* event_handle = static_cast<eCAL::EventHandleT*>(handle_);
    bool success = eCAL::gCloseEvent(*event_handle);
    delete event_handle;
    return(success);
  }

  ECALC_API int eCAL_Event_gSetEvent(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    eCAL::EventHandleT* event_handle = static_cast<eCAL::EventHandleT*>(handle_);
    return(eCAL::gSetEvent(*event_handle));
  }

  ECALC_API int eCAL_Event_gWaitForEvent(ECAL_HANDLE handle_, long timeout_)
  {
    if (handle_ == nullptr) return(0);
    eCAL::EventHandleT* event_handle = static_cast<eCAL::EventHandleT*>(handle_);
    return(eCAL::gWaitForEvent(*event_handle, timeout_));
  }

  ECALC_API int eCAL_Event_gEventIsValid(ECAL_HANDLE handle_)
  {
    if (handle_ == nullptr) return(0);
    eCAL::EventHandleT* event_handle = static_cast<eCAL::EventHandleT*>(handle_);
    return(eCAL::gEventIsValid(*event_handle));
  }
}

/////////////////////////////////////////////////////////
// Publisher
/////////////////////////////////////////////////////////
static std::recursive_mutex g_pub_callback_mtx;
static void g_pub_event_callback(const char* topic_name_, const struct eCAL::SPubEventCallbackData* data_, const PubEventCallbackCT callback_, void* par_)
{
  std::lock_guard<std::recursive_mutex> lock(g_pub_callback_mtx);
  SPubEventCallbackDataC data;
  data.time  = data_->time;
  data.type  = data_->type;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Pub_New()
  {
    eCAL::CPublisher* pub = new eCAL::CPublisher;
    return(pub);
  }

  ECALC_API int eCAL_Pub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_, const char* topic_desc_, int topic_desc_len_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (!pub->Create(topic_name_, topic_type_, std::string(topic_desc_, static_cast<size_t>(topic_desc_len_)))) return(0);
    return(1);
  }

  ECALC_API int eCAL_Pub_Destroy(ECAL_HANDLE handle_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    delete pub;
    pub = NULL;
    return(1);
  }

  ECALC_API int eCAL_Pub_SetDescription(ECAL_HANDLE handle_, const char* topic_desc_, int topic_desc_len_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->SetDescription(std::string(topic_desc_, static_cast<size_t>(topic_desc_len_)))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_ShareType(ECAL_HANDLE handle_, int state_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    pub->ShareType(state_ != 0);
    return(1);
  }

  ECALC_API int eCAL_Pub_ShareDescription(ECAL_HANDLE handle_, int state_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    pub->ShareDescription(state_ != 0);
    return(1);
  }

  ECALC_API int eCAL_Pub_SetQOS(ECAL_HANDLE handle_, struct SWriterQOSC qos_) //-V813
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    eCAL::QOS::SWriterQOS qos;
    qos.history_kind       = static_cast<eCAL::QOS::eQOSPolicy_HistoryKind>(qos_.history_kind);
    qos.history_kind_depth = qos_.history_kind_depth;
    qos.reliability        = static_cast<eCAL::QOS::eQOSPolicy_Reliability>(qos_.reliability);
    if (pub->SetQOS(qos)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_GetQOS(ECAL_HANDLE handle_, struct SWriterQOSC* qos_)
  {
    if (handle_ == NULL) return(0);
    if (qos_    == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    eCAL::QOS::SWriterQOS qos = pub->GetQOS();
    qos_->history_kind       = static_cast<eQOSPolicy_HistoryKindC>(qos.history_kind);
    qos_->history_kind_depth = qos.history_kind_depth;;
    qos_->reliability        = static_cast<eQOSPolicy_ReliabilityC>(qos.reliability);
    return(0);
  }

  ECALC_API int eCAL_Pub_SetLayerMode(ECAL_HANDLE handle_, enum eTransportLayerC layer_, enum eSendModeC mode_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->SetLayerMode(static_cast<eCAL::TLayer::eTransportLayer>(layer_), static_cast<eCAL::TLayer::eSendMode>(mode_))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_SetMaxBandwidthUDP(ECAL_HANDLE handle_, long bandwidth_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->SetMaxBandwidthUDP(bandwidth_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_SetID(ECAL_HANDLE handle_, long long id_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->SetID(id_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_IsSubscribed(ECAL_HANDLE handle_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    if(pub->IsSubscribed()) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_Send(ECAL_HANDLE handle_, const void* const buf_, int buf_len_, long long time_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    size_t ret = pub->Send(buf_, static_cast<size_t>(buf_len_), time_);
    if(static_cast<int>(ret) == buf_len_)
    {
      return(buf_len_);
    }
    return(0);
  }

  ECALC_API int eCAL_Pub_AddEventCallbackC(ECAL_HANDLE handle_, eCAL_Publisher_Event type_, PubEventCallbackCT callback_, void * par_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    auto callback = std::bind(g_pub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    if (pub->AddEventCallback(type_, callback)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_RemEventCallback(ECAL_HANDLE handle_, eCAL_Publisher_Event type_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    if (pub->RemEventCallback(type_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Pub_Dump(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if(handle_   == NULL) return(0);
    eCAL::CPublisher* pub = static_cast<eCAL::CPublisher*>(handle_);
    std::string dump = pub->Dump();
    if(!dump.empty())
    {
      return(CopyBuffer(buf_, buf_len_, dump));
    }
    return(0);
  }
}

/////////////////////////////////////////////////////////
// Subscriber
/////////////////////////////////////////////////////////
static std::recursive_mutex g_sub_callback_mtx;
static void g_sub_receive_callback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_, const ReceiveCallbackCT callback_, void* par_)
{
  std::lock_guard<std::recursive_mutex> lock(g_sub_callback_mtx);
  SReceiveCallbackDataC data;
  data.buf   = data_->buf;
  data.size  = data_->size;
  data.id    = data_->id;
  data.time  = data_->time;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

static void g_sub_event_callback(const char* topic_name_, const struct eCAL::SSubEventCallbackData* data_, const SubEventCallbackCT callback_, void* par_)
{
  std::lock_guard<std::recursive_mutex> lock(g_sub_callback_mtx);
  SSubEventCallbackDataC data;
  data.time  = data_->time;
  data.type  = data_->type;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Sub_New()
  {
    eCAL::CSubscriber* sub = new eCAL::CSubscriber;
    return(sub);
  }

  ECALC_API int eCAL_Sub_Create(ECAL_HANDLE handle_, const char* topic_name_, const char* topic_type_, const char* topic_desc_, int topic_desc_len_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    if (!sub->Create(topic_name_, topic_type_, std::string(topic_desc_, static_cast<size_t>(topic_desc_len_)))) return(0);
    return(1);
  }

  ECALC_API int eCAL_Sub_Destroy(ECAL_HANDLE handle_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    delete sub;
    sub = NULL;
    return(1);
  }

  ECALC_API int eCAL_Sub_SetQOS(ECAL_HANDLE handle_, struct SReaderQOSC qos_) //-V813
  {
    if (handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    eCAL::QOS::SReaderQOS qos;
    qos.history_kind       = static_cast<eCAL::QOS::eQOSPolicy_HistoryKind>(qos_.history_kind);
    qos.history_kind_depth = qos_.history_kind_depth;
    qos.reliability        = static_cast<eCAL::QOS::eQOSPolicy_Reliability>(qos_.reliability);
    if (sub->SetQOS(qos)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_SetID(ECAL_HANDLE handle_, const long long* id_array_, const int id_num_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    std::set<long long> id_set;
    if (id_array_ != NULL)
    {
      for (size_t i = 0; i < static_cast<size_t>(id_num_); ++i)
      {
        id_set.insert(id_array_[i]);
      }
    }
    if (sub->SetID(id_set)) return(1);
    return(1);
  }

  ECALC_API int eCAL_Sub_GetQOS(ECAL_HANDLE handle_, struct SReaderQOSC* qos_)
  {
    if (handle_ == NULL) return(0);
    if (qos_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    eCAL::QOS::SReaderQOS qos = sub->GetQOS();
    qos_->history_kind       = static_cast<eQOSPolicy_HistoryKindC>(qos.history_kind);
    qos_->history_kind_depth = qos.history_kind_depth;;
    qos_->reliability        = static_cast<eQOSPolicy_ReliabilityC>(qos.reliability);
    return(0);
  }

  ECALC_API int eCAL_Sub_Receive(ECAL_HANDLE handle_, void** buf_, int buf_len_, long long* time_, int rcv_timeout_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    std::string buf;
    if(sub->Receive(buf, time_, rcv_timeout_))
    {
      return(CopyBuffer(buf_, buf_len_, buf));
    }
    return(0);
  }

  ECALC_API int eCAL_Sub_AddReceiveCallbackC(ECAL_HANDLE handle_, ReceiveCallbackCT callback_, void* par_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    auto callback = std::bind(g_sub_receive_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    if(sub->AddReceiveCallback(callback)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_RemReceiveCallback(ECAL_HANDLE handle_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    if(sub->RemReceiveCallback()) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_AddEventCallbackC(ECAL_HANDLE handle_, eCAL_Subscriber_Event type_, SubEventCallbackCT callback_, void* par_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    auto callback = std::bind(g_sub_event_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    if (sub->AddEventCallback(type_, callback)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_RemEventCallback(ECAL_HANDLE handle_, eCAL_Subscriber_Event type_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    if (sub->RemEventCallback(type_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Sub_GetDescription(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    std::string desc = sub->GetDescription();
    int buffer_len = CopyBuffer(buf_, buf_len_, desc);
    if (buffer_len != static_cast<int>(desc.size()))
    {
      return(0);
    }
    else
    {
      return(buffer_len);
    }
  }

  ECALC_API int eCAL_Sub_SetTimeout(ECAL_HANDLE handle_, int timeout_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    return(sub->SetTimeout(timeout_));
  }

  ECALC_API int eCAL_Sub_Dump(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CSubscriber* sub = static_cast<eCAL::CSubscriber*>(handle_);
    std::string dump = sub->Dump();
    if(!dump.empty())
    {
      return(CopyBuffer(buf_, buf_len_, dump));
    }
    return(0);
  }
}

static std::recursive_mutex g_dyn_json_sub_receive_callback_mtx;
static void g_dyn_json_sub_receive_callback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_, const ReceiveCallbackCT callback_, void* par_)
{
  std::lock_guard<std::recursive_mutex> lock(g_dyn_json_sub_receive_callback_mtx);
  SReceiveCallbackDataC data;
  data.buf   = data_->buf;
  data.size  = data_->size;
  data.id    = data_->id;
  data.time  = data_->time;
  data.clock = data_->clock;
  callback_(topic_name_, &data, par_);
}

extern "C"
{
  ECAL_HANDLE eCAL_Proto_Dyn_JSON_Sub_Create(const char* topic_name_)
  {
    eCAL::protobuf::CDynamicJSONSubscriber* sub = new eCAL::protobuf::CDynamicJSONSubscriber(topic_name_);
    if (!sub->IsCreated())
    {
      delete sub;
      return(nullptr);
    }
    return(sub);
  }

  int eCAL_Proto_Dyn_JSON_Sub_Destroy(ECAL_HANDLE handle_)
  {
    eCAL::protobuf::CDynamicJSONSubscriber* sub = static_cast<eCAL::protobuf::CDynamicJSONSubscriber*>(handle_);
    if (sub)
    {
      delete sub;
      sub = nullptr;
      return(1);
    }
    return(0);
  }

  int eCAL_Proto_Dyn_JSON_Sub_AddReceiveCallbackC(ECAL_HANDLE handle_, const ReceiveCallbackCT callback_, void* par_)
  {
    eCAL::protobuf::CDynamicJSONSubscriber* sub = static_cast<eCAL::protobuf::CDynamicJSONSubscriber*>(handle_);

    auto callback = std::bind(g_dyn_json_sub_receive_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
    return(sub->AddReceiveCallback(callback));
  }

  int eCAL_Proto_Dyn_JSON_Sub_RemReceiveCallback(ECAL_HANDLE handle_)
  {
    eCAL::protobuf::CDynamicJSONSubscriber* sub = static_cast<eCAL::protobuf::CDynamicJSONSubscriber*>(handle_);

    return(sub->RemReceiveCallback());
  }
}

/////////////////////////////////////////////////////////
// Time
/////////////////////////////////////////////////////////
ECALC_API int eCAL_Time_GetName(void* name_, int name_len_)
{
  std::string name = eCAL::Time::GetName();
  if (!name.empty())
  {
    return(CopyBuffer(name_, name_len_, name));
  }
  return(0);
}

ECALC_API long long eCAL_Time_GetMicroSeconds()
{
  return(eCAL::Time::GetMicroSeconds());
}

ECALC_API long long eCAL_Time_GetNanoSeconds()
{
  return(eCAL::Time::GetNanoSeconds());
}

ECALC_API int eCAL_Time_SetNanoSeconds(long long time_)
{
  return(eCAL::Time::SetNanoSeconds(time_));
}

ECALC_API int eCAL_Time_IsTimeSynchronized()
{
  return(eCAL::Time::IsSynchronized());
}

ECALC_API int eCAL_Time_IsTimeMaster()
{
  return(eCAL::Time::IsMaster());
}

ECALC_API void eCAL_Time_SleepForNanoseconds(long long duration_nsecs_)
{
  eCAL::Time::SleepForNanoseconds(duration_nsecs_);
}

ECALC_API int eCAL_Time_GetStatus(int* error_, char** status_message_, const int max_len_)
{
  if (max_len_ == ECAL_ALLOCATE_4ME || max_len_ > 0)
  {
    std::string status_message;
    eCAL::Time::GetStatus(*error_, &status_message);

    if (!status_message.empty())
    {
      return CopyBuffer(status_message_, max_len_, status_message);
    }
    return 0;
  }
  else 
  {
    eCAL::Time::GetStatus(*error_, nullptr);
    return 0;
  }
}

/////////////////////////////////////////////////////////
// Timer
/////////////////////////////////////////////////////////
static std::recursive_mutex g_timer_callback_mtx;
static void g_timer_callback(const TimerCallbackCT callback_, void* par_)
{
  std::lock_guard<std::recursive_mutex> lock(g_timer_callback_mtx);
  callback_(par_);
}

extern "C"
{
  ECALC_API ECAL_HANDLE eCAL_Timer_Create()
  {
    eCAL::CTimer* timer = new eCAL::CTimer;
    return(timer);
  }

  ECALC_API int eCAL_Timer_Destroy(ECAL_HANDLE handle_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CTimer* timer = static_cast<eCAL::CTimer*>(handle_);
    delete timer;
    timer = NULL;
    return(1);
  }

  ECALC_API int eCAL_Timer_Start(ECAL_HANDLE handle_, int timeout_, TimerCallbackCT callback_, int delay_, void* par_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CTimer* timer = static_cast<eCAL::CTimer*>(handle_);
    auto callback = std::bind(g_timer_callback, callback_, par_);
    if(timer->Start(timeout_, callback, delay_)) return(1);
    else                                         return(0);
  }

  ECALC_API int eCAL_Timer_Stop(ECAL_HANDLE handle_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CTimer* timer = static_cast<eCAL::CTimer*>(handle_);
    if(timer->Stop()) return(1);
    else              return(0);
  }
}

/////////////////////////////////////////////////////////
// Service Server
/////////////////////////////////////////////////////////
extern "C"
{
  static std::recursive_mutex g_request_callback_mtx;
  static int g_method_callback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_, MethodCallbackCT callback_, void* par_)
  {
    std::lock_guard<std::recursive_mutex> lock(g_request_callback_mtx);
    void* response(nullptr);
    int   response_len(ECAL_ALLOCATE_4ME);
    int ret_state = callback_(method_.c_str(), req_type_.c_str(), resp_type_.c_str(), request_.c_str(), static_cast<int>(request_.size()), &response, &response_len, par_);
    if (response_len > 0)
    {
      response_ = std::string(static_cast<const char*>(response), static_cast<size_t>(response_len));
    }
    return ret_state;
  }

  ECALC_API ECAL_HANDLE eCAL_Server_Create(const char* service_name_)
  {
    if (service_name_ == NULL) return(NULL);
    eCAL::CServiceServer* server = new eCAL::CServiceServer(service_name_);
    return(server);
  }

  ECALC_API int eCAL_Server_Destroy(ECAL_HANDLE handle_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CServiceServer* server = static_cast<eCAL::CServiceServer*>(handle_);
    delete server;
    server = NULL;
    return(1);
  }

  ECALC_API int eCAL_Server_AddMethodCallbackC(ECAL_HANDLE handle_, const char* method_, const char* req_type_, const char* resp_type_, MethodCallbackCT callback_, void* par_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CServiceServer* server = static_cast<eCAL::CServiceServer*>(handle_);
    auto callback = std::bind(g_method_callback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, callback_, par_);
    return server->AddMethodCallback(method_, req_type_, resp_type_, callback);
  }

  ECALC_API int eCAL_Server_RemMethodCallbackC(ECAL_HANDLE handle_, const char* method_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CServiceServer* server = static_cast<eCAL::CServiceServer*>(handle_);
    return server->RemMethodCallback(method_);
  }


  ECALC_API int eCAL_Server_GetServiceName(ECAL_HANDLE handle_, void* buf_, int buf_len_)
  {
    if (handle_ == NULL) return(0);
    eCAL::CServiceServer* server = static_cast<eCAL::CServiceServer*>(handle_);
    std::string service_name = server->GetServiceName();
    int buffer_len = CopyBuffer(buf_, buf_len_, service_name);
    if (buffer_len != static_cast<int>(service_name.size()))
    {
      return(0);
    }
    else
    {
      return(buffer_len);
    }
  }
}

/////////////////////////////////////////////////////////
// Service Client
/////////////////////////////////////////////////////////
extern "C"
{
  static std::recursive_mutex g_response_callback_mtx;
  static void g_response_callback(const struct eCAL::SServiceInfo& service_info_, const std::string& response_, const ResponseCallbackCT callback_, void* par_)
  {
    std::lock_guard<std::recursive_mutex> lock(g_response_callback_mtx);
    struct SServiceInfoC service_info;
    service_info.host_name     = service_info_.host_name.c_str();
    service_info.service_name  = service_info_.service_name.c_str();
    service_info.method_name   = service_info_.method_name.c_str();
    service_info.error_msg     = service_info_.error_msg.c_str();
    service_info.ret_state     = service_info_.ret_state;
    service_info.call_state    = service_info_.call_state;
    callback_(&service_info, response_.c_str(), static_cast<int>(response_.size()), par_);
  }

  ECALC_API ECAL_HANDLE eCAL_Client_Create(const char* service_name_)
  {
    if(service_name_ == NULL) return(NULL);
    eCAL::CServiceClient* client = new eCAL::CServiceClient(service_name_);
    return(client);
  }

  ECALC_API int eCAL_Client_Destroy(ECAL_HANDLE handle_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
    delete client;
    client = NULL;
    return(1);
  }

  ECALC_API int eCAL_Client_SetHostName(ECAL_HANDLE handle_, const char* host_name_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
    if(client->SetHostName(host_name_)) return(1);
    return(0);
  }

  ECALC_API int eCAL_Client_Call(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
    if(client->Call(method_name_, std::string(request_, static_cast<size_t>(request_len_)))) return(1);
    return(0);
  }

  ECALC_API int eCAL_Client_Call_Wait(ECAL_HANDLE handle_, const char* host_name_, const char* method_name_, const char* request_, int request_len_, struct SServiceInfoC* service_info_, void* response_, int response_len_)
  {
    if(handle_ == NULL) return(0);
    eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
    std::string response;
    eCAL::SServiceInfo service_info;
    if(client->Call(host_name_, method_name_, std::string(request_, static_cast<size_t>(request_len_)), service_info, response))
    {
      service_info_->host_name    = NULL;
      service_info_->service_name = NULL;
      service_info_->method_name  = NULL;
      service_info_->error_msg    = NULL;
      service_info_->ret_state    = service_info.ret_state;
      service_info_->call_state   = service_info.call_state;
      return(CopyBuffer(response_, response_len_, response));
    }
    return(0);
  }
}

int eCAL_Client_AddResponseCallbackC(ECAL_HANDLE handle_, ResponseCallbackCT callback_, void* par_)
{
  if(handle_ == NULL) return(0);
  eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
  auto callback = std::bind(g_response_callback, std::placeholders::_1, std::placeholders::_2, callback_, par_);
  return client->AddResponseCallback(callback);
}

int eCAL_Client_RemResponseCallback(ECAL_HANDLE handle_)
{
  if(handle_ == NULL) return(0);
  eCAL::CServiceClient* client = static_cast<eCAL::CServiceClient*>(handle_);
  return client->RemResponseCallback();
}
