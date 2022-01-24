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
 * @brief  eCALMonitor proto message decoding class
**/

#include <ecal/protobuf/ecal_proto_decoder.h>

#include <ecal/protobuf/ecal_proto_hlp.h>
#include <ecal/protobuf/ecal_proto_message_filter.h>
#include <ecal/protobuf/ecal_proto_visitor.h>

#include <sstream>

namespace eCAL
{
namespace protobuf
{
  CProtoDecoder::CProtoDecoder()
    : visitor(std::make_shared<MessageVisitor>())
  {}

  bool CProtoDecoder::ProcProtoMsg(const google::protobuf::Message& msg_, const std::string& name_, const std::string& prefix_, bool is_Array_, size_t index_)
  {
    const google::protobuf::Reflection* ref_ptr = msg_.GetReflection();
    if (ref_ptr == nullptr) return false;

    auto descriptor = msg_.GetDescriptor();

    int count = descriptor->field_count();

    if (!is_Array_)
    {
      visitor->ScalarMessageStart({ name_, prefix_, (int)index_, msg_.GetTypeName()}, GetProtoMessageFieldNames(descriptor));
    }
    else
    {
      visitor->ArrayMessageStart({ name_, prefix_, (int)index_, msg_.GetTypeName()}, GetProtoMessageFieldNames(descriptor));
    }

    std::string complete_message_name = CreateCompleteMessageName(name_, prefix_);


    for (int i = 0; i < count; ++i)
    {
      auto field = descriptor->field(i);
      std::string child_message_name = CreateCompleteMessageName(field->name(), complete_message_name);

      if (visitor->AcceptMessage(child_message_name))
      {
        // only required in case of repeated fields
        bool accept_complete_array(false);
        int fsize = 0;

        if (field->is_repeated())
        {
          accept_complete_array = visitor->AcceptMessage(child_message_name + "[*]");
          fsize = ref_ptr->FieldSize(msg_, field);
          visitor->ArrayStart({ field->name(), complete_message_name, field->number(), field->type_name() }, field->type(), fsize);
        }

        const google::protobuf::FieldDescriptor::CppType fdt = field->cpp_type();
        switch (fdt)
        {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:      // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) +"]"))
              {
                info.id = fnum;
                visitor->ArrayValueIntegral(info, ref_ptr->GetRepeatedInt32(msg_, field, fnum));
              }
            }
          }
          else
          {
            visitor->ScalarValueIntegral({ field->name(), complete_message_name, field->number(), field->type_name() }, ref_ptr->GetInt32(msg_, field));
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:     // TYPE_UINT32, TYPE_FIXED32
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                visitor->ArrayValueIntegral(info, ref_ptr->GetRepeatedUInt32(msg_, field, fnum));
              }
            }
          }
          else
          {
            visitor->ScalarValueIntegral({field->name(), complete_message_name, field->number(), field->type_name()}, ref_ptr->GetUInt32(msg_, field));
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:      // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                visitor->ArrayValueIntegral(info, ref_ptr->GetRepeatedInt64(msg_, field, fnum));
              }
            }
          }
          else
          {
            visitor->ScalarValueIntegral({field->name(), complete_message_name, field->number(), field->type_name() }, ref_ptr->GetInt64(msg_, field));
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:     // TYPE_UINT64, TYPE_FIXED64
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                visitor->ArrayValueIntegral(info, ref_ptr->GetRepeatedUInt64(msg_, field, fnum));
              }
            }
          }
          else
          {
            visitor->ScalarValueIntegral({field->name(), complete_message_name, field->number(), field->type_name() }, ref_ptr->GetUInt64(msg_, field));
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:      // TYPE_FLOAT
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                visitor->ArrayValueIntegral(info, ref_ptr->GetRepeatedFloat(msg_, field, fnum));
              }
            }
          }
          else
          {
            visitor->ScalarValueIntegral({field->name(), complete_message_name, field->number(), field->type_name() }, ref_ptr->GetFloat(msg_, field));
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:     // TYPE_DOUBLE
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                visitor->ArrayValueIntegral(info, ref_ptr->GetRepeatedDouble(msg_, field, fnum));
              }
            }
          }
          else
          {
            visitor->ScalarValueIntegral({field->name(), complete_message_name, field->number(), field->type_name() }, ref_ptr->GetDouble(msg_, field));
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:       // TYPE_BOOL
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                visitor->ArrayValueIntegral(info, ref_ptr->GetRepeatedBool(msg_, field, fnum));
              }
            }
          }
          else
          {
            visitor->ScalarValueIntegral({field->name(), complete_message_name, field->number() , field->type_name()}, ref_ptr->GetBool(msg_, field));
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:       // TYPE_ENUM
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                visitor->ArrayValueEnum(info, ref_ptr->GetRepeatedEnum(msg_, field, fnum)->number(), ref_ptr->GetRepeatedEnum(msg_, field, fnum)->name());
              }
            }
          }
          else
          {
            visitor->ScalarValueEnum({field->name(), complete_message_name, field->number(), field->type_name() }, ref_ptr->GetEnum(msg_, field)->number(), ref_ptr->GetEnum(msg_, field)->name());
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:     // TYPE_STRING, TYPE_BYTES
          if (field->is_repeated())
          {
            MessageInfo info{field->name(), complete_message_name, 0, field->type_name()};
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                info.id = fnum;
                if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
                {
                    visitor->ArrayValueString(info, ref_ptr->GetRepeatedString(msg_, field, fnum));
                }
                else // TYPE_BYTES
                {
                  visitor->ArrayValueBytes(info, ref_ptr->GetRepeatedString(msg_, field, fnum));
                }
              }
            }
          }
          else
          {
            if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
            {
              visitor->ScalarValueString({field->name(), complete_message_name, field->number(), field->type_name()}, ref_ptr->GetString(msg_, field));
            }
            else // TYPE_BYTES
            {
              visitor->ScalarValueBytes({field->name(), complete_message_name, field->number(), field->type_name() }, ref_ptr->GetString(msg_, field));
            }
          }
          break;
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:    // TYPE_MESSAGE, TYPE_GROUP
        {
          if (field->is_repeated())
          {
            for (int fnum = 0; fnum < fsize; ++fnum)
            {
              if (accept_complete_array || visitor->AcceptMessage(child_message_name + "[" + std::to_string(fnum) + "]"))
              {
                const google::protobuf::Message& msg = ref_ptr->GetRepeatedMessage(msg_, field, fnum);
                std::stringstream name;
                name << field->name() << "[" << fnum << "]";

                // do not process default messages to avoid infinite recursions.
                std::vector<const google::protobuf::FieldDescriptor*> msg_fields;
                msg.GetReflection()->ListFields(msg, &msg_fields);
                if (msg_fields.size() > 0)
                {
                  ProcProtoMsg(msg, name.str(), complete_message_name, true, fnum);
                }
              }
            }
          }
          else
          {
            const google::protobuf::Message& msg = ref_ptr->GetMessage(msg_, field);

            // do not process default messages to avoid infinite recursions.
            std::vector<const google::protobuf::FieldDescriptor*> msg_fields;
            msg.GetReflection()->ListFields(msg, &msg_fields);
            if (msg_fields.size() > 0)
            {
              ProcProtoMsg(msg, field->name(), complete_message_name, false, field->number());
            }
          }
        }
        break;
        default:
          break;
        }

        if (field->is_repeated())
        {
          visitor->ArrayEnd();
        }
      }
    }
    if (!is_Array_)
    {
      visitor->ScalarMessageEnd();
    }
    else
    {
      visitor->ArrayMessageEnd();
    }
    return true;
  }

}
}
