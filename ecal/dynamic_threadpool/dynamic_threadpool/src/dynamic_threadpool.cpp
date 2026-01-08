/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

#include <dynamic_threadpool/dynamic_threadpool.h>
#include "dynamic_threadpool_impl.h"

#include <cstddef>
#include <functional>
#include <memory>


DynamicThreadPool::DynamicThreadPool()
  : impl(std::make_unique<DynamicThreadPoolImpl>())
{}

DynamicThreadPool::DynamicThreadPool(unsigned int max_size_)
  : impl(std::make_unique<DynamicThreadPoolImpl>(max_size_))
{}

DynamicThreadPool::~DynamicThreadPool() = default;

bool DynamicThreadPool::Post(const std::function<void()>& task_)
{
  return impl->Post(task_);
}

void DynamicThreadPool::Shutdown()
{
  impl->Shutdown();
}

void DynamicThreadPool::Join()
{
  impl->Join();
}

size_t DynamicThreadPool::GetSize() const
{
  return impl->GetSize();
}

size_t DynamicThreadPool::GetIdleCount() const
{
  return impl->GetIdleCount();
}
