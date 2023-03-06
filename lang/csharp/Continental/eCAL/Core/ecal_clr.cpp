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

std::string ByteArrayToStlString(array<Byte>^ array_)
{
  GCHandle handle = GCHandle::Alloc(array_, GCHandleType::Pinned);
  size_t len = array_->Length;
  std::string ret((const char*)(void*)handle.AddrOfPinnedObject(), len);
  handle.Free();
  return(ret);
}

array<Byte>^ StlStringToByteArray(const std::string& string_)
{
  array<Byte>^ array_ = gcnew array<Byte>(string_.size());
  System::Runtime::InteropServices::Marshal::Copy(IntPtr((void*)(string_.data())), array_, 0, string_.size());
  return(array_);
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

Publisher::Publisher(System::String^ topic_name_, System::String^ topic_type_, array<Byte>^ topic_desc_)
{
  m_pub = new ::eCAL::CPublisher(StringToStlString(topic_name_), StringToStlString(topic_type_), ByteArrayToStlString(topic_desc_));
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

size_t Publisher::Send(array<Byte>^ buffer, long long time_)
{
  if(m_pub == nullptr) return(0); 
  GCHandle handle = GCHandle::Alloc(buffer, GCHandleType::Pinned);
  size_t len = buffer->Length;
  size_t ret = m_pub->Send((void*)handle.AddrOfPinnedObject(), len, time_);
  handle.Free();
  return(ret);
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

Subscriber::Subscriber(System::String^ topic_name_, System::String^ topic_type_, array<Byte>^ topic_desc_)
{
  m_sub = new ::eCAL::CSubscriber(StringToStlString(topic_name_), StringToStlString(topic_type_), ByteArrayToStlString(topic_desc_));
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

Subscriber::ReceiveCallbackDataUnsafe^ Subscriber::ReceiveUnsafe(const int rcv_timeout_)
{
  if(m_sub == nullptr) return(nullptr);
  long long rcv_time = 0;
  std::string rcv_buf;
  auto size = m_sub->Receive(rcv_buf, &rcv_time, rcv_timeout_);
  if (size > 0)
  {
    ReceiveCallbackDataUnsafe^ rcv_data = gcnew ReceiveCallbackDataUnsafe();
    rcv_data->id = 0;
    rcv_data->clock = 0;
    rcv_data->time = rcv_time;
    rcv_data->data = (void*)rcv_buf.data();
    rcv_data->size = size;
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

bool Subscriber::AddReceiveCallback(ReceiverCallbackUnsafe^ callback_)
{
    if (m_sub == nullptr) return(false);
    if (m_callbacks_unsafe == nullptr)
    {
        m_sub_callback = gcnew subCallback(this, &Subscriber::OnReceiveUnsafe);
        m_gch_unsafe = GCHandle::Alloc(m_sub_callback);
        IntPtr ip = Marshal::GetFunctionPointerForDelegate(m_sub_callback);
        m_sub->AddReceiveCallback(static_cast<stdcall_eCAL_ReceiveCallbackT>(ip.ToPointer()));
    }
    m_callbacks_unsafe += callback_;
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
bool Subscriber::RemReceiveCallback(ReceiverCallbackUnsafe^ callback_)
{
    if (m_sub == nullptr) return(false);
    if (m_callbacks_unsafe == callback_)
    {
        m_sub->RemReceiveCallback();
        m_gch_unsafe.Free();
    }
    m_callbacks_unsafe -= callback_;

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
void Subscriber::OnReceiveUnsafe(const char* topic_name_, const ::eCAL::SReceiveCallbackData* data_)
{
  ReceiveCallbackDataUnsafe^ data = gcnew ReceiveCallbackDataUnsafe();
  data->data    = data_->buf;
  data->size    = data_->size;
  data->id      = data_->id;
  data->time    = data_->time;
  data->clock   = data_->clock;
  std::string topic_name = std::string(topic_name_);
  m_callbacks_unsafe(StlStringToString(topic_name), data);
}


/////////////////////////////////////////////////////////////////////////////
// ServiceServer
/////////////////////////////////////////////////////////////////////////////
ServiceServer::ServiceServer() : m_serv(new ::eCAL::CServiceServer())
{
}

ServiceServer::ServiceServer(System::String^ server_name_)
{
  m_serv = new ::eCAL::CServiceServer(StringToStlString(server_name_));
}

ServiceServer::~ServiceServer()
{
  if(m_serv == nullptr) return;
  delete m_serv;
}

bool ServiceServer::Destroy()
{
  if(m_serv == nullptr) return(false);
  return(m_serv->Destroy());
}

bool ServiceServer::AddMethodCallback(String^ methodName, String^ reqType, String^ responseType, MethodCallback^ callback_)
{
  if(m_serv == nullptr) return(false);

  if (m_callbacks == nullptr)
  {
    m_sub_callback = gcnew servCallback(this, &ServiceServer::OnMethodCall);
    m_gch = GCHandle::Alloc(m_sub_callback);
    IntPtr ip = Marshal::GetFunctionPointerForDelegate(m_sub_callback);
    m_serv->AddMethodCallback(StringToStlString(methodName), StringToStlString(reqType), StringToStlString(responseType), static_cast<stdcall_eCAL_MethodCallbackT>(ip.ToPointer()));
  }
  m_callbacks += callback_;
  return(true);
}

bool ServiceServer::RemMethodCallback(String^ methodName, MethodCallback^ callback_)
{
  if(m_serv == nullptr) return(false);

  if (m_callbacks == callback_)
  {
    m_serv->RemMethodCallback(StringToStlString(methodName));
    m_gch.Free();
  }
  m_callbacks -= callback_;

  return(false);
}

int ServiceServer::OnMethodCall(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_)
{
    array<Byte>^ result = m_callbacks(StlStringToString(method_), StlStringToString(method_), StlStringToString(method_), StlStringToByteArray(request_));
    response_ = ByteArrayToStlString(result);
    return 42;
}


/////////////////////////////////////////////////////////////////////////////
// ServiceClient
/////////////////////////////////////////////////////////////////////////////
ServiceClient::ServiceClient() : m_client(new ::eCAL::CServiceClient())
{
}

ServiceClient::ServiceClient(System::String^ service_name_)
{
    m_client = new ::eCAL::CServiceClient(StringToStlString(service_name_));
}

ServiceClient::~ServiceClient()
{
    if (m_client == nullptr) return;
    delete m_client;
}

// What behavior is expected here
// should we throw an exception? is that more C# like?
List<ServiceClient::ServiceClientCallbackData^>^ ServiceClient::Call(System::String^ method_name_, array<Byte>^ request, const int rcv_timeout_)
{
    if (m_client == nullptr) return(nullptr);
    ::eCAL::ServiceResponseVecT responseVecT;

    if (m_client->Call(StringToStlString(method_name_), ByteArrayToStlString(request), rcv_timeout_, &responseVecT))
    {
        List<ServiceClientCallbackData^>^ rcv_Datas = gcnew List<ServiceClientCallbackData^>();

        for each (::eCAL::SServiceResponse response in responseVecT)
        {
            ServiceClientCallbackData^ rcv_data = gcnew ServiceClientCallbackData;
            rcv_data->call_state = static_cast<CallState>(response.call_state);
            rcv_data->error_msg = StlStringToString(response.error_msg);
            rcv_data->host_name = StlStringToString(response.host_name);
            rcv_data->method_name = StlStringToString(response.method_name);
            rcv_data->ret_state = response.ret_state;
            rcv_data->service_id = StlStringToString(response.service_id);
            rcv_data->service_name = StlStringToString(response.service_name);
            rcv_data->ret_state = response.ret_state;
            rcv_data->response = StlStringToByteArray(response.response);
            rcv_Datas->Add(rcv_data);
        }
        return rcv_Datas;
    }
    else
    {
        return nullptr;
    }
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

String^ Monitoring::GetMonitoring()
{
    std::string monitoring;
    ::eCAL::Monitoring::GetMonitoring(monitoring);
    return StlStringToString(monitoring);
}

String^ Monitoring::GetLogging()
{
    std::string logging;
    ::eCAL::Monitoring::GetLogging(logging);
    return StlStringToString(logging);
}

array<Byte>^ Monitoring::GetMonitoringBytes()
{
    std::string monitoring;
    ::eCAL::Monitoring::GetMonitoring(monitoring);
    array<Byte>^ data = gcnew array<Byte>(monitoring.size());
    System::Runtime::InteropServices::Marshal::Copy(IntPtr(&monitoring[0]), data, 0, monitoring.size());
    return data;
}

array<Byte>^ Monitoring::GetLoggingBytes()
{
    std::string logging;
    ::eCAL::Monitoring::GetLogging(logging);
    array<Byte>^ data = gcnew array<Byte>(logging.size());
    System::Runtime::InteropServices::Marshal::Copy(IntPtr(&logging[0]), data, 0, logging.size());
    return data;
}

DateTime Continental::eCAL::Core::Monitoring::GetTime()
{
    return DateTime(::eCAL::Time::GetMicroSeconds());
}
