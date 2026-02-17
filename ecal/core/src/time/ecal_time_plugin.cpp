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

#include "ecal_time_plugin.h"

#include <array>
#include <sstream>

namespace
{
  eCAL::CTimePlugin::Error ToPluginError(const EcalUtils::DlError& e)
  {
    using PC = eCAL::CTimePlugin::Error::Code;
    using DC = EcalUtils::DlError::Code;

    PC mapped = PC::PlatformError;
    switch (e.code)
    {
    case DC::InvalidArgument: mapped = PC::InvalidArgument; break;
    case DC::LoadFailed:      mapped = PC::LoadFailed;      break;
    case DC::SymbolNotFound:  mapped = PC::IncompleteAPI;   break;
    default:                  mapped = PC::PlatformError;   break;
    }

    return eCAL::CTimePlugin::Error{ mapped, e.message };
  }

  struct MissingSymbol
  {
    const char* name;
    EcalUtils::DlError error;
  };

  using MissingSymbols = std::vector<MissingSymbol>;
  
  bool HasMissingSymbol(const MissingSymbols& missing_symbols)
  {
    return !missing_symbols.empty();
  }

  eCAL::CTimePlugin::Error GetError(const MissingSymbols& missing_symbols)
  {
    std::ostringstream oss;
    oss << "Incomplete eCAL time plugin API. Missing/failed symbols:";
    for (const auto& m : missing_symbols)
    {
      oss << "\n  - " << m.name << ": " << m.error.message;
    }
    return eCAL::CTimePlugin::Error{ eCAL::CTimePlugin::Error::Code::IncompleteAPI, oss.str() };
  }
}

std::variant<eCAL::CTimePlugin::Api, eCAL::CTimePlugin::Error>
eCAL::CTimePlugin::LoadApi(const EcalUtils::DynamicLibrary& lib) noexcept
{
  Api api;

  MissingSymbols missing_symbols;

  // Bind one symbol into a function pointer member, or record an error
  auto Bind = [&lib, &missing_symbols](auto& out_fn, const char* symbol_name)
    {
      using FunctionPointerT = std::decay_t<decltype(out_fn)>;

      auto function_pointer_or_error = lib.LoadSymbol<FunctionPointerT>(symbol_name);

      if (std::holds_alternative<FunctionPointerT>(function_pointer_or_error))
      {
        out_fn = std::get<FunctionPointerT>(function_pointer_or_error);
      }
      else
      {
        out_fn = nullptr;
        missing_symbols.push_back(MissingSymbol{ symbol_name, std::get<EcalUtils::DlError>(std::move(function_pointer_or_error)) });
      }
    };

  Bind(api.Initialize, "etime_initialize");
  Bind(api.Finalize, "etime_finalize");
  Bind(api.GetNanoSeconds, "etime_get_nanoseconds");
  Bind(api.SetNanoSeconds, "etime_set_nanoseconds");
  Bind(api.IsSynchronized, "etime_is_synchronized");
  Bind(api.IsMaster, "etime_is_master");
  Bind(api.SleepForNanoseconds, "etime_sleep_for_nanoseconds");
  Bind(api.GetStatus, "etime_get_status");

  if (HasMissingSymbol(missing_symbols))
  {
    return GetError(missing_symbols);
  }

  return api;
}

// In general, it follows the following steps, and continues if the previous step was successful.
// 1. Load the dynamic library. 
// 2. Load all symbols which are relevant for the time plugin
// 3. Initialize the time plugin
std::variant<eCAL::CTimePlugin, eCAL::CTimePlugin::Error> eCAL::CTimePlugin::LoadFromPath(std::string full_path) noexcept
{
  if (full_path.empty())
    return eCAL::CTimePlugin::Error{ eCAL::CTimePlugin::Error::Code::InvalidArgument, "Empty plugin path" };

  auto lib_res = EcalUtils::DynamicLibrary::Load(full_path);
  if (std::holds_alternative<EcalUtils::DlError>(lib_res))
    return ToPluginError(std::get<EcalUtils::DlError>(std::move(lib_res)));

  EcalUtils::DynamicLibrary lib = std::move(std::get<EcalUtils::DynamicLibrary>(lib_res));

  auto api_or_error = LoadApi(lib);
  if (std::holds_alternative<eCAL::CTimePlugin::Error>(api_or_error))
    return std::get<eCAL::CTimePlugin::Error>(std::move(api_or_error));

  const Api api = std::get<Api>(std::move(api_or_error));
  int plugin_initialization_result = api.Initialize();
  
  if (plugin_initialization_result != 0)
    return eCAL::CTimePlugin::Error{ eCAL::CTimePlugin::Error::Code::InitializationFailed, std::string{"Initialization of plugin failed with error code "} + std::to_string(plugin_initialization_result) };

  return eCAL::CTimePlugin(std::move(lib), api, std::move(full_path));
}

eCAL::CTimePlugin::CTimePlugin(EcalUtils::DynamicLibrary&& lib, Api api, std::string loaded_path) noexcept
  : loaded_library_(std::move(lib))
  , api_(api)
  , loaded_path_(std::move(loaded_path))
{
}

eCAL::CTimePlugin::CTimePlugin(CTimePlugin&& other) noexcept
  : loaded_library_(std::move(other.loaded_library_))
  , api_(std::move(other.api_))
  , loaded_path_(std::move(other.loaded_path_))
{
  // reset api on moved from object, so it may be destructed safely.
  other.api_ = Api{};
}

eCAL::CTimePlugin::~CTimePlugin()
{
  // If object was move-constructed, the API pointer will be nullptr.
  if (api_.Finalize != nullptr)
  {
    api_.Finalize();
  }
}

// ---- member wrappers ----
long long eCAL::CTimePlugin::GetNanoSeconds() const
{
  return api_.GetNanoSeconds();
}

bool eCAL::CTimePlugin::SetNanoSeconds(long long time_) const
{
  return api_.SetNanoSeconds(time_) == 0;
}

bool eCAL::CTimePlugin::IsSynchronized() const
{
  return api_.IsSynchronized() != 0;
}

bool eCAL::CTimePlugin::IsMaster() const
{
  return api_.IsMaster() != 0;
}

void eCAL::CTimePlugin::SleepForNanoseconds(long long duration_nsecs_) const
{
  api_.SleepForNanoseconds(duration_nsecs_);
}

void eCAL::CTimePlugin::GetStatus(int& error_, std::string* const status_message_) const
{
  static constexpr std::size_t buffer_len = 256;
  std::array<char, buffer_len> buffer{};
  buffer[0] = '\0';
  api_.GetStatus(&error_, buffer.data(), static_cast<int>(buffer.size()));
  buffer.back() = '\0';

  if (status_message_ != nullptr)
    status_message_->assign(buffer.data());
}
