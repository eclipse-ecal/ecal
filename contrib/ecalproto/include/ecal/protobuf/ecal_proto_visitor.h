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

#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4244 4267 4512 4996 4146 4800) // disable proto warnings
#endif
#include <google/protobuf/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

// disable the unreferenced formal parameter warning, because these are prototype functions with an empty body
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning( disable : 4100 )
#endif // _MSC_VER

namespace eCAL
{
  namespace protobuf
  {
    class MessageFilter;

    struct MessageInfo
    {
      std::string field_name;
      std::string group_name;
      int id;
      std::string type;
    };

    class MessageVisitor
    {
      friend class CProtoDecoder;
    public:
      virtual ~MessageVisitor() {};

    protected:
      virtual void ArrayStart(const MessageInfo& /*info*/, const google::protobuf::FieldDescriptor::Type& /*type_*/, size_t /*size_*/) {};
      virtual void ArrayValueIntegral(const MessageInfo& /*info_*/, google::protobuf::int32  /*value_*/) {};
      virtual void ArrayValueIntegral(const MessageInfo& /*info_*/, google::protobuf::uint32 /*value_*/) {};
      virtual void ArrayValueIntegral(const MessageInfo& /*info_*/, google::protobuf::int64  /*value_*/) {};
      virtual void ArrayValueIntegral(const MessageInfo& /*info_*/, google::protobuf::uint64 /*value_*/) {};
      virtual void ArrayValueIntegral(const MessageInfo& /*info_*/, float                    /*value_*/) {};
      virtual void ArrayValueIntegral(const MessageInfo& /*info_*/, double                   /*value_*/) {};
      virtual void ArrayValueIntegral(const MessageInfo& /*info_*/, bool                     /*value_*/) {};
      virtual void ArrayValueString  (const MessageInfo& /*info_*/, const std::string&       /*value_*/) {};
      virtual void ArrayValueBytes   (const MessageInfo& /*info_*/, const std::string&       /*value_*/) {};
      virtual void ArrayValueEnum    (const MessageInfo& /*info_*/, int                      /*value_*/, const std::string& /*name_*/) {};
      virtual void ArrayEnd() {};

      virtual void ScalarValueIntegral(const MessageInfo& /*info_*/, google::protobuf::int32  /*value_*/) {};
      virtual void ScalarValueIntegral(const MessageInfo& /*info_*/, google::protobuf::uint32 /*value_*/) {};
      virtual void ScalarValueIntegral(const MessageInfo& /*info_*/, google::protobuf::int64  /*value_*/) {};
      virtual void ScalarValueIntegral(const MessageInfo& /*info_*/, google::protobuf::uint64 /*value_*/) {};
      virtual void ScalarValueIntegral(const MessageInfo& /*info_*/, float                    /*value_*/) {};
      virtual void ScalarValueIntegral(const MessageInfo& /*info_*/, double                   /*value_*/) {};
      virtual void ScalarValueIntegral(const MessageInfo& /*info_*/, bool                     /*value_*/) {};
      virtual void ScalarValueString  (const MessageInfo& /*info_*/, const std::string&       /*value_*/) {};
      virtual void ScalarValueBytes   (const MessageInfo& /*info_*/, const std::string&       /*value_*/) {};
      virtual void ScalarValueEnum    (const MessageInfo& /*info_*/, int                      /*value_*/, const std::string& /*name*/) {};

      virtual void ScalarMessageStart(const MessageInfo& /*info*/, const std::vector<std::string>& /*fields_*/) {};
      virtual void ScalarMessageEnd() {};
      virtual void ArrayMessageStart(const MessageInfo& /*info*/, const std::vector<std::string>& /*fields_*/){};
      virtual void ArrayMessageEnd(){};

      virtual bool AcceptMessage(const std::string& /*name*/) { return true; };
    };


    // This is a convenience class
    // You can inherit from this class if you'd like to only use double values, e.g. saves you the hassle of implementing
    // all other array / scalar ValueIntegral functions
    class MessageVisitorDoubleIntegral : public MessageVisitor
    {
      friend class CProtoDecoder;
    protected:
      using MessageVisitor::ArrayValueIntegral;
      using MessageVisitor::ScalarValueIntegral;

      void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::int32  value_) override
      {
        ArrayValueIntegral(info_, static_cast<double>(value_));
      };
      void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::uint32 value_) override
      {
        ArrayValueIntegral(info_, static_cast<double>(value_));
      }
      void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::int64  value_) override
      {
        ArrayValueIntegral(info_, static_cast<double>(value_));
      };
      void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::uint64 value_) override
      {
        ArrayValueIntegral(info_, static_cast<double>(value_));
      };
      void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, float                    value_) override
      {
        ArrayValueIntegral(info_, static_cast<double>(value_));
      };
      void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, bool                     value_) override
      {
        ArrayValueIntegral(info_, static_cast<double>(value_));
      };

      void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::int32  value_) override
      {
        ScalarValueIntegral(info_, static_cast<double>(value_));
      };
      void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::uint32 value_) override
      {
        ScalarValueIntegral(info_, static_cast<double>(value_));
      };
      void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::int64  value_) override
      {
        ScalarValueIntegral(info_, static_cast<double>(value_));
      };
      void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::uint64 value_) override
      {
        ScalarValueIntegral(info_, static_cast<double>(value_));
      };
      void ScalarValueIntegral(const MessageInfo& info_, float                    value_) override
      {
        ScalarValueIntegral(info_, static_cast<double>(value_));
      };
      void ScalarValueIntegral(const MessageInfo& info_, bool                     value_) override
      {
        ScalarValueIntegral(info_, static_cast<double>(value_));
      };
    };
  }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER