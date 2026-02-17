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

#include <ecal_utils/dynamic_library.h>

#include <cassert>
#include <utility>

#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <dlfcn.h>
#endif

namespace EcalUtils
{
  DynamicLibrary::DynamicLibrary(void* handle_) noexcept
    : m_handle(handle_)
  {
    assert(m_handle != nullptr);
  }

  DynamicLibrary::DynamicLibrary(DynamicLibrary&& other_) noexcept
    : m_handle(other_.m_handle)
  {
    other_.m_handle = nullptr;
  }

  DynamicLibrary::~DynamicLibrary() noexcept
  {
    Unload();
  }

  void DynamicLibrary::Unload() noexcept
  {
    if (!m_handle)
      return;

#if defined(_WIN32)
    ::FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#else
    ::dlclose(m_handle);
#endif

    m_handle = nullptr;
  }

  std::variant<DynamicLibrary, DlError> DynamicLibrary::Load(const std::string& path_) noexcept
  {
    if (path_.empty())
      return DlError{ DlError::Code::InvalidArgument, "Empty library path" };

#if defined(_WIN32)
    HMODULE h = ::LoadLibrary(path_.c_str());
    if (!h)
    {
      // Optional: improve message using GetLastError + FormatMessage
      return DlError{ DlError::Code::LoadFailed, "LoadLibrary failed for: " + path_ };
    }
    return DynamicLibrary(reinterpret_cast<void*>(h));
#else
    void* h = ::dlopen(path_.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!h)
    {
      const char* e = ::dlerror();
      return DlError{
        DlError::Code::LoadFailed,
        std::string("dlopen failed for: ") + path_ + (e ? (": " + std::string(e)) : "")
      };
    }
    return DynamicLibrary(h);
#endif
  }

  std::variant<void*, DlError> DynamicLibrary::LoadRawSymbol(const char* name_) const noexcept
  {
    assert(m_handle && "LoadRawSymbol called on moved-from DynamicLibrary");

    if (!name_ || !*name_)
      return DlError{ DlError::Code::InvalidArgument, "Empty symbol name" };

#if defined(_WIN32)
    FARPROC p = ::GetProcAddress(reinterpret_cast<HMODULE>(m_handle), name_);
    if (!p)
      return DlError{ DlError::Code::SymbolNotFound, std::string("GetProcAddress failed for symbol: ") + name_ };
    return reinterpret_cast<void*>(p);
#else
    ::dlerror(); // clear
    void* p = ::dlsym(m_handle, name_);
    if (const char* e = ::dlerror())
      return DlError{ DlError::Code::SymbolNotFound, std::string("dlsym failed for symbol: ") + name_ + ": " + e };
    if (!p)
      return DlError{ DlError::Code::SymbolNotFound, std::string("dlsym returned nullptr for symbol: ") + name_ };
    return p;
#endif
  }
}
