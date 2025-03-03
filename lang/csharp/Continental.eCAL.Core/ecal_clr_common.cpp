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

/**
 * @file  ecal_clr_common.cpp
**/

#include "ecal_clr_common.h"

using namespace System::Runtime::InteropServices;

namespace Internal
{
  System::String^ StlStringToString(const std::string& ss_)
  {
    String^ s = gcnew String(ss_.c_str(), 0, static_cast<int>(ss_.length()));
    return s;
  }

  std::string StringToStlString(System::String^ s_)
  {
    std::string s = std::string((const char*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(s_).ToPointer(), s_->Length);
    return s;
  }

  std::string ByteArrayToStlString(array<Byte>^ array_)
  {
    GCHandle handle = GCHandle::Alloc(array_, GCHandleType::Pinned);
    size_t len = array_->Length;
    std::string ret((const char*)(void*)handle.AddrOfPinnedObject(), len);
    handle.Free();
    return ret;
  }

  array<Byte>^ StlStringToByteArray(const std::string& string_)
  {
    array<Byte>^ array_ = gcnew array<Byte>(static_cast<int>(string_.size()));
    System::Runtime::InteropServices::Marshal::Copy(IntPtr((void*)(string_.data())), array_, 0, static_cast<int>(string_.size()));
    return array_;
  }
}