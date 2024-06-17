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
#include <list>
#include <map>

namespace eCAL
{
  namespace Util
  {
    /**
     * @brief A map that stores key-value pairs with an expiration time.
     *
     * @tparam Key The type of the keys.
     * @tparam Value The type of the values.
     */
    template <typename Key, typename Value>
    class CExpMap
    {
    public:
      using clock_type = std::chrono::steady_clock;
      using duration_type = clock_type::duration;

    private:
      using ExpiryTime     = clock_type::time_point;
      using ExpiryIterator = typename std::list<std::pair<ExpiryTime, Key>>::iterator;
      using ValueType      = std::pair<Key, Value>;

      struct MapValue
      {
        Value          value;
        ExpiryTime     expiry_time;
        ExpiryIterator expiry_iter;
      };

      duration_type                         expiration_time_;
      std::map<Key, MapValue>               map_;
      std::list<std::pair<ExpiryTime, Key>> expiry_list_;

    public:
      /**
       * @brief Constructs a CExpMap with a default expiration time of 5000 milliseconds.
       */
      CExpMap() : expiration_time_(std::chrono::milliseconds(5000)) {}

      /**
       * @brief Constructs a CExpMap with a specified expiration time.
       *
       * @param expiration_time  The expiration time for entries in the map.
       */
      explicit CExpMap(duration_type expiration_time) : expiration_time_(expiration_time) {}

      /**
       * @brief Sets a new expiration time for the key-value pairs in the map.
       *
       * @param expiration_time  The new expiration time.
       */
      void set_expiration(duration_type expiration_time)
      {
        expiration_time_ = expiration_time;
      }

      /**
       * @brief Inserts a key-value pair into the map with the current expiration time.
       *
       * @param key    The key to insert.
       * @param value  The value to insert.
       */
      void insert(const Key& key, const Value& value)
      {
        ExpiryTime expiry_time = clock_type::now() + expiration_time_;
        auto expiry_iter = expiry_list_.insert(expiry_list_.end(), std::make_pair(expiry_time, key));
        map_[key] = { value, expiry_time, expiry_iter };
      }

      /**
       * @brief Gets the value associated with the given key if it has not expired.
       *
       * @param key     The key to get the value for.
       * @param value   The value associated with the key.
       *
       * @return true   If the key exists and has not expired.
       * @return false  If the key does not exist or has expired, resetting its expiration time.
       */
      bool get(const Key& key, Value& value)
      {
        auto it = map_.find(key);
        if (it != map_.end())
        {
          if (clock_type::now() < it->second.expiry_time)
          {
            value = it->second.value;
            reset_expiration(key); // reset expiration time
            return true;
          }
          else
          {
            expiry_list_.erase(it->second.expiry_iter);
            map_.erase(it);
          }
        }
        return false;
      }

      /**
       * @brief Accesses the value associated with the given key, resetting its expiration time.
       *
       * @param key  The key to access the value for.
       *
       * @return     The value associated with the key.
       */
      Value& operator[](const Key& key)
      {
        auto it = map_.find(key);
        auto now = clock_type::now();
        if (it == map_.end() || now >= it->second.expiry_time)
        {
          ExpiryTime expiry_time = now + expiration_time_;
          auto expiry_iter = expiry_list_.insert(expiry_list_.end(), std::make_pair(expiry_time, key));
          map_[key] = { Value(), expiry_time, expiry_iter };
        }
        else
        {
          reset_expiration(key); // reset expiration time
        }
        return map_[key].value;
      }

      /**
       * @brief Erases the key-value pair associated with the given key.
       *
       * @param key     The key to erase.
       *
       * @return true   If the key existed and was erased.
       * @return false  If the key did not exist.
       */
      bool erase(const Key& key)
      {
        auto it = map_.find(key);
        if (it != map_.end())
        {
          expiry_list_.erase(it->second.expiry_iter);
          map_.erase(it);
          return true;
        }
        return false;
      }

      /**
       * @deprecated (please use erase_expired)
       */
      void remove_deprecated()
      {
        erase_expired();
      }

      /**
       * @brief Erase all expired key-value pairs from the map.
       */
      void erase_expired()
      {
        auto now = clock_type::now();
        while (!expiry_list_.empty() && expiry_list_.front().first <= now)
        {
          map_.erase(expiry_list_.front().second);
          expiry_list_.pop_front();
        }
      }

      /**
       * @brief Clears all key-value pairs from the map.
       */
      void clear()
      {
        map_.clear();
        expiry_list_.clear();
      }

      /**
       * @brief Gets the number of key-value pairs in the map.
       *
       * @return  The number of key-value pairs in the map.
       */
      size_t size() const
      {
        return map_.size();
      }

      /**
       * @brief Checks if the map is empty.
       *
       * @return true   If the map is empty.
       * @return false  If the map is not empty.
       */
      bool empty() const
      {
        return map_.empty();
      }

