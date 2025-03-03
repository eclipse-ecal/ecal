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
  * @file  ecal_clr_subscriber.cpp
  */

#include "ecal_clr_subscriber.h"
#include "ecal_clr_common.h"
#include "ecal_clr_datatype.h"
#include "ecal_clr_topicid.h"
#include "ecal_clr_subscribercallback.h"
#include "ecal_clr_receivecallbackdata.h"

#include <ecal/pubsub/subscriber.h>
#include <sstream>
#include <vcclr.h>
#include <functional>

using namespace Continental::eCAL::Core;
using namespace System::Runtime::InteropServices;
using namespace Internal;

namespace
{
  /**
   * @brief Helper function to convert native SReceiveCallbackData to managed ReceiveCallbackData.
   *
   * @param nativeData The native SReceiveCallbackData.
   * @return A managed ReceiveCallbackData^ instance.
   */
  ReceiveCallbackData^ ConvertReceiveCallbackData(const eCAL::SReceiveCallbackData& nativeData)
  {
    ReceiveCallbackData^ data = gcnew ReceiveCallbackData();
    data->BufferSize = static_cast<int>(nativeData.buffer_size);
    data->SendTimestamp = nativeData.send_timestamp;
    data->SendClock = nativeData.send_clock;
    if (nativeData.buffer && nativeData.buffer_size > 0)
    {
      data->Buffer = gcnew array<Byte>(static_cast<int>(nativeData.buffer_size));
      Marshal::Copy(IntPtr((void*)nativeData.buffer), data->Buffer, 0, static_cast<int>(nativeData.buffer_size));
    }
    else
    {
      data->Buffer = gcnew array<Byte>(0);
    }
    return data;
  }

  /**
   * @brief Helper function to convert native STopicId to managed TopicId.
   *
   * @param nativeId The native STopicId.
   * @return A managed TopicId^ instance.
   */
  TopicId^ ConvertTopicId(const eCAL::STopicId& nativeId)
  {
    unsigned __int64 entityId = nativeId.topic_id.entity_id;
    String^ topicName = StlStringToString(nativeId.topic_name);
    return gcnew TopicId(entityId, topicName);
  }

  /**
   * @brief Helper function to create a native callback from a managed ReceiveCallbackDelegate.
   *
   * @param callback The managed callback delegate.
   * @return A std::function wrapping the managed callback.
   */
  static std::function<void(const ::eCAL::STopicId&, const ::eCAL::SDataTypeInformation&, const ::eCAL::SReceiveCallbackData&)>
    CreateNativeReceiveCallback(ReceiveCallbackDelegate^ callback)
  {
    gcroot<ReceiveCallbackDelegate^> managedCallback(callback);
    return [managedCallback](const ::eCAL::STopicId& publisherId,
      const ::eCAL::SDataTypeInformation& dataTypeInfo,
      const ::eCAL::SReceiveCallbackData& data)
      {
        TopicId^ managedId = ConvertTopicId(publisherId);
        DataTypeInformation^ managedDT = gcnew DataTypeInformation(
          StlStringToString(dataTypeInfo.name),
          StlStringToString(dataTypeInfo.encoding),
          StlStringToString(dataTypeInfo.descriptor));
        ReceiveCallbackData^ managedData = ConvertReceiveCallbackData(data);
        managedCallback->Invoke(managedId, managedDT, managedData);
      };
  }
}

// Constructors
Subscriber::Subscriber(String^ topicName)
{
  std::string nativeTopic = StringToStlString(topicName);
  m_subscriber = new ::eCAL::CSubscriber(nativeTopic);
}

Subscriber::Subscriber(String^ topicName, DataTypeInformation^ dataTypeInfo)
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo;
  nativeDataTypeInfo.name       = StringToStlString(dataTypeInfo->Name);
  nativeDataTypeInfo.encoding   = StringToStlString(dataTypeInfo->Encoding);
  nativeDataTypeInfo.descriptor = StringToStlString(dataTypeInfo->Descriptor);
  m_subscriber = new ::eCAL::CSubscriber(StringToStlString(topicName), nativeDataTypeInfo);
}

// Destructor
Subscriber::~Subscriber()
{
  this->!Subscriber();
}

// Finalizer
Subscriber::!Subscriber()
{
  if (m_subscriber != nullptr)
  {
    delete m_subscriber;
    m_subscriber = nullptr;
  }
}

// SetReceiveCallback
bool Subscriber::SetReceiveCallback(ReceiveCallbackDelegate^ callback)
{
  m_receiveCallback = callback;
  auto nativeCallback = CreateNativeReceiveCallback(callback);
  return m_subscriber->SetReceiveCallback(nativeCallback);
}

// RemoveReceiveCallback
bool Subscriber::RemoveReceiveCallback()
{
  m_receiveCallback = nullptr;
  return m_subscriber->RemoveReceiveCallback();
}

int Subscriber::GetPublisherCount()
{
  return static_cast<int>(m_subscriber->GetPublisherCount());
}

String^ Subscriber::GetTopicName()
{
  std::string nativeTopic = m_subscriber->GetTopicName();
  return StlStringToString(nativeTopic);
}

TopicId^ Subscriber::GetTopicId()
{
  ::eCAL::STopicId nativeId = m_subscriber->GetTopicId();
  return ConvertTopicId(nativeId);
}

DataTypeInformation^ Subscriber::GetDataTypeInformation()
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo = m_subscriber->GetDataTypeInformation();
  return gcnew DataTypeInformation(
    StlStringToString(nativeDataTypeInfo.name),
    StlStringToString(nativeDataTypeInfo.encoding),
    StlStringToString(nativeDataTypeInfo.descriptor));
}
