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

#pragma once

#include <memory>
#include <functional>

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4834)
#endif

#include <asio.hpp>
#include <utility>

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include "ecal/cimpl/ecal_callback_cimpl.h"

namespace eCAL
{

  class CAsioServerSession
  {
  /////////////////////////////////////
  // Custom types for API
  /////////////////////////////////////
  public:
    using RequestCallbackT = std::function<int (const std::string &, std::string &)>;
    using EventCallbackT   = std::function<void (eCAL_Server_Event, const std::string &)>;

  /////////////////////////////////////
  // Constructor, Destructor, Create
  /////////////////////////////////////
  public:
    virtual ~CAsioServerSession() = default;

  protected:
    CAsioServerSession(asio::io_context& io_context_, const RequestCallbackT& request_callback, const EventCallbackT& event_callback)
      : socket_          (io_context_)
      , request_callback_(request_callback)
      , event_callback_  (event_callback)
    {}


  /////////////////////////////////////
  // Public API
  /////////////////////////////////////
  public:
    asio::ip::tcp::socket& socket() { return socket_; }
    virtual void start() = 0;

  /////////////////////////////////////
  // Member variables
  /////////////////////////////////////
  protected:
    asio::ip::tcp::socket socket_;

    const RequestCallbackT request_callback_;
    const EventCallbackT   event_callback_;
  };

} // namespace eCAL