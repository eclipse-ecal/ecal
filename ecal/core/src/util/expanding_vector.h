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
 * @brief  std::vector wrapper, that never deallocates element, just clears them
**/

#pragma once

#include <iterator>
#include <stdexcept>
#include <vector>

namespace eCAL
{
  namespace Util
  {
    /**
     * @brief A vector that never destructs its elements, but only calls clear on them.
     *
     * @tparam T         The type of the values. Must provide a `.clear()` function
     * 
     * This class is *not* threadsafe and needs to be protected by locks / mutexes in multithreaded environments.
     * 
     * From the outside / for the user, this class acts as a regular std::vector.
     * However, when calling clear(), a regular vector will destroy the elements which are stored in this vector.
     * This class, will instead call the `clear()` functions on all members.
     */

     // Templated class CExpandingVector
    template <class T>
    class CExpandingVector {
    public:
        using value_type = typename std::vector<T>::value_type;
        using allocator_type = typename std::vector<T>::allocator_type;
        using size_type = typename std::vector<T>::size_type;
        using difference_type = typename std::vector<T>::difference_type;
        using reference = typename std::vector<T>::reference;
        using const_reference = typename std::vector<T>::const_reference;
        using pointer = typename std::vector<T>::pointer;
        using const_pointer = typename std::vector<T>::const_pointer;
        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;
        using reverse_iterator = typename std::vector<T>::reverse_iterator;
        using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

    private:
        std::vector<T> data;
        size_t internal_size{ 0 };  // Track size separately
    
    public:
        // Access to the internal size
        size_t size() const {
            return internal_size;
        }
    
        // Clear the content but not the underlying vector
        void clear() {
            for (auto& elem : data) {
                elem.clear();  // Call the clear() function on individual elements
            }
            // We don't modify the size of the underlying vector but keep the internal size consistent.
            internal_size = 0;
        }
    
        // Add new element to the vector
        void push_back(const T& value) {
            if (internal_size < data.size()) {
                data[internal_size] = value;  // Reuse space
            } else {
                data.push_back(value);  // Expand the vector if needed
            }
            ++internal_size;
        }

        // Push back (rvalue reference version for move semantics)
        void push_back(T&& value) {
          if (internal_size < data.size()) {
            data[internal_size] = std::move(value);  // Reuse space
          }
          else {
            data.push_back(std::move(value));  // Move into the vector
          }
          ++internal_size;
        }

        T& push_back()
        {
          if (internal_size == data.size()) {
            data.push_back(T{});  // Expand the vector if needed, else do nothing
          }
          ++internal_size;

          return back();
        }

        // Access the first element
        T& front() {
          if (internal_size == 0) {
            throw std::out_of_range("CExpandingVector is empty");
          }
          return data.front();  // Return the first element
        }

        const T& front() const {
          if (internal_size == 0) {
            throw std::out_of_range("CExpandingVector is empty");
          }
          return data.front();  // Return the first element
        }

        // Access the last element
        T& back() {
          if (internal_size == 0) {
            throw std::out_of_range("CExpandingVector is empty");
          }
          return data[internal_size - 1];  // Return the last element
        }

        const T& back() const {
          if (internal_size == 0) {
            throw std::out_of_range("CExpandingVector is empty");
          }
          return data[internal_size - 1];  // Return the last element
        }


        // Resize the vector
        void resize(size_t new_size) {
            if (new_size > data.size()) {
                data.resize(new_size);
            }
            internal_size = new_size;
        }
    
        // Access element with bounds checking
        T& at(size_t index) {
            if (index >= internal_size) {
                throw std::out_of_range("Index out of bounds");
            }
            return data.at(index);
        }
    
        // Const access element with bounds checking
        const T& at(size_t index) const {
            if (index >= internal_size) {
                throw std::out_of_range("Index out of bounds");
            }
            return data.at(index);
        }
    
        // Iterator functions
        iterator begin() {
            return data.begin();
        }
    
        const_iterator begin() const {
            return data.begin();
        }
    
        iterator end() {
            return data.begin() + internal_size;
        }
    
        const_iterator end() const {
            return data.begin() + internal_size;
        }
    
        // Access the underlying capacity
        size_t capacity() const {
            return data.capacity();
        }
    
        // Operator[] overload
        T& operator[](size_t index) {
            return data[index];
        }
    
        const T& operator[](size_t index) const {
            return data[index];
        }
    
        // Check if vector is empty
        bool empty() const {
            return internal_size == 0;
        }
    
        // Get underlying vector's full size (not just the internal size)
        size_t full_size() const {
            return data.size();
        }

        // Equality operator (compares size and elements)
        bool operator==(const CExpandingVector<T>& other) const {
          // Check if sizes are equal
          if (internal_size != other.internal_size) {
            return false;
          }

          // Compare elements
          for (size_t i = 0; i < internal_size; ++i) {
            if (!(data[i] == other.data[i])) {
              return false;
            }
          }

          return true;  // Sizes are equal and all elements are the same
        }

        bool operator!=(const CExpandingVector<T>& other) const {
          return !(*this == other);  // Use the equality operator to implement inequality
        }
    };

  }
}
