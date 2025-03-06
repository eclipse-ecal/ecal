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

#include "ecal_clr_publisher.h"

#include "ecal_clr_common.h"
#include "ecal_clr_types.h"

#include <sstream>
#include <vcclr.h>
#include <functional>

using namespace Continental::eCAL::Core;
using namespace System::Runtime::InteropServices;
using namespace Internal;

using namespace msclr::interop;

namespace
{
  /**
   * @brief Helper function to create a native publisher event callback from a managed PublisherEventCallbackDelegate.
   *
   * @param callback The managed publisher event callback delegate.
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

// Constructor using only a topic name.
Publisher::Publisher(String^ topicName)
{
  m_native_publisher = new ::eCAL::CPublisher(StringToStlString(topicName));
}

// Constructor with topic name and data type information.
Publisher::Publisher(String^ topicName, DataTypeInformation^ dataTypeInfo)
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo;
  nativeDataTypeInfo.name       = StringToStlString(dataTypeInfo->Name);
  nativeDataTypeInfo.encoding   = StringToStlString(dataTypeInfo->Encoding);
  nativeDataTypeInfo.descriptor = ByteArrayToStlString(dataTypeInfo->Descriptor);
  m_native_publisher = new ::eCAL::CPublisher(StringToStlString(topicName), nativeDataTypeInfo);
}

// Constructor with topic name, data type information, and an optional event callback.
Publisher::Publisher(String^ topicName, DataTypeInformation^ dataTypeInfo, PublisherEventCallbackDelegate^ eventCallback)
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo;
  nativeDataTypeInfo.name = StringToStlString(dataTypeInfo->Name);
  nativeDataTypeInfo.encoding = StringToStlString(dataTypeInfo->Encoding);
  nativeDataTypeInfo.descriptor = ByteArrayToStlString(dataTypeInfo->Descriptor);

  std::string nativeTopic = StringToStlString(topicName);

  if (eventCallback != nullptr)
  {
    // Create a gcroot to safely capture the managed event callback.
    gcroot<PublisherEventCallbackDelegate^> managedCallback(eventCallback);
    // Use the helper to create a native callback.
    auto nativeCallback = CreatePublisherEventCallback(managedCallback);
    // Use the native callback constructor.
    m_native_publisher = new ::eCAL::CPublisher(nativeTopic, nativeDataTypeInfo, nativeCallback);
  }
  else
  {
    // If no event callback is provided, use the basic constructor.
    m_native_publisher = new ::eCAL::CPublisher(nativeTopic, nativeDataTypeInfo);
  }
}

Publisher::~Publisher()
{
  this->!Publisher();
}

Publisher::!Publisher()
{
  if (m_native_publisher != nullptr)
  {
    delete m_native_publisher;
    m_native_publisher = nullptr;
  }
}

bool Publisher::Send(array<Byte>^ data)
{
  return Send(data, DEFAULT_TIME_ARGUMENT);
}

bool Publisher::Send(array<Byte>^ data, long long time)
{
  if (data == nullptr || data->Length == 0)
    return false;
  pin_ptr<Byte> pinnedData = &data[0];
  return m_native_publisher->Send(static_cast<const void*>(pinnedData), data->Length, time);
}

int Publisher::GetSubscriberCount()
{
  return static_cast<int>(m_native_publisher->GetSubscriberCount());
}

String^ Publisher::GetTopicName()
{
  std::string nativeTopic = m_native_publisher->GetTopicName();
  return StlStringToString(nativeTopic);
}

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

DataTypeInformation^ Publisher::GetDataTypeInformation()
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo = m_native_publisher->GetDataTypeInformation();
  return gcnew DataTypeInformation(
    StlStringToString(nativeDataTypeInfo.name),
    StlStringToString(nativeDataTypeInfo.encoding),
    StlStringToByteArray(nativeDataTypeInfo.descriptor)
  );
}
