/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2023 Continental Corporation
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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

template <typename T>
class atomic_signalable
{
public:
  atomic_signalable(T initial_value) : value(initial_value) {}

  atomic_signalable<T>& operator=(const T new_value)
  {
    std::lock_guard<std::mutex> lock(mutex);
    value = new_value;
    cv.notify_all();
    return *this;
  }

  T operator++()
  {
    std::lock_guard<std::mutex> lock(mutex);
    T newValue = ++value;
    cv.notify_all();
    return newValue;
  }

  T operator++(T) 
  {
    std::lock_guard<std::mutex> lock(mutex);
    T oldValue = value++;
    cv.notify_all();
    return oldValue;
  }

  T operator--()
  {
    std::lock_guard<std::mutex> lock(mutex);
    T newValue = --value;
    cv.notify_all();
    return newValue;
  }

  T operator--(T) 
  {
    std::lock_guard<std::mutex> lock(mutex);
    T oldValue = value--;
    cv.notify_all();
    return oldValue;
  }

  T operator+=(const T& other) 
  {
    std::lock_guard<std::mutex> lock(mutex);
    value += other;
    cv.notify_all();
    return value;
  }

  T operator-=(const T& other) 
  {
    std::lock_guard<std::mutex> lock(mutex);
    value -= other;
    cv.notify_all();
    return value;
  }

  T operator*=(const T& other) 
  {
    std::lock_guard<std::mutex> lock(mutex);
    value *= other;
    cv.notify_all();
    return value;
  }

  T operator/=(const T& other) 
  {
    std::lock_guard<std::mutex> lock(mutex);
    value /= other;
    cv.notify_all();
    return value;
  }

  T operator%=(const T& other)
  {
    std::lock_guard<std::mutex> lock(mutex);
    value %= other;
    cv.notify_all();
    return value;
  }

  template <typename Predicate>
  bool wait_for(Predicate predicate, std::chrono::milliseconds timeout)
  {
    std::unique_lock<std::mutex> lock(mutex);
    return cv.wait_for(lock, timeout, [&]() { return predicate(value); });
  }

  bool operator==(T other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value == other;
  }

  bool operator==(const atomic_signalable<T>& other) const
  {
    std::lock_guard<std::mutex> lock_this(mutex);
    std::lock_guard<std::mutex> lock_other(other.mutex);
    return value == other.value;
  }

  bool operator!=(T other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value != other;
  }

  bool operator<(T other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value < other;
  }

  bool operator<=(T other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value <= other;
  }

  bool operator>(T other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value > other;
  }

  bool operator>=(T other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value >= other;
  }

private:
  T value;
  std::condition_variable cv;
  mutable std::mutex mutex;
};


template <typename T>
bool operator==(const T& other, const atomic_signalable<T>& atomic)
{
  return atomic == other;
}

template <typename T>
bool operator!=(const T& other, const atomic_signalable<T>& atomic)
{
  return atomic != other;
}

template <typename T>
bool operator<(const T& other, const atomic_signalable<T>& atomic)
{
  return atomic > other;
}

template <typename T>
bool operator<=(const T& other, const atomic_signalable<T>& atomic)
{
  return atomic >= other;
}

template <typename T>
bool operator>(const T& other, const atomic_signalable<T>& atomic)
{
  return atomic < other;
}

template <typename T>
bool operator>=(const T& other, const atomic_signalable<T>& atomic)
{
  return atomic <= other;
}
