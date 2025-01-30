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

#include "service_generate.h"
#include "common_generate.h"
#include <serialization/ecal_struct_service.h>

#include <cstdlib>

namespace eCAL
{
  namespace Service
  {
    // generate ServiceHeader
    ServiceHeader GenerateServiceHeader()
    {
      ServiceHeader header;
      header.host_name    = GenerateString(8);
      header.service_name = GenerateString(8);
      header.service_id   = GenerateString(5);
      header.method_name  = GenerateString(8);
      header.error        = GenerateString(15);
      header.id           = rand() % 100;
      header.state        = static_cast<eMethodCallState>(rand() % 3);

      return header;
    }

    // generate Request
    Request GenerateRequest()
    {
      Request request;
      request.header  = GenerateServiceHeader();
      request.request = GenerateString(20);

      return request;
    }

    // generate Response
    Response GenerateResponse()
    {
      Response response;
      response.header    = GenerateServiceHeader();
      response.response  = GenerateString(20);
      response.ret_state = rand() % 100;

      return response;
    }
  }
}
