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

#include "ecal_clr_subscriber.h"

#include "ecal_clr_common.h"
#include "ecal_clr_types.h"
#include "ecal_clr_types_pubsub.h"

#include <sstream>
#include <vcclr.h>
#include <functional>

using namespace Eclipse::eCAL::Core;
using namespace System::Runtime::InteropServices;
using namespace Internal;

namespace
{
  /**
   * @brief Helper function to convert native SReceiveCallbackData to managed ReceiveCallbackData.
   *
   * @param nativeData The native SReceiveCallbackData.
   * 
   * @return A managed ReceiveCallbackData^ instance.
   */
  ReceiveCallbackData^ ConvertReceiveCallbackData(const eCAL::SReceiveCallbackData& nativeData)
  {
    ReceiveCallbackData^ data = gcnew ReceiveCallbackData();
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
   * @param nativeTopicId The native STopicId.
   * 
   * @return A managed TopicId^ instance.
   */
  TopicId^ ConvertTopicId(const eCAL::STopicId& nativeTopicId)
  {
    return gcnew TopicId(
      gcnew EntityId(
        nativeTopicId.topic_id.entity_id,
        nativeTopicId.topic_id.process_id,
        StlStringToString(nativeTopicId.topic_id.host_name)
      ),
      StlStringToString(nativeTopicId.topic_name)
    );
  }

  /**
   * @brief Helper function to create a native callback from a managed ReceiveCallbackDelegate.
   *
   * @param callback The managed callback delegate.
   * 
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
        StlStringToByteArray(dataTypeInfo.descriptor)
      );
      ReceiveCallbackData^ managedData = ConvertReceiveCallbackData(data);
      managedCallback->Invoke(managedId, managedDT, managedData);
    };
  }

  /**
   * @brief Helper function to create a native subscriber event callback from a managed SubscriberEventCallbackDelegate.
   *
   * @param callback The managed subscriber event callback delegate.
   * 
   * @return A std::function wrapping the managed callback.
   */
  static std::function<void(const ::eCAL::STopicId&, const ::eCAL::SSubEventCallbackData&)>
    CreateNativeSubscriberEventCallback(gcroot<SubscriberEventCallbackDelegate^> callback)
  {
    return [callback](const ::eCAL::STopicId& nativeTopicId, const ::eCAL::SSubEventCallbackData& nativeData)
    {
      TopicId^ managedTopicId = ConvertTopicId(nativeTopicId);
      SubEventCallbackData^ managedData = gcnew SubEventCallbackData();
      managedData->EventType = static_cast<SubscriberEvent>(nativeData.event_type);
      managedData->EventTime = nativeData.event_time;
      managedData->PublisherDataType = gcnew DataTypeInformation(
        StlStringToString(nativeData.publisher_datatype.name),
        StlStringToString(nativeData.publisher_datatype.encoding),
        StlStringToByteArray(nativeData.publisher_datatype.descriptor)
      );
      callback->Invoke(managedTopicId, managedData);
    };
  }
}

// Constructor
Subscriber::Subscriber(String^ topicName, DataTypeInformation^ dataTypeInfo, SubscriberEventCallbackDelegate^ eventCallback)
{
  std::string nativeTopic = StringToStlString(topicName);

  // Use a default DataTypeInformation if none is provided.
  if (dataTypeInfo == nullptr)
  {
    dataTypeInfo = gcnew DataTypeInformation("", "", gcnew array<Byte>(0));
  }

  ::eCAL::SDataTypeInformation nativeDataTypeInfo;
  nativeDataTypeInfo.name       = StringToStlString(dataTypeInfo->Name);
  nativeDataTypeInfo.encoding   = StringToStlString(dataTypeInfo->Encoding);
  nativeDataTypeInfo.descriptor = ByteArrayToStlString(dataTypeInfo->Descriptor);

  if (eventCallback != nullptr)
  {
    gcroot<SubscriberEventCallbackDelegate^> managedCallback(eventCallback);
    auto nativeCallback = CreateNativeSubscriberEventCallback(managedCallback);
    m_native_subscriber = new ::eCAL::CSubscriber(nativeTopic, nativeDataTypeInfo, nativeCallback);
  }
  else
  {
    m_native_subscriber = new ::eCAL::CSubscriber(nativeTopic, nativeDataTypeInfo);
  }
}

// Destructor
Subscriber::~Subscriber()
{
  this->!Subscriber();
}

// Finalizer
Subscriber::!Subscriber()
{
  if (m_native_subscriber != nullptr)
  {
    delete m_native_subscriber;
    m_native_subscriber = nullptr;
  }
}

// Set a receive callback.
bool Subscriber::SetReceiveCallback(ReceiveCallbackDelegate^ callback)
{
  m_receiveCallback = callback;
  auto nativeCallback = CreateNativeReceiveCallback(callback);
  return m_native_subscriber->SetReceiveCallback(nativeCallback);
}

// Remove the receive callback.
bool Subscriber::RemoveReceiveCallback()
{
  m_receiveCallback = nullptr;
  return m_native_subscriber->RemoveReceiveCallback();
}

// Get the number of publishers.
int Subscriber::GetPublisherCount()
{
  return static_cast<int>(m_native_subscriber->GetPublisherCount());
}

// Get the topic name.
String^ Subscriber::GetTopicName()
{
  std::string nativeTopic = m_native_subscriber->GetTopicName();
  return StlStringToString(nativeTopic);
}

// Get the topic ID.
TopicId^ Subscriber::GetTopicId()
{
  ::eCAL::STopicId nativeId = m_native_subscriber->GetTopicId();
  return ConvertTopicId(nativeId);
}

// Get the data type information.
DataTypeInformation^ Subscriber::GetDataTypeInformation()
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo = m_native_subscriber->GetDataTypeInformation();
  return gcnew DataTypeInformation(
    StlStringToString(nativeDataTypeInfo.name),
    StlStringToString(nativeDataTypeInfo.encoding),
    StlStringToByteArray(nativeDataTypeInfo.descriptor)
  );
}
