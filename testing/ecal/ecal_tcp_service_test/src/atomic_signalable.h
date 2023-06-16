#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

template <typename T>
class atomic_signalable
{
public:
  atomic_signalable(T initial_value) : value(initial_value) {}

  T operator++()
  {
    std::lock_guard<std::mutex> lock(mutex);
    T newValue = ++value;
    cv.notify_all();
    return newValue;
  }

  T operator++(int) 
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

  T operator--(int) 
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

  bool operator==(const T& other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value == other;
  }

  bool operator!=(const T& other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value != other;
  }

  bool operator<(const T& other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value < other;
  }

  bool operator<=(const T& other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value <= other;
  }

  bool operator>(const T& other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value > other;
  }

  bool operator>=(const T& other) const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return value >= other;
  }

private:
  T value;
  std::condition_variable cv;
  mutable std::mutex mutex;
};