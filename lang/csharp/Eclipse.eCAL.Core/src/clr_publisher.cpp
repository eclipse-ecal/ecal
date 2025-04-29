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

#include <msclr/marshal_cppstd.h>

#include "pubsub/clr_publisher.h"

#include "clr_common.h"
#include "clr_types.h"

#include <sstream>
#include <vcclr.h>
#include <functional>

using namespace Eclipse::eCAL::Core;
using namespace System::Runtime::InteropServices;
using namespace Internal;

using namespace msclr::interop;

namespace
{
  /**
   * @brief Helper function to create a native publisher event callback from a managed PublisherEventCallbackDelegate.
   *
   * @param callback The managed publisher event callback delegate.
   * 
   * @return A std::function wrapping the managed callback.
   */
  std::function<void(const ::eCAL::STopicId&, const ::eCAL::SPubEventCallbackData&)>
    CreatePublisherEventCallback(gcroot<PublisherEventCallbackDelegate^> managedCallback)
  {
    return [managedCallback](const ::eCAL::STopicId& topicId, const ::eCAL::SPubEventCallbackData& data)
      {
        // Convert native topic id to managed TopicId.
        TopicId^ mTopicId = gcnew TopicId(
          gcnew EntityId(topicId.topic_id.entity_id,
            topicId.topic_id.process_id,
            StlStringToString(topicId.topic_id.host_name)),
          StlStringToString(topicId.topic_name)
        );
        // Convert native SPubEventCallbackData to managed PubEventCallbackData.
        PubEventCallbackData^ mData = gcnew PubEventCallbackData();
        mData->EventType = static_cast<PublisherEvent>(data.event_type);
        mData->EventTime = data.event_time;
        mData->SubscriberDataType = gcnew DataTypeInformation(
          StlStringToString(data.subscriber_datatype.name),
          StlStringToString(data.subscriber_datatype.encoding),
          StlStringToByteArray(data.subscriber_datatype.descriptor)
        );
        // Invoke the managed delegate.
        managedCallback->Invoke(mTopicId, mData);
      };
  }
}

// Constructor
Publisher::Publisher(String^ topicName, DataTypeInformation^ dataTypeInfo, PublisherEventCallbackDelegate^ eventCallback)
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
    gcroot<PublisherEventCallbackDelegate^> managedCallback(eventCallback);
    auto nativeCallback = CreatePublisherEventCallback(managedCallback);
    m_native_publisher = new ::eCAL::CPublisher(nativeTopic, nativeDataTypeInfo, nativeCallback);
  }
  else
  {
    m_native_publisher = new ::eCAL::CPublisher(nativeTopic, nativeDataTypeInfo);
  }
}

// Destructor
Publisher::~Publisher()
{
  this->!Publisher();
}

// Finalizer
Publisher::!Publisher()
{
  if (m_native_publisher != nullptr)
  {
    delete m_native_publisher;
    m_native_publisher = nullptr;
  }
}

// Send a message to all subscribers using a byte array payload (using eCAL time).
bool Publisher::Send(array<Byte>^ data)
{
  return Send(data, DEFAULT_TIME_ARGUMENT);
}

// Send a message to all subscribers using a byte array payload and a specific time.
bool Publisher::Send(array<Byte>^ data, long long time)
{
  if (data == nullptr || data->Length == 0)
    return false;
  pin_ptr<Byte> pinnedData = &data[0];
  return m_native_publisher->Send(static_cast<const void*>(pinnedData), data->Length, time);
}

// Get the number of subscribers.
int Publisher::GetSubscriberCount()
{
  return static_cast<int>(m_native_publisher->GetSubscriberCount());
}

// Get the topic name.
String^ Publisher::GetTopicName()
{
  std::string nativeTopic = m_native_publisher->GetTopicName();
  return StlStringToString(nativeTopic);
}

// Get the topic ID.
TopicId^ Publisher::GetTopicId()
{
  ::eCAL::STopicId nativeTopicId = m_native_publisher->GetTopicId();
  return gcnew TopicId(
    gcnew EntityId(
      nativeTopicId.topic_id.entity_id,
      nativeTopicId.topic_id.process_id,
      StlStringToString(nativeTopicId.topic_id.host_name)
    ),
    StlStringToString(nativeTopicId.topic_name)
  );
}

// Get the data type information.
DataTypeInformation^ Publisher::GetDataTypeInformation()
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo = m_native_publisher->GetDataTypeInformation();
  return gcnew DataTypeInformation(
    StlStringToString(nativeDataTypeInfo.name),
    StlStringToString(nativeDataTypeInfo.encoding),
    StlStringToByteArray(nativeDataTypeInfo.descriptor)
  );
}
