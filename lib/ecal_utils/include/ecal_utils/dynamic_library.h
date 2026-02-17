/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include <string>
#include <variant>

namespace EcalUtils
{
  struct DlError
  {
    enum class Code
    {
      InvalidArgument,
      LoadFailed,
      SymbolNotFound,
      PlatformError
    };

    Code code{ Code::PlatformError };
    std::string message;
  };

  class DynamicLibrary
  {
  public:
    DynamicLibrary(const DynamicLibrary&) = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    DynamicLibrary(DynamicLibrary&& other) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&&) = delete;

    ~DynamicLibrary() noexcept;

    // Factory method to load a dynamic library from a given path. 
    // The path can be absolute or relative, but must include the library file name and extension.
    static std::variant<DynamicLibrary, DlError> Load(const std::string& path_) noexcept;

    std::variant<void*, DlError> LoadRawSymbol(const char* name_) const noexcept;

    template <typename T>
    std::variant<T, DlError> LoadSymbol(const char* name_) const noexcept
    {
      auto raw = LoadRawSymbol(name_);
      if (std::holds_alternative<DlError>(raw))
        return std::get<DlError>(std::move(raw));

      // NOTE: converting void* to function pointer is technically not strictly portable in ISO C++,
      // but is the standard, practical approach on POSIX/Windows for dlsym/GetProcAddress.
      return reinterpret_cast<T>(std::get<void*>(raw));
    }

    template <typename T>
    T SymbolOrNone(const char* name_) const noexcept
    {
      auto symbol_or_error = LoadSymbol<T>(name_);
      if (std::holds_alternative<T>(symbol_or_error))
        return std::get<T>(symbol_or_error);
      return nullptr;
    }

  private:
    // No default construction: only open() can create an instance
    explicit DynamicLibrary(void* handle_) noexcept;

    void Unload() noexcept;

    void* m_handle{ nullptr }; // opaque OS handle (HMODULE or dlopen handle)
  };
}
