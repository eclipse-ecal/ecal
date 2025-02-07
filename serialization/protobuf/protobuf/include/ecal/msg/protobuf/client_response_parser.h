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
 * @file   client_response_parser.h
 * @brief  eCAL client response message parser
**/

#pragma once

#include <ecal/service/types.h>
#include <ecal/msg/protobuf/client_protobuf_types.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <memory>
#include <string>

namespace eCAL
{
  namespace protobuf
  {
    namespace ResponseParser
    {
      /**
       * @brief Parses a generic SServiceResponse into a typed TMsgServiceResponse<ResponseT>.
       *
       * This function creates a new mutable message instance from the response prototype,
       * parses the raw response string, and attempts to convert the generic message to the expected type.
       *
       * @tparam ResponseT             Expected protobuf response type.
       * 
       * @param  service_descriptor_   The service descriptor for the service containing the method.
       * @param  method_name_          Name of the service method (used for error messages).
       * @param  generic_response_     The generic service response containing the raw response string.
       * @param  typed_response_ (out) The typed response structure to fill.
       * 
       * @return  True if parsing and conversion succeeded; false otherwise.
       */
      template <typename ResponseT>
      bool ParseResponse(const google::protobuf::ServiceDescriptor* service_descriptor_,
                         const std::string& method_name_,
                         const SServiceResponse& generic_response_,
                         TMsgServiceResponse<ResponseT>& typed_response_)
      {
        // Check service descriptor.
        if (service_descriptor_ == nullptr)
        {
          typed_response_.error_msg = "Failed to get service descriptor";
          return false;
        }

        // Find the method descriptor.
        const google::protobuf::MethodDescriptor* method_descriptor = service_descriptor_->FindMethodByName(method_name_);
        if (method_descriptor == nullptr)
        {
          typed_response_.error_msg = "Method not found in service descriptor";
          return false;
        }

        // Get the prototype for the response.
        const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(method_descriptor->output_type());
        if (prototype == nullptr)
        {
          typed_response_.error_msg = "Failed to get prototype for response";
          return false;
        }

        // Create a new mutable message instance.
        std::unique_ptr<google::protobuf::Message> generic_msg(prototype->New());
        if ((generic_msg == nullptr) || !generic_msg->ParseFromString(generic_response_.response))
        {
          typed_response_.error_msg = "Failed to parse response";
          return false;
        }

        // Attempt to convert to the expected type.
        ResponseT* typed_ptr = dynamic_cast<ResponseT*>(generic_msg.get());
        if (typed_ptr == nullptr)
        {
          typed_response_.error_msg = "Response type conversion failed";
          return false;
        }

        // Transfer ownership to the typed response.
        generic_msg.release();
        typed_response_.response.reset(typed_ptr);

        // Fill the typed response attributes.
        typed_response_.server_id                  = generic_response_.server_id;
        typed_response_.service_method_information = generic_response_.service_method_information;
        typed_response_.call_state                 = generic_response_.call_state;
        typed_response_.ret_state                  = generic_response_.ret_state;
        typed_response_.error_msg                  = generic_response_.error_msg;
        return true;
      }

    } // namespace ResponseParser
  } // namespace protobuf
} // namespace eCAL
