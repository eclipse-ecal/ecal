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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecal/cimpl/ecal_callback_cimpl.h"
#include "ecal_service_error.h"

namespace eCAL
{
  namespace service
  {
    class ClientSession
    {
    /////////////////////////////////////
    // Custom types for API
    /////////////////////////////////////
    public:
      using EventCallbackT    = std::function<void (eCAL_Client_Event, const std::string &)>;  // TODO: Ask Rex what the "eCAL Client Event timeout" is. Should it maybe be fired, when the connection establishement failed? Currently, in that case no event whatsoever is fired, as there is no "Connection failed" event.
      using ResponseCallbackT = std::function<void (const eCAL::service::Error&, const std::shared_ptr<std::string>&)>;

    /////////////////////////////////////
    // Constructor, Destructor, Create
    /////////////////////////////////////
    protected:
      ClientSession(asio::io_context& io_context_, const EventCallbackT& event_callback)
        : io_context_    (io_context_)
        , socket_        (io_context_)
        , event_callback_(event_callback)
      {}

    public:
      virtual ~ClientSession() = default;

    /////////////////////////////////////
    // API
    /////////////////////////////////////
    public:
      virtual void async_call_service(const std::shared_ptr<std::string>& request, const ResponseCallbackT& response_callback) = 0;

      // TODO: Write tests that test whether this function behaves correctly in error cases
      inline eCAL::service::Error call_service(const std::shared_ptr<std::string>& request, std::shared_ptr<std::string>& response)
      {
        eCAL::service::Error error(Error::GENERIC_ERROR);
        
        // Create a condition variable and a mutex to wait for the response
        std::mutex              mutex;
        std::condition_variable condition_variable;

        // Create a response callback, that will set the response and notify the condition variable
        ResponseCallbackT response_callback
                  = [&error, &response, &mutex, &condition_variable]
                    (const eCAL::service::Error& response_error, const std::shared_ptr<std::string>& response_)
                    {
                      response = response_;
                      error    = response_error;

                      {
                        std::lock_guard<std::mutex> lock(mutex);
                        condition_variable.notify_all();
                      }
                    };

        // Lock mutex, call service asynchronously and wait for the condition variable to be notified
        {
          std::unique_lock<std::mutex> lock(mutex);
          async_call_service(request, response_callback); // TODO: If I decide to directly call the callback from the same thread in error cases, this mutex will cause undefined behavior. I can work around that with more code (adding a boolean and a predicate telling me, whether the condition variable has already been notified and calling the service without having the mutex locked)
          condition_variable.wait(lock);
        }

        return error;
      }

    /////////////////////////////////////
    // Member variables
    /////////////////////////////////////
    protected:
      asio::io_context&     io_context_;
      asio::ip::tcp::socket socket_;
      const EventCallbackT  event_callback_;
    };

  } // namespace service
} // namespace eCAL
