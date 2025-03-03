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
 * @file  ecal_clr_publisher.cpp
**/

#include "ecal_clr_publisher.h"
#include "ecal_clr_common.h"
#include "ecal_clr_datatype.h"

#include <sstream>

using namespace Continental::eCAL::Core;
using namespace System::Runtime::InteropServices;
using namespace Internal;

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

bool Publisher::Send(String^ payload)
{
  return Send(payload, DEFAULT_TIME_ARGUMENT);
}

bool Publisher::Send(String^ payload, long long time)
{
  std::string nativePayload = StringToStlString(payload);
  return m_native_publisher->Send(nativePayload, time);
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
  // Extract the unique id from the native SEntityId and convert the topic name.
  unsigned __int64 entityId = nativeTopicId.topic_id.entity_id;
  String^ topicName = StlStringToString(nativeTopicId.topic_name);
  return gcnew TopicId(entityId, topicName);
}

DataTypeInformation^ Publisher::GetDataTypeInformation()
{
  ::eCAL::SDataTypeInformation nativeDataTypeInfo = m_native_publisher->GetDataTypeInformation();
  return gcnew DataTypeInformation(
    StlStringToString(nativeDataTypeInfo.name),
    StlStringToString(nativeDataTypeInfo.encoding),
    StlStringToByteArray(nativeDataTypeInfo.descriptor));
}
