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
    * @brief A time expiration map
    **/
    template<class Key,
      class T,
      class clock_type = std::chrono::steady_clock,
      class Compare = std::less<Key>,
      class Alloc   = std::allocator<std::pair<const Key, T> > >
    class CExpiredMap
    {
    public:
      using allocator_type  = Alloc;
      using value_type      = std::pair<const Key, T>;
      using reference       = typename Alloc::reference;
      using size_type       = typename Alloc::size_type;
      using key_type        = Key;
      using mapped_type     = T;

    private:
      struct AccessTimestampListEntry
      {
        typename clock_type::time_point timestamp;
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
      using InternalMapType = std::map<Key, std::pair<T, typename AccessTimestampListType::iterator>>;

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
          return std::make_pair(it->first, it->second.first);
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
          return std::make_pair(it->first, it->second.first);
        }

        //friend void swap(iterator& lhs, iterator& rhs); //C++11 I think
        bool operator==(const const_iterator& rhs) const { return it == rhs.it; }
        bool operator!=(const const_iterator& rhs) const { return it != rhs.it; }

      private:
        typename InternalMapType::const_iterator it;
      };


      // Constructor specifies the timeout of the map
      CExpiredMap() : _timeout(std::chrono::milliseconds(5000)) {};
      explicit CExpiredMap(typename clock_type::duration t) : _timeout(t) {};

      /**
      * @brief  set expiration time
      **/
      void set_expiration(typename clock_type::duration t) { _timeout = t; };

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

      // Element access
      // Obtain value of the cached function for k 
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
          update_timestamp(k);
        }

        // Return the retrieved value 
        return (*it).second.first;
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

      // Purge the timed out elements from the cache 
      void erase_expired(std::list<Key>* keys_erased_from_expired_map = nullptr) //-V826
      {
        // Assert method is never called when cache is empty 
        //assert(!_access_timestamps_list.empty());
        typename clock_type::time_point eviction_limit = get_curr_time() - _timeout;

        auto it(_access_timestamps_list.begin());

        while (it != _access_timestamps_list.end() && it->timestamp < eviction_limit)
        {
          if (keys_erased_from_expired_map != nullptr) keys_erased_from_expired_map->push_back(it->corresponding_map_key);
          _internal_map.erase(it->corresponding_map_key); // erase the element from the map 
          it = _access_timestamps_list.erase(it);         // erase the element from the list
        }
      }

      // Remove specific element from the cache
      bool erase(const Key& k)
      {
        auto it = _internal_map.find(k);
        if (it != _internal_map.end())
        {
          _access_timestamps_list.erase(it->second.second); // erase the element from the list
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
      void update_timestamp(const Key& k)
      {
        auto it_in_map = _internal_map.find(k);
        if (it_in_map != _internal_map.end())
        {
          auto& it_in_list = it_in_map->second.second;

          // move the element to the end of the list
          _access_timestamps_list.splice(_access_timestamps_list.end(), _access_timestamps_list, it_in_list);

          // update the timestamp
          it_in_list->timestamp = get_curr_time();
        }
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
            std::make_pair(v, it)
          )
        );
        // return iterator to newly inserted element.
        return ret;
      }

      typename clock_type::time_point get_curr_time()
      {
        return clock_type::now();
      }

      // Key access history 
      AccessTimestampListType _access_timestamps_list;

      // Key-to-value lookup 
      InternalMapType _internal_map;

      // Timeout of map
      typename clock_type::duration _timeout;
    };
  }
}
