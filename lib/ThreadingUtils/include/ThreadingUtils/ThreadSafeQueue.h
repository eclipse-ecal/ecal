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

#include <queue>
#include <mutex>
#include <future>

template <class T>
class ThreadSafeQueue
{
private:
  std::queue<T> mData;
  mutable std::mutex mMutex;
  std::condition_variable mEmptyCondition;

public:

  size_t size()
  {
    std::lock_guard<std::mutex> lock(mMutex);
    return(mData.size());
  }

  void push(T pValue)
  {
    std::lock_guard<std::mutex> lock(mMutex); 
    mData.push(pValue); 
    mEmptyCondition.notify_one(); 
  }

  std::shared_ptr<T> pop()
  {
    std::unique_lock<std::mutex> lock(mMutex); 
    mEmptyCondition.wait(lock, [this]{ return !mData.empty();});

    std::shared_ptr<T> ret = std::shared_ptr<T>(std::make_shared<T>(mData.front()));
    mData.pop();

    return ret;
  }

  std::shared_ptr<T> try_pop()
  {
    std::lock_guard<std::mutex> lock(mMutex); 
    if(mData.empty())
      return std::shared_ptr<T>();

    std::shared_ptr<T> ret = std::shared_ptr<T>(std::make_shared<T>(mData.front()));
    mData.pop();
    return ret;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock(mMutex); 
    return mData.empty();
  }
};
