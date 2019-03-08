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

#include "udpcap/npcap_helpers.h"

#include <mutex>

#include <tchar.h>
#include <iostream>
#include <algorithm>

#include <locale>
#include <codecvt>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Udpcap
{
  namespace // Private Namespace
  {
    static std::mutex npcap_mutex;
    static bool is_initialized(false);

    static std::string loopback_device_uuid_string;
    static bool loopback_device_name_initialized(false);

    bool LoadNpcapDlls()
    {
      _TCHAR npcap_dir[512];
      UINT len;
      len = GetSystemDirectory(npcap_dir, 480);
      if (!len) {
        fprintf(stderr, "Error in GetSystemDirectory: %x", GetLastError());
        return false;
      }
      _tcscat_s(npcap_dir, 512, _T("\\Npcap"));
      if (SetDllDirectory(npcap_dir) == 0) {
        fprintf(stderr, "Error in SetDllDirectory: %x", GetLastError());
        return false;
      }
      return true;
    }

    LONG GetDWORDRegKey(HKEY hKey, const std::wstring &strValueName, DWORD &nValue, DWORD nDefaultValue)
    {
      nValue = nDefaultValue;
      DWORD dwBufferSize(sizeof(DWORD));
      DWORD nResult(0);
      LONG nError = ::RegQueryValueExW(hKey,
        strValueName.c_str(),
        0,
        NULL,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
      if (ERROR_SUCCESS == nError)
      {
        nValue = nResult;
      }
      return nError;
    }


    LONG GetBoolRegKey(HKEY hKey, const std::wstring &strValueName, bool &bValue, bool bDefaultValue)
    {
      DWORD nDefValue((bDefaultValue) ? 1 : 0);
      DWORD nResult(nDefValue);
      LONG nError = GetDWORDRegKey(hKey, strValueName, nResult, nDefValue);
      if (ERROR_SUCCESS == nError)
      {
        bValue = (nResult != 0) ? true : false;
      }
      return nError;
    }


    LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue)
    {
      strValue = strDefaultValue;
      WCHAR szBuffer[512];
      DWORD dwBufferSize = sizeof(szBuffer);
      ULONG nError;
      nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
      if (ERROR_SUCCESS == nError)
      {
        strValue = szBuffer;
      }
      return nError;
    }

    bool LoadLoopbackDeviceNameFromRegistry()
    {
      HKEY hkey;
      LONG error_code = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\npcap\\Parameters", 0, KEY_READ, &hkey);
      if (error_code)
      {
        std::cerr << "Udpcap ERROR: NPCAP doesn't seem to be installed. Please download and install Npcap from https://nmap.org/npcap/#download" << std::endl;
        return false;
      }

      bool loopback_supported = false;
      GetBoolRegKey(hkey, L"LoopbackSupport", loopback_supported, false);

      if (!loopback_supported)
      {
        std::cerr << "Udpcap ERROR: NPCAP was installed without loopback support. Please re-install NPCAP" << std::endl;
        RegCloseKey(hkey);
        return false;
      }

      std::wstring loopback_device_name_w;
      GetStringRegKey(hkey, L"LoopbackAdapter", loopback_device_name_w, L"");

      if (loopback_device_name_w.empty())
      {
        std::cerr << "Udpcap ERROR: unable to retrieve NPCAP Loopback adapter name. Please reinstall Npcap:" << std::endl;
        std::cerr << "    1) Uninstall Npcap" << std::endl;
        std::cerr << "    2) Uninstall all \"Npcap Loopback Adapters\" from the device manager" << std::endl;
        std::cerr << "    3) Uninstall all \"Microsoft KM_TEST Loopback Adapters\" from the device manager" << std::endl;
        std::cerr << "    4) Install Npcap again" << std::endl;
        RegCloseKey(hkey);
        return false;
      }

      RegCloseKey(hkey);

      std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
      std::string loopback_device_name = converter.to_bytes(loopback_device_name_w);

      // The Registry entry is in the form: \Device\{6DBF8591-55F9-4DEF-A317-54B9563A42E3}
      // We however only want the UUID:              6DBF8591-55F9-4DEF-A317-54B9563A42E3
      size_t open_bracket_pos    = loopback_device_name.find('{');
      size_t closing_bracket_pos = loopback_device_name.find('}', open_bracket_pos);
      loopback_device_uuid_string = loopback_device_name.substr(open_bracket_pos + 1, closing_bracket_pos - open_bracket_pos - 1);

      return true;
    }
  }

  bool Initialize()
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);

    if (is_initialized) return true;

    std::cout << "Udpcap: Initializing Npcap..." << std::endl;

    if (!LoadLoopbackDeviceNameFromRegistry())
    {
      return false;
    }

    std::cout << "Udpcap: Using Loopback device " << loopback_device_uuid_string << std::endl;

    if (!LoadNpcapDlls())
    {
      std::cerr << "Udpcap ERROR: Unable to load Npcap. Please download and install Npcap from https://nmap.org/npcap/#download" << std::endl;
      return false;
    }

    std::cout << "Udpcap: Npcap is ready" << std::endl;

    is_initialized = true;
    return true;
  }

  bool IsInitialized()
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);
    return is_initialized;
  }

  std::string GetLoopbackDeviceUuidString()
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);

    if (!loopback_device_name_initialized)
    {
      loopback_device_name_initialized = LoadLoopbackDeviceNameFromRegistry();
    }

    return loopback_device_uuid_string;
  }

  std::string GetLoopbackDeviceName()
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);

    if (!loopback_device_name_initialized)
    {
      loopback_device_name_initialized = LoadLoopbackDeviceNameFromRegistry();
    }
    return "\\Device\\NPCAP_{" + loopback_device_uuid_string + "}";
  }

  bool IsLoopbackDevice(const std::string& device_name_or_uuid_string)
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);

    if (!loopback_device_name_initialized)
    {
      loopback_device_name_initialized = LoadLoopbackDeviceNameFromRegistry();
    }

    if (!loopback_device_name_initialized)
    {
      return false;
    }
    else
    {
      // Extract the UUID from the input, as it might be an entire device path
      size_t open_bracket_pos = device_name_or_uuid_string.find('{');
      size_t closing_bracket_pos = device_name_or_uuid_string.find('}');

      std::string given_uuid;

      if ((open_bracket_pos == std::string::npos) || (closing_bracket_pos == std::string::npos))
      {
        given_uuid = device_name_or_uuid_string;
      }
      else
      {
        given_uuid = device_name_or_uuid_string.substr(open_bracket_pos + 1, closing_bracket_pos - open_bracket_pos - 1);
      }

      // Lower-case everything	  
	  //std::transform(given_uuid.begin(), given_uuid.end(), given_uuid.begin(), ::tolower); // cause warning C4244 with VS2017
	  std::transform(given_uuid.begin(), given_uuid.end(), given_uuid.begin(),
		  [](char c) {return static_cast<char>(::tolower(c)); });
      std::string loopback_uuid_lower = loopback_device_uuid_string;
	  //std::transform(loopback_uuid_lower.begin(), loopback_uuid_lower.end(), loopback_uuid_lower.begin(), ::tolower); // cause warning C4244 with VS2017
	  std::transform(loopback_uuid_lower.begin(), loopback_uuid_lower.end(), loopback_uuid_lower.begin(),
		  [](char c) {return static_cast<char>(::tolower(c)); });

      // String compare
      return (loopback_uuid_lower == given_uuid);
    }
  }
}