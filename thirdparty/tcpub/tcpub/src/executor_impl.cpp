// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include "executor_impl.h"

namespace tcpub
{
  Executor_Impl::Executor_Impl(const logger::logger_t& log_function)
    : log_(log_function)
    , io_service_(std::make_shared<asio::io_service>())
    , dummy_work_(std::make_shared<asio::io_service::work>(*io_service_))
  {
#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Executor: Creating Executor.");
#endif
  }

  Executor_Impl::~Executor_Impl()
  {
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id = ss.str();
    log_(logger::LogLevel::DebugVerbose, "Executor: Deleting from thread " + thread_id + "...");
#endif

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Executor: Waiting for IoService threads to shut down...");
#endif

    // Detach all threads and clear the thread pool
    // The threads hold a shared_ptr to this object and therefore manage their
    // own lifecycle. So when the threads terminate, the last destructor 
    // detaches the terminating threads.
    for (std::thread& thread : thread_pool_)
    {
      thread.detach();
    }
    thread_pool_.clear();

#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Executor: All IoService threads have shut down successfully.");
#endif


#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Executor: Deleted.");
#endif
  }

  void Executor_Impl::start(size_t thread_count)
  {
#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Executor: Starting Executor with " + std::to_string(thread_count) + " threads.");
#endif
    for (size_t i = 0; i < thread_count; i++)
    {
      thread_pool_.emplace_back([me = shared_from_this()]()
                                {
#if (TCPUB_LOG_DEBUG_ENABLED)
                                  std::stringstream ss;
                                  ss << std::this_thread::get_id();
                                  std::string thread_id = ss.str();

                                  me->log_(logger::LogLevel::Debug, "Executor: IoService::Run() in thread " + thread_id);
#endif

                                  me->io_service_->run();

#if (TCPUB_LOG_DEBUG_ENABLED)
                                  me->log_(logger::LogLevel::Debug, "Executor: IoService: Shutdown of thread " + thread_id);
#endif
                                });
    }
  }

  void Executor_Impl::stop()
  {
#if (TCPUB_LOG_DEBUG_ENABLED)
    log_(logger::LogLevel::Debug, "Executor::stop()");
#endif

    // Delete the dummy work
    dummy_work_.reset();

    // Stop the IO Service
    io_service_->stop();
  }

  std::shared_ptr<asio::io_service> Executor_Impl::ioService() const
  {
    return io_service_;
  }

  logger::logger_t Executor_Impl::logFunction() const
  {
    return log_;
  }

}