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

#pragma once

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <unordered_map>

namespace eCAL
{

  template <std::size_t WINDOW_SIZE = 1024>
  class BitsetWindow {
    std::bitset<WINDOW_SIZE> bits;
    std::size_t base = 0;

  public:
    BitsetWindow(size_t base_) :
      base{ base_ }
    {}

    bool Contains(std::size_t value) const {
      return Compare(value) == 0;
    }

    int Compare(std::size_t value) const
    {
      if (value < base) return -1;
      if (value >= base + WINDOW_SIZE) return 1;
      return 0;
    }

    void Clear(std::size_t new_base) {
      bits.reset();
      base = new_base;
    }

    bool Has(std::size_t value) const {
      // Should we throw exceptions, if the values are not in range?
      return Contains(value) && bits.test(GetOffset(value));
    }

    void Set(std::size_t value) {
      // Should we throw exceptions, if the values are not in range?
      if (Contains(value)) {
        bits.set(GetOffset(value));
      }
    }

  private:
    std::size_t GetOffset(std::size_t value) const {
      return value - base;
    }
  };

  template <std::size_t WINDOW_SIZE>
  bool operator<(std::size_t value, const BitsetWindow<WINDOW_SIZE>& window) {
    return window.Compare(value) < 0;
  }

  template <std::size_t WINDOW_SIZE>
  bool operator>(std::size_t value, const BitsetWindow<WINDOW_SIZE>& window) {
    return window.Compare(value) > 0;
  }

  /*
  * This cache takes track of counters that have been registered.
  * It does so by using bitset windows.
  * The purpose of this class is to filter out duplicate or out of order eCAL Messages
  */
  template <std::size_t WINDOW_SIZE = 1024>
  class CounterCache {
  private:
    std::size_t current_base = WINDOW_SIZE;
    BitsetWindow<WINDOW_SIZE> current_window{ current_base };
    BitsetWindow<WINDOW_SIZE> previous{ current_base - WINDOW_SIZE };
    BitsetWindow<WINDOW_SIZE> next{ current_base + WINDOW_SIZE };
    std::size_t max_seen = 0;
    bool registered_any_value = false;

  public:
    enum class CounterInCache
    {
      True, // The counter has been set
      False, // The counter has not been set
      Unsure // It's unsure if the counter has been set, because it is outside of the tracked windows
    };

    /*
    CounterCache()
      : current_base{ 0 }
      , current_window{ current_base }
      , previous{ current_base - WINDOW_SIZE }
      , next{ current_base - WINDOW_SIZE }
    {}*/

    // Returns true if the counter value has already been registered
    CounterInCache HasCounter(std::size_t counter_value_) const
    {
      if (!registered_any_value)
      {
        return CounterInCache::False;
      }

      if (counter_value_ > max_seen)
      {
        return CounterInCache::False;
      }
      if (counter_value_ < previous)
      {
        return CounterInCache::Unsure;
      }
      if (current_window.Has(counter_value_) || previous.Has(counter_value_)) // next never has any values
      {
        return CounterInCache::True;
      }
      return CounterInCache::False;
    }

    // Returns true if value is greater than all previously set values
    bool IsMonotonic(std::size_t counter_value_) const
    {
      // The first value is always monotonic
      if (!registered_any_value)
      {
        return true;
      }
      return counter_value_ > max_seen;
    }

    // We update the cache.
    // Updating the cache with a value < then the currently smallest number stored does nothing
    // Updating with a value in the current sliding window updates the windows.
    // Updating with a value larger than what can currently be stored slides the windows.
    // We either slide by one
    // Or we completely reset
    void SetCounter(std::size_t counter_value_)
    {
      registered_any_value = true;

      max_seen = std::max(max_seen, counter_value_);

      if (counter_value_ < previous)
      {
        return;
      }

      if (current_window.Contains(counter_value_))
      {
        current_window.Set(counter_value_);
        return;
      }

      if (previous.Contains(counter_value_))
      {
        previous.Set(counter_value_);
        return;
      }

      // Shift by one
      if (next.Contains(counter_value_)) {
        ShiftOne(counter_value_);
        return;
      }

      Reset(counter_value_);
    }

  private:
    void ShiftOne(std::size_t value_)
    {
      current_base += WINDOW_SIZE;
      previous = current_window;
      current_window = next;
      next.Clear(current_base + WINDOW_SIZE);
      current_window.Set(value_);
    }

    void Reset(std::size_t value_)
    {
      current_base = value_ / WINDOW_SIZE * WINDOW_SIZE;
      current_window = BitsetWindow<WINDOW_SIZE>{ current_base };
      previous = BitsetWindow<WINDOW_SIZE>{ current_base - WINDOW_SIZE };
      next = BitsetWindow<WINDOW_SIZE>{ current_base + WINDOW_SIZE };
      current_window.Set(value_);
    }

  };

  template<typename Key, std::size_t WINDOW_SIZE = 1024>
  class CounterCacheMap {
  public:
    using CounterInCache = CounterCache::CounterInCache;

    CounterInCache HasCounter(const Key& k, std::size_t counter_value_) const
    {
      return cache_map_[k].HasCounter(counter_value_);
    }

    bool IsMonotonic(const Key& k, std::size_t counter_value_) const
    {
      return cache_map_[k].IsMonotonic(counter_value_);
    }

    void SetCounter(const Key& k, std::size_t counter_value_)
    {
      return cache_map_[k].Update(counter_value_);
    }

  private:
    std::unordered_map<Key, CounterCache<WINDOW_SIZE>> cache_map_;
  };
}