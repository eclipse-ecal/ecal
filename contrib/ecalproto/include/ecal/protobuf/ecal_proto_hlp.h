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
 * @brief  protobuf message description handling
**/

#pragma once

#include <algorithm>
#include <string>
#include <vector>

// protobuf includes
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <google/protobuf/descriptor.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace eCAL
{
namespace protobuf
{
  inline std::string CreateCompleteMessageName(const std::string& name_, const std::string& prefix_)
  {
    std::string message_name;
    if (!prefix_.empty())
    {
      message_name += prefix_;
      message_name += '.';
    }
    message_name += name_;
    return message_name;
  }

  inline std::string ExtractTopicName(const std::string& element_)
  {
    return element_.substr(0, element_.find('.'));
  }

  inline bool HasFile(const google::protobuf::FileDescriptorSet& fset_, const std::string& fname_)
  {
    for (auto findex = 0; findex < fset_.file_size(); ++findex)
    {
      if (fset_.file(findex).name() == fname_)
      {
        return(true);
      }
    }
    return(false);
  }

  inline bool GetFileDescriptor(const google::protobuf::Descriptor* desc_, google::protobuf::FileDescriptorSet& fset_)
  {
    if (desc_ == nullptr) return(false);
    const google::protobuf::FileDescriptor* fdesc = desc_->file();

    for (auto dep = 0; dep < fdesc->dependency_count(); ++dep)
    {
      // iterate containing messages
      const google::protobuf::FileDescriptor* sfdesc = fdesc->dependency(dep);
      for (auto mtype = 0; mtype < sfdesc->message_type_count(); ++mtype)
      {
        const google::protobuf::Descriptor* desc = sfdesc->message_type(mtype);
        GetFileDescriptor(desc, fset_);
      }

      // containing enums ?
      if (sfdesc->enum_type_count() > 0)
      {
        const google::protobuf::EnumDescriptor* edesc = sfdesc->enum_type(0);
        const google::protobuf::FileDescriptor* efdesc = edesc->file();

        if (!HasFile(fset_, efdesc->name()))
        {
          google::protobuf::FileDescriptorProto* epdesc = fset_.add_file();
          efdesc->CopyTo(epdesc);
        }
      }

      // containing services ?
      if (sfdesc->service_count() > 0)
      {
        const google::protobuf::ServiceDescriptor* svdesc = sfdesc->service(0);
        const google::protobuf::FileDescriptor*    svfdesc = svdesc->file();

        if (!HasFile(fset_, svfdesc->name()))
        {
          google::protobuf::FileDescriptorProto* svpdesc = fset_.add_file();
          svfdesc->CopyTo(svpdesc);
        }
      }
    }

    if (HasFile(fset_, fdesc->name())) return(true);

    google::protobuf::FileDescriptorProto* pdesc = fset_.add_file();
    fdesc->CopyTo(pdesc);
    for (auto field = 0; field < desc_->field_count(); ++field)
    {
      const google::protobuf::FieldDescriptor* fddesc = desc_->field(field);
      const google::protobuf::Descriptor*      desc = fddesc->message_type();
      GetFileDescriptor(desc, fset_);
    }

    return(true);
  }

  inline std::string GetProtoMessageDescription(const google::protobuf::Message& msg_)
  {
    const google::protobuf::Descriptor* desc = msg_.GetDescriptor();
    google::protobuf::FileDescriptorSet pset;
    if (GetFileDescriptor(desc, pset))
    {
      std::string desc_s = pset.SerializeAsString();
      return(desc_s);
    }
    return("");
  }

  inline std::string GetProtoMessageTypeName(const google::protobuf::Message& msg_)
  {
    return(std::string("proto:") + msg_.GetTypeName());
  }

  inline std::vector<std::string> GetProtoMessageFieldNames(const std::vector<const google::protobuf::FieldDescriptor*>& fields_)
  {
    std::vector<std::string> names_vec;
    std::transform(fields_.begin(), fields_.end(), std::back_inserter(names_vec), 
      [](const google::protobuf::FieldDescriptor* field) 
    {
      return field->name();
    }
    );
    return names_vec;
  }

  inline std::vector<std::string> GetProtoMessageFieldNames(const google::protobuf::Descriptor* descriptor)
  {
    std::vector<std::string> names_vec;
    auto count = descriptor->field_count();
    for (int i = 0; i < count; ++i)
    {
      auto field = descriptor->field(i);
      names_vec.push_back(field->name());
    }
    return names_vec;
  }
}
}
