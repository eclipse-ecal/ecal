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

/**
 * @brief  relocatable circular queue for shared memory
**/

#pragma once

#include <cstdint>
#include <stdexcept>
#include <limits>
#include <iterator>
#include <cassert>

template<class T>
class RelocatableCircularQueue{
public:
  static constexpr auto invalid_index = std::numeric_limits<std::uint64_t>::max();

  class iterator : public std::iterator<std::forward_iterator_tag, T, std::uint64_t> {
  public:
    explicit iterator(RelocatableCircularQueue<T>& relocatable_circular_queue, std::uint64_t index): m_relocatable_circular_queue(relocatable_circular_queue), m_index(index){
    }

    iterator& operator++()
    {
      if (m_index == m_relocatable_circular_queue.m_header->front_index)
        m_index = invalid_index;
      else
        m_index = (m_index + m_relocatable_circular_queue.m_header->max_size - 1) % m_relocatable_circular_queue.m_header->max_size;

      return *this;
    }

    iterator operator++(int)
    {
      iterator return_value = *this;
      ++(*this);
      return return_value;
    }

    bool operator==(iterator other) const {
      return m_index == other.m_index;
    }

    bool operator!=(iterator other) const {
      return m_index != other.m_index;
    }

    T& operator*()
    {
      assert(m_index != invalid_index);
      return *m_relocatable_circular_queue.Value(m_index);
    }

    const T& operator*() const
    {
      assert(m_index != invalid_index);
      return *m_relocatable_circular_queue.Value(m_index);
    }
  private:
    friend class RelocatableCircularQueue<T>;

    RelocatableCircularQueue<T>& m_relocatable_circular_queue;
    std::uint64_t m_index;
  };

  iterator begin()
  {
    if(m_header->size == 0)
      return iterator(*this, invalid_index);
    else
      return iterator(*this, m_header->back_index);
  }

  iterator end()
  {
    return iterator(*this, invalid_index);
  }

  const iterator begin() const
  {
    if(m_header->size == 0)
      return iterator(*this, invalid_index);
    else
      return iterator(*this, m_header->back_index);
  }

  const iterator end() const
  {
    return iterator(*this, invalid_index);
  }

  RelocatableCircularQueue(): m_base_address(nullptr), m_header(nullptr)
  {}

  void Push(const T& value)
  {
    assert(m_base_address != nullptr);

    if (m_header->size != 0)
      m_header->back_index = (m_header->back_index + 1) % m_header->max_size;

    *Value(m_header->back_index) = value;

    if(m_header->size != m_header->max_size)
      ++m_header->size;
    else
      m_header->front_index = (m_header->front_index + 1) % m_header->max_size;
  }

  void Pop()
  {
    assert(m_base_address != nullptr);

    if(m_header->size == 0)
      throw std::out_of_range("Queue is empty");

    --m_header->size;
    if(m_header->size != 0)
      m_header->front_index = (m_header->front_index + 1) % m_header->max_size;
  }

  T& Front()
  {
    assert(m_base_address != nullptr);

    return *Value(m_header->front_index);
  }

  const T& Front() const
  {
    assert(m_base_address != nullptr);

    return *Value(m_header->front_index);
  }

  T& Back()
  {
    assert(m_base_address != nullptr);

    return *Value(m_header->back_index);
  }

  const T& Back() const
  {
    assert(m_base_address != nullptr);

    return *Value(m_header->back_index);
  }

  void SetBaseAddress(void *base_address)
  {
    m_base_address = base_address;
    m_header = static_cast<Header *>(m_base_address);
  }

  void Reset(std::size_t max_size)
  {
    assert((m_base_address != nullptr) && (max_size != 0));

    m_header->max_size = max_size;
    m_header->front_index = 0;
    m_header->back_index = 0;
    m_header->size = 0;
  }

  std::size_t Size() const
  {
    assert(m_base_address != nullptr);
    return m_header->size;
  }

  std::size_t MaxSize() const
  {
    assert(m_base_address != nullptr);
    return m_header->max;
  }

  std::size_t OccupiedMemorySize() const
  {
    assert(m_base_address != nullptr);
    return sizeof (Header) + sizeof(T) * m_header->max_size;
  }

  static std::size_t PresumablyOccupiedMemorySize(std::size_t max_size)
  {
    return sizeof (Header) + sizeof(T) * max_size;
  }


private:
#pragma pack(push, 1)
  struct Header
  {
    std::uint64_t back_index;
    std::uint64_t front_index;
    std::size_t size;
    std::size_t max_size;
  };
#pragma pack(pop)

  void* m_base_address;
  Header* m_header;

  T * Value(std::uint64_t index)
  {
    assert(m_base_address != nullptr);
    return reinterpret_cast<T *>(static_cast<char *>(m_base_address) + sizeof(Header) + sizeof(T) * index);
  }

  const T * Value(std::uint64_t index) const
  {
    assert(m_base_address != nullptr);
    return reinterpret_cast<const T *>(static_cast<char *>(m_base_address) + sizeof(Header) + sizeof(T) * index);
  }
};
