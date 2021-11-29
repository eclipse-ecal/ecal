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
#include <sstream>

#include <locale>
#include <codecvt>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <pcap/pcap.h>

namespace Udpcap
{
  namespace // Private Namespace
  {
    static std::mutex npcap_mutex;
    static bool is_initialized(false);

    static std::string loopback_device_uuid_string;
    static bool loopback_device_name_initialized(false);

    static std::string human_readible_error_("Npcap has not been initialized, yet");

    bool LoadNpcapDlls()
    {
      _TCHAR npcap_dir[512];
      UINT len;
      len = GetSystemDirectory(npcap_dir, 480);
      if (!len) {
        human_readible_error_ = "Error in GetSystemDirectory";
        fprintf(stderr, "Error in GetSystemDirectory: %x", GetLastError());
        return false;
      }
      _tcscat_s(npcap_dir, 512, _T("\\Npcap"));
      if (SetDllDirectory(npcap_dir) == 0) {
        human_readible_error_ = "Error in SetDllDirectory";
        fprintf(stderr, "Error in SetDllDirectory: %x", GetLastError());
        return false;
      }

      if(LoadLibrary("wpcap.dll") == NULL)
        return false;

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
        human_readible_error_ = "NPCAP doesn't seem to be installed. Please download and install Npcap from https://nmap.org/npcap/#download";
        std::cerr << "Udpcap ERROR: " << human_readible_error_ << std::endl;
        return false;
      }

      bool loopback_supported = false;
      GetBoolRegKey(hkey, L"LoopbackSupport", loopback_supported, false);

      if (!loopback_supported)
      {
        human_readible_error_ = "NPCAP was installed without loopback support. Please re-install NPCAP";
        std::cerr << "Udpcap ERROR: " << human_readible_error_ << std::endl;
        RegCloseKey(hkey);
        return false;
      }

      std::wstring loopback_device_name_w;
      GetStringRegKey(hkey, L"LoopbackAdapter", loopback_device_name_w, L"");

      //if (loopback_device_name_w.empty())
      //{
      //  std::stringstream error_ss;

      //  error_ss << "Unable to retrieve NPCAP Loopback adapter name. Please reinstall Npcap:" << std::endl;
      //  error_ss << "    1) Uninstall Npcap" << std::endl;
      //  error_ss << "    2) Uninstall all \"Npcap Loopback Adapters\" from the device manager" << std::endl;
      //  error_ss << "    3) Uninstall all \"Microsoft KM_TEST Loopback Adapters\" from the device manager" << std::endl;
      //  error_ss << "    4) Install Npcap again";

      //  human_readible_error_ = error_ss.str();

      //  std::cerr << "Udpcap ERROR: " << human_readible_error_ << std::endl;

      //  RegCloseKey(hkey);
      //  return false;
      //}

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

    bool IsLoopbackDevice_NoLock(const std::string& device_name)
    {
      if (!loopback_device_name_initialized)
      {
        loopback_device_name_initialized = LoadLoopbackDeviceNameFromRegistry();
      }

      std::string lower_given_device_name    = device_name;
      std::string lower_loopback_device_name;

      if (!loopback_device_uuid_string.empty())
        lower_loopback_device_name = std::string("\\device\\{" + loopback_device_uuid_string + "}");
      else
        lower_loopback_device_name = "\\device\\npf_loopback";

      std::transform(lower_given_device_name.begin(), lower_given_device_name.end(), lower_given_device_name.begin(),
                    [](char c)
                    {return static_cast<char>(::tolower(c)); });

      std::transform(lower_loopback_device_name.begin(), lower_loopback_device_name.end(), lower_loopback_device_name.begin(),
                    [](char c)
                    {return static_cast<char>(::tolower(c)); });

      // At some point between NPCAP 0.9996 and NPCAP 1.10 the loopback device
      // was renamed to "\device\npf_loopback".
      // For newer NPCAP versions the complicated method to get the Device
      // UUID is obsolete. However, we leave the code in place, as it works
      // and still provides downwards compatibility to older NPCAP versions.
      return (lower_given_device_name == "\\device\\npf_loopback") || (lower_loopback_device_name == lower_given_device_name);
    }