      /**
       * @brief An iterator for the CExpMap class.
       */
      class iterator
      {
      private:
        typename std::map<Key, MapValue>::iterator map_iter_;
        typename std::map<Key, MapValue>::iterator map_end_;
        ExpiryTime now_;

        void advance_to_valid()
        {
          while (map_iter_ != map_end_ && map_iter_->second.expiry_time <= now_)
          {
            ++map_iter_;
          }
        }

      public:
        iterator(typename std::map<Key, MapValue>::iterator map_iter,
          typename std::map<Key, MapValue>::iterator map_end)
          : map_iter_(map_iter), map_end_(map_end), now_(clock_type::now())
        {
          advance_to_valid();
        }

        iterator& operator++()
        {
          ++map_iter_;
          advance_to_valid();
          return *this;
        }

        std::pair<const Key, Value> operator*() const
        {
          return { map_iter_->first, map_iter_->second.value };
        }

        bool operator!=(const iterator& other) const
        {
          return map_iter_ != other.map_iter_;
        }

        bool operator==(const iterator& other) const
        {
          return map_iter_ == other.map_iter_;
        }
      };

      /**
       * @brief A const_iterator for the CExpMap class.
       */
      class const_iterator
      {
      private:
        typename std::map<Key, MapValue>::const_iterator map_iter_;
        typename std::map<Key, MapValue>::const_iterator map_end_;
        ExpiryTime now_;

        void advance_to_valid()
        {
          while (map_iter_ != map_end_ && map_iter_->second.expiry_time <= now_)
          {
            ++map_iter_;
          }
        }

      public:
        const_iterator(typename std::map<Key, MapValue>::const_iterator map_iter,
          typename std::map<Key, MapValue>::const_iterator map_end)
          : map_iter_(map_iter), map_end_(map_end), now_(clock_type::now())
        {
          advance_to_valid();
        }

        const_iterator& operator++()
        {
          ++map_iter_;
          advance_to_valid();
          return *this;
        }

        std::pair<const Key, const Value> operator*() const
        {
          return { map_iter_->first, map_iter_->second.value };
        }

        bool operator!=(const const_iterator& other) const
        {
          return map_iter_ != other.map_iter_;
        }

        bool operator==(const const_iterator& other) const
        {
          return map_iter_ == other.map_iter_;
        }
      };

      /**
       * @brief Returns an iterator to the beginning of the map.
       *
       * @return  An iterator to the beginning of the map.
       */
      iterator begin()
      {
        return iterator(map_.begin(), map_.end());
      }

      /**
       * @brief Returns an iterator to the end of the map.
       *
       * @return  An iterator to the end of the map.
       */
      iterator end()
      {
        return iterator(map_.end(), map_.end());
      }

      /**
       * @brief Returns a const_iterator to the beginning of the map.
       *
       * @return  A const_iterator to the beginning of the map.
       */
      const_iterator cbegin() const
      {
        return const_iterator(map_.cbegin(), map_.cend());
      }

      /**
       * @brief Returns a const_iterator to the end of the map.
       *
       * @return  A const_iterator to the end of the map.
       */
      const_iterator cend() const
      {
        return const_iterator(map_.cend(), map_.cend());
      }

      /**
       * @brief Returns a const_iterator to the beginning of the map.
       *
       * @return  A const_iterator to the beginning of the map.
       */
      const_iterator begin() const
      {
        return cbegin();
      }

      /**
       * @brief Returns a const_iterator to the end of the map.
       *
       * @return  A const_iterator to the end of the map.
       */
      const_iterator end() const
      {
        return cend();
      }

      /**
       * @brief Finds the iterator to the element with the given key.
       *
       * @param key  The key to find.
       * @return     An iterator to the element if it exists and is not expired, otherwise end().
       */
      iterator find(const Key& key)
      {
        auto it = map_.find(key);
        if (it != map_.end() && clock_type::now() < it->second.expiry_time)
        {
          return iterator(it, map_.end());
        }
        return end();
      }

      /**
       * @brief Finds the const_iterator to the element with the given key.
       *
       * @param key  The key to find.
       * @return     A const_iterator to the element if it exists and is not expired, otherwise cend().
       */
      const_iterator find(const Key& key) const
      {
        auto it = map_.find(key);
        if (it != map_.end() && clock_type::now() < it->second.expiry_time)
        {
          return const_iterator(it, map_.cend());
        }
        return cend();
      }

    private:
      /**
       * @brief Resets the expiration time of the given key.
       *
       * @param key  The key to reset the expiration time for.
       */
      void reset_expiration(const Key& key)
      {
        auto map_iter = map_.find(key);
        if (map_iter != map_.end())
        {
          // update the expiry time in place and move the element to the end of the list
          ExpiryTime new_expiry_time = clock_type::now() + expiration_time_;
          map_iter->second.expiry_time = new_expiry_time;
          expiry_list_.splice(expiry_list_.end(), expiry_list_, map_iter->second.expiry_iter);
          map_iter->second.expiry_iter = std::prev(expiry_list_.end());
          map_iter->second.expiry_iter->first = new_expiry_time;
        }
      }
    };
  }
}
