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
 * @brief  eCAL C# interface
**/

#include "ecal_clr.h"

using namespace Continental::eCAL::Core;


/////////////////////////////////////////////////////////////////////////////
// Help
/////////////////////////////////////////////////////////////////////////////
System::String^ StlStringToString(const std::string& ss_)
{
  String^ s = gcnew String(ss_.c_str(), 0, static_cast<int>(ss_.length()));
  return(s);
}

std::string StringToStlString(System::String^ s_)
{
  std::string s = std::string((const char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(s_).ToPointer(), s_->Length);
  return(s);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Using the eCAL static library interface
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Core
/////////////////////////////////////////////////////////////////////////////
void Util::Initialize(System::String^ task_name_)
{
  ::eCAL::Initialize(0, nullptr, StringToStlString(task_name_).c_str());
}

void Util::Terminate()
{
  ::eCAL::Finalize();
}


/////////////////////////////////////////////////////////////////////////////
// Util
/////////////////////////////////////////////////////////////////////////////
System::String^ Util::GetVersion()
{
  return(ECAL_VERSION);
}

System::String^ Util::GetDate()
{
  return(ECAL_DATE);
}

bool Util::Ok()
{
  return(::eCAL::Ok());
}

void Util::ShutdownProcesses()
{
  ::eCAL::Util::ShutdownProcesses();
}

void Util::ShutdownCore()
{
  ::eCAL::Util::ShutdownCore();
}

void Util::EnableLoopback(bool state_)
{
  ::eCAL::Util::EnableLoopback(state_);
}


/////////////////////////////////////////////////////////////////////////////
// Logger
/////////////////////////////////////////////////////////////////////////////
void Logger::SetLogLevel(LogLevel level_)
{
  switch(level_)
  {
  case info:
    ::eCAL::Logging::SetLogLevel(log_level_info);
    break;
  case warning:
    ::eCAL::Logging::SetLogLevel(log_level_warning);
    break;
  case error:
    ::eCAL::Logging::SetLogLevel(log_level_error);
    break;
  case fatal:
    ::eCAL::Logging::SetLogLevel(log_level_fatal);
    break;
  }
}

void Logger::Log(System::String^ message_)
{
  ::eCAL::Logging::Log(StringToStlString(message_));
}

void Logger::SetCoreTime(double time_)
{
  ::eCAL::Logging::SetCoreTime(time_);
}


/////////////////////////////////////////////////////////////////////////////
// Publisher
/////////////////////////////////////////////////////////////////////////////
Publisher::Publisher() : m_pub(new ::eCAL::CPublisher())
{
}

Publisher::Publisher(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_desc_)
{
  m_pub = new ::eCAL::CPublisher(StringToStlString(topic_name_), StringToStlString(topic_type_), StringToStlString(topic_desc_));
}

Publisher::~Publisher()
{
  if(m_pub == nullptr) return;
  delete m_pub;
}

bool Publisher::Create(System::String^ topic_name_, System::String^ topic_type_)
{
  if(m_pub == nullptr) return(false);
  return(m_pub->Create(StringToStlString(topic_name_), StringToStlString(topic_type_)));
}

bool Publisher::Destroy()
{
  if(m_pub == nullptr) return(false);
  return(m_pub->Destroy());
}

size_t Publisher::Send(System::String^ s_, long long time_)
{
  if(m_pub == nullptr) return(0);
  void*  buf = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(s_).ToPointer();
  size_t len = s_->Length;
  return(m_pub->Send(buf, len, time_));
}

bool Publisher::IsCreated()
{
  if(m_pub == nullptr) return(false);
  return(m_pub->IsCreated());
}

System::String^ Publisher::GetTopicName()
{
  if(m_pub == nullptr) return("");
  return(StlStringToString(m_pub->GetTopicName()));
}

System::String^ Publisher::GetTypeName()
{
  if(m_pub == nullptr) return("");
  return(StlStringToString(m_pub->GetTypeName()));
}

System::String^ Publisher::Dump()
{
  if(m_pub == nullptr) return("");
  return(StlStringToString(m_pub->Dump()));
}


/////////////////////////////////////////////////////////////////////////////
// Subscriber
/////////////////////////////////////////////////////////////////////////////
Subscriber::Subscriber() : m_sub(new ::eCAL::CSubscriber())
{
}

Subscriber::Subscriber(System::String^ topic_name_, System::String^ topic_type_, System::String^ topic_desc_)
{
  m_sub = new ::eCAL::CSubscriber(StringToStlString(topic_name_), StringToStlString(topic_type_), StringToStlString(topic_desc_));
}

Subscriber::~Subscriber()
{
  if(m_sub == nullptr) return;
  delete m_sub;
}

bool Subscriber::Create(System::String^ topic_name_, System::String^ topic_type_)
{
  if(m_sub == nullptr) return(false);
  return(m_sub->Create(StringToStlString(topic_name_), StringToStlString(topic_type_)));
}

bool Subscriber::Destroy()
{
  if(m_sub == nullptr) return(false);
  return(m_sub->Destroy());
}

Subscriber::ReceiveCallbackData^ Subscriber::Receive(const int rcv_timeout_)
{
  if(m_sub == nullptr) return(nullptr);
  long long rcv_time = 0;
  std::string rcv_buf;

  auto size = m_sub->Receive(rcv_buf, &rcv_time, rcv_timeout_);
  if (size > 0)
  {
    ReceiveCallbackData^ rcv_data = gcnew ReceiveCallbackData();
    rcv_data->id = 0;
    rcv_data->clock = 0;
    rcv_data->time = rcv_time;
    rcv_data->data = StlStringToString(rcv_buf);
    return rcv_data;
  }
  else
  {
    return nullptr;
  }
}

bool Subscriber::AddReceiveCallback(ReceiverCallback^ callback_)
{
  if(m_sub == nullptr) return(false);

  if (m_callbacks == nullptr)
  {
    m_sub_callback = gcnew subCallback(this, &Subscriber::OnReceive);
    m_gch = GCHandle::Alloc(m_sub_callback);
    IntPtr ip = Marshal::GetFunctionPointerForDelegate(m_sub_callback);
    m_sub->AddReceiveCallback(static_cast<stdcall_eCAL_ReceiveCallbackT>(ip.ToPointer()));
  }
  m_callbacks += callback_;
  return(true);
}

bool Subscriber::RemReceiveCallback(ReceiverCallback^ callback_)
{
  if(m_sub == nullptr) return(false);

  if (m_callbacks == callback_)
  {
    m_sub->RemReceiveCallback();
    m_gch.Free();
  }
  m_callbacks -= callback_;

  return(false);
}

bool Subscriber::IsCreated()
{
  if(m_sub == nullptr) return(false);
  return(m_sub->IsCreated());
}

System::String^ Subscriber::GetTopicName()
{
  if(m_sub == nullptr) return("");
  return(StlStringToString(m_sub->GetTopicName()));
}

System::String^ Subscriber::GetTypeName()
{
  if(m_sub == nullptr) return("");
  return(StlStringToString(m_sub->GetTypeName()));
}

System::String^ Subscriber::Dump()
{
  if(m_sub == nullptr) return("");
  return(StlStringToString(m_sub->Dump()));
}

void Subscriber::OnReceive(const char* topic_name_, const ::eCAL::SReceiveCallbackData* data_)
{
  std::string received_bytes = std::string(static_cast<const char*>(data_->buf), static_cast<size_t>(data_->size));
  ReceiveCallbackData^ data = gcnew ReceiveCallbackData();
  data->data    = StlStringToString(received_bytes);
  data->id      = data_->id;
  data->time    = data_->time;
  data->clock   = data_->clock;
  std::string topic_name = std::string(topic_name_);
  m_callbacks(StlStringToString(topic_name), data);
}





/////////////////////////////////////////////////////////////////////////////
// JSONProtobufSubscriber
/////////////////////////////////////////////////////////////////////////////
JSONProtobufSubscriber::JSONProtobufSubscriber() : m_sub(new ::eCAL::protobuf::CDynamicJSONSubscriber())
{
}

JSONProtobufSubscriber::JSONProtobufSubscriber(System::String^ topic_name_)
{
  m_sub = new ::eCAL::protobuf::CDynamicJSONSubscriber(StringToStlString(topic_name_));
}

JSONProtobufSubscriber::~JSONProtobufSubscriber()
{
  if(m_sub == nullptr) return;
  delete m_sub;
}

void JSONProtobufSubscriber::Create(System::String^ topic_name_)
{
  if(m_sub == nullptr) return;
  m_sub->Create(StringToStlString(topic_name_));
}

void JSONProtobufSubscriber::Destroy()
{
  if(m_sub == nullptr) return;
  m_sub->Destroy();
}


bool JSONProtobufSubscriber::AddReceiveCallback(ReceiverCallback^ callback_)
{
  if(m_sub == nullptr) return(false);

  if (m_callbacks == nullptr)
  {
    m_sub_callback = gcnew subCallback(this, &JSONProtobufSubscriber::OnReceive);
    m_gch = GCHandle::Alloc(m_sub_callback);
    IntPtr ip = Marshal::GetFunctionPointerForDelegate(m_sub_callback);
    m_sub->AddReceiveCallback(static_cast<stdcall_eCAL_ReceiveCallbackT>(ip.ToPointer()));
  }
  m_callbacks += callback_;
  return(true);
}

bool JSONProtobufSubscriber::RemReceiveCallback(ReceiverCallback^ callback_)
{
  if(m_sub == nullptr) return(false);

  if (m_callbacks == callback_)
  {
    m_sub->RemReceiveCallback();
    m_gch.Free();
  }
  m_callbacks -= callback_;

  return(false);
}

bool JSONProtobufSubscriber::IsCreated()
{
  if(m_sub == nullptr) return(false);
  return(m_sub->IsCreated());
}

void JSONProtobufSubscriber::OnReceive(const char* topic_name_, const ::eCAL::SReceiveCallbackData* data_)
{
  std::string received_bytes = std::string(static_cast<const char*>(data_->buf), static_cast<size_t>(data_->size));
  ReceiveCallbackData^ data = gcnew ReceiveCallbackData();
  data->data    = StlStringToString(received_bytes);
  data->id      = data_->id;
  data->time    = data_->time;
  data->clock   = data_->clock;
  std::string topic_name = std::string(topic_name_);
  m_callbacks(StlStringToString(topic_name), data);
}




/////////////////////////////////////////////////////////////////////////////
// Monitoring
/////////////////////////////////////////////////////////////////////////////
void Monitoring::Initialize()
{
  ::eCAL::Initialize(0, nullptr, "", ::eCAL::Init::Monitoring);
}

void Monitoring::Terminate()
{
  ::eCAL::Finalize();
}

System::String^ Monitoring::GetMonitoring()
{
  std::string monitoring;
  ::eCAL::Monitoring::GetMonitoring(monitoring);
  return(StlStringToString(monitoring));
}

System::String^ Monitoring::GetLogging()
{
  std::string logging;
  ::eCAL::Monitoring::GetLogging(logging);
  return(StlStringToString(logging));
}