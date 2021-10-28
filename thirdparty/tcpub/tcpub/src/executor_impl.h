// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <stdint.h>
#include <thread>
#include <string>
#include <vector>
#include <memory>

#include <asio.hpp>

#include "tcpub_logger_abstraction.h"

namespace tcpub
{
  class Executor_Impl : public std::enable_shared_from_this<Executor_Impl>
  {
  public:
    Executor_Impl(const logger::logger_t& log_function);
    ~Executor_Impl();

    // Copy
    Executor_Impl(const Executor_Impl&)            = delete;
    Executor_Impl& operator=(const Executor_Impl&) = delete;

    // Move
    Executor_Impl& operator=(Executor_Impl&&)      = delete;
    Executor_Impl(Executor_Impl&&)                 = delete;

  public:
    void start(size_t thread_count);
    void stop();

    std::shared_ptr<asio::io_service> ioService()   const;
    logger::logger_t                  logFunction() const;



  /////////////////////////////////////////
  // Member variables
  ////////////////////////////////////////

  private:
    const logger::logger_t                  log_;             /// Logger
    std::shared_ptr<asio::io_service>       io_service_;      /// global io service

    std::vector<std::thread>                thread_pool_;     /// Asio threadpool executing the io servic
    std::shared_ptr<asio::io_service::work> dummy_work_;      /// Dummy work, so the io_service will never run out of work and shut down, even if there is no publisher or subscriber at the moment
  };
}