    bool TestLoopbackDevice()
    {
      typedef std::unique_ptr<pcap_if_t*, void(*)(pcap_if_t**)> pcap_if_t_uniqueptr;

      char errbuf[PCAP_ERRBUF_SIZE];
      pcap_if_t* alldevs_rawptr;
      pcap_if_t_uniqueptr alldevs(&alldevs_rawptr, [](pcap_if_t** p) { pcap_freealldevs(*p); });

      bool loopback_device_found = false;

      if (pcap_findalldevs(alldevs.get(), errbuf) == -1)
      {
        human_readible_error_ = "Error in pcap_findalldevs: " + std::string(errbuf);
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        return false;
      }

      // Check if the loopback device is accessible
      for (pcap_if_t* pcap_dev = *alldevs.get(); pcap_dev; pcap_dev = pcap_dev->next)
      {
        if (IsLoopbackDevice_NoLock(pcap_dev->name))
        {
          loopback_device_found = true;
        }
      }

      // if we didn't find the loopback device, the test has failed
      if (!loopback_device_found)
      {
        std::stringstream error_ss;

        error_ss << "Udpcap ERROR: Loopback adapter is inaccessible. On some systems the Npcap driver fails to start properly. Please open a command prompt with administrative privileges and run the following commands:" << std::endl;
        error_ss << "    When npcap was installed in normal mode:" << std::endl;
        error_ss << "       > sc stop npcap" << std::endl;
        error_ss << "       > sc start npcap" << std::endl;
        error_ss << "    When npcap was installed in WinPcap compatible mode:" << std::endl;
        error_ss << "       > sc stop npf" << std::endl;
        error_ss << "       > sc start npf";

        human_readible_error_ = error_ss.str();

        std::cerr << "Udpcap ERROR: " << human_readible_error_ << std::endl;

        return false;
      }

      return true;
    }
  }

  bool Initialize()
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);

    if (is_initialized) return true;

    human_readible_error_ = "Unknown error";

    std::cout << "Udpcap: Initializing Npcap..." << std::endl;

    LoadLoopbackDeviceNameFromRegistry();
    // Don't return false, as modern NPCAP will work without the registry key
    //if (!LoadLoopbackDeviceNameFromRegistry())
    //{
    //  return false;
    //}

    if (!loopback_device_uuid_string.empty())
      std::cout << "Udpcap: Using Loopback device " << loopback_device_uuid_string << std::endl;
    else
      std::cout << "Udpcap: Using Loopback device \\device\\npf_loopback" << std::endl;

    if (!LoadNpcapDlls())
    {
      std::cerr << "Udpcap ERROR: Unable to load Npcap. Please download and install Npcap from https://nmap.org/npcap/#download" << std::endl;
      return false;
    }

    if (!TestLoopbackDevice())
    {
      return false;
    }

    human_readible_error_ = "Npcap is ready";
    std::cout << "Udpcap: " << human_readible_error_ << std::endl;

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
      LoadLoopbackDeviceNameFromRegistry();
      loopback_device_name_initialized = true;    // Even when we were not able to read the loopback device name, we assume it is present, as recent NPCAP versions don't create the specific adapter any more.
    }

    if (!loopback_device_uuid_string.empty())
      return "\\device\\npcap_{" + loopback_device_uuid_string + "}";
    else
      return "\\device\\npf_loopback";
  }

  bool IsLoopbackDevice(const std::string& device_name)
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);
    return IsLoopbackDevice_NoLock(device_name);
  }

  std::string GetHumanReadibleErrorText()
  {
    std::lock_guard<std::mutex> npcap_lock(npcap_mutex);
    return human_readible_error_;
  }
}