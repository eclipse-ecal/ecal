/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

/**
 * @brief  eCAL map with time expiration
**/

#pragma once

#include <chrono>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace eCAL
{
  namespace Util
  {
    /**
     * @brief A map that stores key-value pairs and the time at which they have last been updated.
     *
     * @tparam Key       The type of the keys.
     * @tparam T         The type of the values.
     * @tparam ClockType The type of the clock based on which the Map expires its elements
     * 
     * This class is *not* threadsafe and needs to be protected by locks / mutexes in multithreaded environments.
     * 
     * From the outside / for the user, this class acts as a regular std::map.
     * However, it provides one additional function (erase_expired) that removes expired elements from this map.
     * Elements are considered to be expired, if they have not been accessed (via `operator[]`) within a given timeout period.
     * 
     * Internally, this is realized by storing both a map and a list.
     * The map stores the regular key, and then the actual value and additional an iterator into the timestamp list.
     * The timestamp list stores together a timestamp and the key which was inserted into the list at that given timestamp.
     * It is always kept in a sorted order.
     * 
     * Whenever a map element is accessed, the responding timestamp is updated and moved to the end of the list.
     * This happens in constant time.
     */
    template<class Key,
      class T,
      class ClockType = std::chrono::steady_clock,
      class Compare = std::less<Key>,
      class Alloc   = std::allocator<std::pair<const Key, T> > >
    class CExpirationMap
    {
    public:
      // Type declarations necessary to be compliant to a regular map.
      using allocator_type  = Alloc;
      using value_type      = std::pair<const Key, T>;
      using reference       = typename Alloc::reference;
      using size_type       = typename Alloc::size_type;
      using key_type        = Key;
      using mapped_type     = T;

    private:
      struct AccessTimestampListEntry
      {
        typename ClockType::time_point timestamp;
        Key corresponding_map_key;
      };

      // Key access history, most recent at back 
      using AccessTimestampListType = std::list<AccessTimestampListEntry>;

      struct InternalMapEntry
      {
        T map_value;
        typename AccessTimestampListType::iterator timestamp_list_iterator;
      };

      // Key to value and key history iterator 
      using InternalMapType = std::map<Key, InternalMapEntry>;

    public:

      class iterator
      {
        friend class const_iterator;

      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = std::pair<Key, T>;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::pair<Key, T>*;
        using reference         = std::pair<Key, T>&;

        explicit iterator(const typename InternalMapType::iterator _it)
          : it(_it)
        {}

        iterator& operator++()
        {
          it++;
          return *this;
        } //prefix increment

        iterator& operator--()
        {
          it--;
          return *this;
        } //prefix decrement

        std::pair<Key, T> operator*() const
        {
          return std::make_pair(it->first, it->second.map_value);
        }

        //friend void swap(iterator& lhs, iterator& rhs); //C++11 I think
        bool operator==(const iterator& rhs) const { return it == rhs.it; }
        bool operator!=(const iterator& rhs) const { return it != rhs.it; }

      private:
        typename InternalMapType::iterator it;
      };

      class const_iterator
      {
      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = std::pair<Key, T>;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::pair<Key, T>*;
        using reference         = std::pair<Key, T>&;

        explicit const_iterator(const iterator& other)
          : it(other.it)
        {}

        explicit const_iterator(const typename InternalMapType::const_iterator _it)
          : it(_it)
        {}

        const_iterator& operator++()
        {
          it++;
          return *this;
        } //prefix increment

        const_iterator& operator--()
        {
          it--;
          return *this;
        } //prefix decrement
          //reference operator*() const

        std::pair<Key, T> operator*() const
        {
          return std::make_pair(it->first, it->second.map_value);
        }

        //friend void swap(iterator& lhs, iterator& rhs); //C++11 I think
        bool operator==(const const_iterator& rhs) const { return it == rhs.it; }
        bool operator!=(const const_iterator& rhs) const { return it != rhs.it; }

      private:
        typename InternalMapType::const_iterator it;
      };


      // Constructor specifies the timeout of the map
      CExpirationMap() : _timeout(std::chrono::milliseconds(5000)) {};
      explicit CExpirationMap(typename ClockType::duration t) : _timeout(t) {};

      /**
      * @brief  set expiration time
      **/
      void set_expiration(typename ClockType::duration t) { _timeout = t; };

      // Iterators:
      iterator begin() noexcept
      {
        return iterator(_internal_map.begin());
      }

      iterator end() noexcept
      {
        return iterator(_internal_map.end());
      }

      const_iterator begin() const noexcept
      {
        return const_iterator(_internal_map.begin());
      }

      const_iterator end() const noexcept
      {
        return const_iterator(_internal_map.end());
      }

      // Const begin and end functions
      const_iterator cbegin() const noexcept {
        return const_iterator(_internal_map.cbegin());
      }

      const_iterator cend() const noexcept {
        return const_iterator(_internal_map.cend());
      }

      // Capacity
      bool empty() const noexcept
      {
        return _internal_map.empty();
      }

      size_type size() const noexcept
      {
        return _internal_map.size();
      }

      size_type max_size() const noexcept
      {
        return  _internal_map.max_size();
      }

      /**
       * @brief Accesses the value associated with the given key, resetting its expiration time.
       *
       * @param key  The key to access the value for.
       *
       * @return     The value associated with the key.
       */
      T& operator[](const Key& k)
      {
        // Attempt to find existing record 
        typename InternalMapType::iterator it
          = _internal_map.find(k);

        if (it == _internal_map.end())
        {
          // We don't have it: 
          // Evaluate function and create new record 
          T v{};
          auto ret = insert(k, v);
          it = ret.first;
        }
        else
        {
          // We do have it: 
          // Update access record by moving 
          // accessed key to back of list 
          update_timestamp(it);
        }

        // Return the retrieved value 
        return (*it).second.map_value;
      };

      mapped_type& at(const key_type& k)
      {
        return _internal_map.at(k).first;
      }

      const mapped_type& at(const key_type& k) const
      {
        return _internal_map.at(k).first;
      }

      // Modifiers
      std::pair<iterator, bool> insert(const value_type& val)
      {
        auto result = insert(val.first, val.second);
        return std::make_pair(iterator(result.first), result.second);
      }

      // Operations
      iterator find(const key_type& k)
      {
        return iterator(_internal_map.find(k));
      }

      const_iterator find(const Key& k) const
      {
        return const_iterator(_internal_map.find(k));
      }

      /**
       * @brief Erase all expired key-value pairs from the map.
       * 
       * This function erases all expired key-value pairs from the internal map / timestamp list.
       * The CExpirationMap class does not call this function internally, it has to be called explicitly by the user.
       */
      std::map<Key, T> erase_expired()
      {
        std::map<Key, T> erased_values;
        // To erase timestamps from the map, the time point of the last access is calculated, all older entries will be erased.
        // Since the list is sorted, we need to remove everything from the first element until the eviction limit.
        typename ClockType::time_point eviction_limit = get_curr_time() - _timeout;
        auto it(_access_timestamps_list.begin());
        while (it != _access_timestamps_list.end() && it->timestamp < eviction_limit)
        {
          auto erased_value = _internal_map.find(it->corresponding_map_key);
          // for performance reason, we should be able to move from the map, however with C++17 we can use std::map::extract
          erased_values[it->corresponding_map_key] = erased_value->second.map_value;
          _internal_map.erase(it->corresponding_map_key); // erase the element from the map 
          it = _access_timestamps_list.erase(it);         // erase the element from the list
        }
        return erased_values;
      }

      // Remove specific element from the cache
      bool erase(const Key& k)
      {
        auto it = _internal_map.find(k);
        if (it != _internal_map.end())
        {
          _access_timestamps_list.erase(it->second.timestamp_list_iterator); // erase the element from the list
          _internal_map.erase(k);                // erase the element from the map
          return true;
        }
        return false;
      }

      // Remove all elements from the cache 
      void clear()
      {
        _internal_map.clear(); // erase all elements from the map 
        _access_timestamps_list.clear();  // erase all elements from the list
      }

    private:

      // Maybe pass the iterator instead of the key? or at least only get k once
      void update_timestamp(const typename InternalMapType::iterator& it_in_map)
      {
        auto& it_in_list = it_in_map->second.timestamp_list_iterator;

        // move the element to the end of the list
        _access_timestamps_list.splice(_access_timestamps_list.end(), _access_timestamps_list, it_in_list);

        // update the timestamp
        it_in_list->timestamp = get_curr_time();
      }
      
      // Record a fresh key-value pair in the cache
      std::pair<typename InternalMapType::iterator, bool> insert(const Key& k, const T& v)
      {
        // sorted list, containing (pair ( timestamp, K))
        auto it = _access_timestamps_list.emplace(_access_timestamps_list.end(), AccessTimestampListEntry{ get_curr_time(), k });

        // entry mapping k -> pair (T, iterator(pair(timestamp, K)))
        auto ret = _internal_map.emplace(
          std::make_pair(
            k,
            InternalMapEntry{ v, it }
          )
        );
        // return iterator to newly inserted element.
        return ret;
      }

      typename ClockType::time_point get_curr_time()
      {
        return ClockType::now();
      }

      // Key access history 
      AccessTimestampListType _access_timestamps_list;

      // Key-to-value lookup 
      InternalMapType _internal_map;

      // Timeout of map
      typename ClockType::duration _timeout;
    };
  }
}
