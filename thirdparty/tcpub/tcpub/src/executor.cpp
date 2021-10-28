// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include <tcpub/executor.h>

#include "executor_impl.h"

namespace tcpub
{
  Executor::Executor(size_t thread_count, const tcpub::logger::logger_t & log_function)
    : executor_impl_(std::make_shared<Executor_Impl>(log_function))
  {
    executor_impl_->start(thread_count);
  }

  Executor::~Executor()
  {
    executor_impl_->stop();
  }
}