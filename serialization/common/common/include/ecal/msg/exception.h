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

#include <exception>
#include <sstream>
#include <string>

namespace eCAL
{
    /**
     * @brief Exception thrown when an error occurs during deserialization.
     *
     * This exception is raised when any kind of error occurs during the deserialization process,
     * such as when the data is corrupt or does not match the expected format.
     */
    class DeserializationException : public std::exception
    {
    public:
      /**
       * @brief Constructs a new DeserializationException object.
       *
       * Initializes the exception with a descriptive error message.
       *
       * @param message A string containing a detailed description of the error.
       */
      DeserializationException(const std::string& message) : message_(message) {}
      virtual const char* what() const noexcept { return message_.c_str(); }
    private:
      std::string message_;
    };

    /**
     * @brief Exception thrown when a given datatype is incompatible with a serializer.
     *
     * This exception is raised when a serializer cannot accept the provided datatype,
     * for example when creating an input channel or subscriber. It encapsulates both the
     * expected and the actual datatype information to facilitate debugging and error handling.
     *
     * @tparam DatatypeInformation A type that holds information about a datatype.
     *        It is expected to have at least the members `encoding` and `name`.
     */
    template<typename DatatypeInformation>
    class TypeMismatchException : public std::exception
    {
    public:
      /**
       * @brief Constructs a new TypeMismatchException object.
       *
       * Initializes the exception with the expected and actual datatype information.
       * An error message is generated detailing the mismatch between the two datatypes.
       *
       * @param expected_datatype_ The datatype information that was expected.
       * @param actual_datatype_ The datatype information that was actually provided.
       */
      TypeMismatchException(const DatatypeInformation& expected_datatype_,
        const DatatypeInformation& actual_datatype_)
        : expected_datatype(expected_datatype_),
        actual_datatype(actual_datatype_)
      {
        std::ostringstream oss;
        oss << "Type Mismatch error: Expected and actual datatypes do not match\n"
          << "Expected type:\n"
          << "  encoding: " << expected_datatype.encoding << "\n"
          << "  name: " << expected_datatype.name << "\n"
          << "Actual type:\n"
          << "  encoding: " << actual_datatype.encoding << "\n"
          << "  name: " << actual_datatype.name << "\n";
        error_message = oss.str();
      }

      virtual const char* what() const noexcept override {
        return error_message.c_str();
      }

      // Optionally, add accessors for the datatype information.
      const DatatypeInformation& getExpectedDatatype() const noexcept {
        return expected_datatype;
      }

      const DatatypeInformation& getActualDatatype() const noexcept {
        return actual_datatype;
      }

    private:
      const DatatypeInformation expected_datatype;
      const DatatypeInformation actual_datatype;
      std::string error_message;  // Store the error message persistently.
    };
}