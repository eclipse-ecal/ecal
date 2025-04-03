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

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/dynamic_subscriber.h>

#include <iostream>

const std::string MESSAGE_NAME("person");

void ProcValue(const std::string& group_, const std::string& name_, const double value_, size_t index_)
{
  std::string var_name;
  if(!group_.empty()) var_name += group_ + ".";
  var_name += name_;
  if(index_ > 0) var_name += "[" + std::to_string(index_) + "]";
  std::cout << var_name << " : " << value_ << std::endl;
}

void ProcString(const std::string& group_, const std::string& name_, const std::string& value_, size_t index_)
{
  std::string var_name;
  if(!group_.empty()) var_name += group_ + ".";
  var_name += name_;
  if(index_ > 0) var_name += "[" + std::to_string(index_) + "]";
  std::cout << var_name << " : " << value_ << std::endl;
}

void ProcProtoType(const std::string& group_, const std::string& name_, google::protobuf::int32 value_, size_t index_)
{
  ProcValue(group_, name_, double(value_), index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, google::protobuf::int64 value_, size_t index_)
{
  ProcValue(group_, name_, double(value_), index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, google::protobuf::uint32 value_, size_t index_)
{
  ProcValue(group_, name_, double(value_), index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, google::protobuf::uint64 value_, size_t index_)
{
  ProcValue(group_, name_, double(value_), index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, float value_, size_t index_)
{
  ProcValue(group_, name_, double(value_), index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, double value_, size_t index_)
{
  ProcValue(group_, name_, double(value_), index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, bool value_, size_t index_)
{
  ProcValue(group_, name_, double(value_), index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, const std::string& value_, size_t index_)
{
  ProcString(group_, name_, value_, index_);
}

void ProcProtoType(const std::string& group_, const std::string& name_, const google::protobuf::EnumValueDescriptor* value_, size_t index_)
{
  ProcValue(group_, name_, double(value_->number()), index_);
}

void ProcProtoMsg(const google::protobuf::Message& msg_, const std::string& prefix_ /* = "" */)
{
  int count = msg_.GetDescriptor()->field_count();
  const google::protobuf::Reflection* ref_ptr = msg_.GetReflection();

  if(ref_ptr)
  {
    for (int i = 0; i < count; ++i)
    {
      auto field = msg_.GetDescriptor()->field(i);
  
      const google::protobuf::FieldDescriptor::CppType fdt = field->cpp_type();
      switch(fdt)
      {
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32:      // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedInt32(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetInt32(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_INT64:      // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedInt64(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetInt64(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:     // TYPE_UINT32, TYPE_FIXED32
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedUInt32(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetUInt32(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:     // TYPE_UINT64, TYPE_FIXED64
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedUInt64(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetUInt64(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:     // TYPE_DOUBLE
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedDouble(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetDouble(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:      // TYPE_FLOAT
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedFloat(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetFloat(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:       // TYPE_BOOL
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedBool(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetBool(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:       // TYPE_ENUM
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedEnum(msg_, field, fnum), static_cast<size_t>(fnum));
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetEnum(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_STRING:     // TYPE_STRING, TYPE_BYTES
        if(field->is_repeated())
        {
          int fsize = ref_ptr->FieldSize(msg_, field);
          for(int fnum = 0; fnum < fsize; ++fnum)
          {
            ProcProtoType(prefix_, field->name(), ref_ptr->GetRepeatedString(msg_, field, fnum), fnum);
          }
        }
        else
        {
          ProcProtoType(prefix_, field->name(), ref_ptr->GetString(msg_, field), 0);
        }
        break;
      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:    // TYPE_MESSAGE, TYPE_GROUP
        {
          if(field->is_repeated())
          {
            int fsize = ref_ptr->FieldSize(msg_, field);
            for(int fnum = 0; fnum < fsize; ++fnum)
            {
              const google::protobuf::Message& msg = ref_ptr->GetRepeatedMessage(msg_, field, fnum);
              std::string prefix = field->name();
              prefix += "[";
              prefix += std::to_string(fnum);
              prefix += "]";
              if(!prefix_.empty()) prefix = prefix_ + "." + prefix;

              // do not process default messages to avoid infinite recursions.
              std::vector<const google::protobuf::FieldDescriptor*> msg_fields;
              msg.GetReflection()->ListFields(msg, &msg_fields);
              
              if (prefix_.find(field->name()) == std::string::npos || !msg_fields.empty())
                ProcProtoMsg(msg, prefix);
            }
          }
          else
          {
            const google::protobuf::Message& msg = ref_ptr->GetMessage(msg_, field);
            std::string prefix = field->name();
            if(!prefix_.empty()) prefix = prefix_ + "." + prefix;

            // do not process default messages to avoid infinite recursions.
            std::vector<const google::protobuf::FieldDescriptor*> msg_fields;
            msg.GetReflection()->ListFields(msg, &msg_fields);

            if (prefix_.find(field->name()) == std::string::npos || !msg_fields.empty())
              ProcProtoMsg(msg, prefix);
          }
        }
        break;
      default:
        break;
      }
    }
  }
}

void ProtoMsgCallback(const eCAL::STopicId& topic_id_, const std::shared_ptr<google::protobuf::Message>& msg_)
{
  ProcProtoMsg(*msg_, topic_id_.topic_name);
  std::cout << std::endl;
}

int main()
{
  // initialize eCAL API
  eCAL::Initialize("proto_dyn");

  // create dynamic subscribers for receiving and decoding messages
  eCAL::protobuf::CDynamicSubscriber sub(MESSAGE_NAME);
  sub.SetReceiveCallback(std::bind(ProtoMsgCallback, std::placeholders::_1, std::placeholders::_2));

  // enter main loop
  while(eCAL::Ok())
  {
    // sleep main thread for 1 second
    eCAL::Process::SleepMS(1000);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
