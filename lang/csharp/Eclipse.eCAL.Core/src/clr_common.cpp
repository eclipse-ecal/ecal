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

#include "clr_common.h"

#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Internal
{
  System::String^ StlStringToString(const std::string& ss_)
  {
    return msclr::interop::marshal_as<System::String^>(ss_);;
  }

  std::string StringToStlString(System::String^ s_)
  {
    if (s_ == nullptr) return std::string();
    return msclr::interop::marshal_as<std::string>(s_);
  }

  std::string ByteArrayToStlString(array<Byte>^ a_)
  {
    if (a_ == nullptr) return std::string();
    GCHandle handle = GCHandle::Alloc(a_, GCHandleType::Pinned);
    size_t len = a_->Length;
    std::string ret((const char*)(void*)handle.AddrOfPinnedObject(), len);
    handle.Free();
    return ret;
  }

  array<Byte>^ StlStringToByteArray(const std::string& ss_)
  {
    array<Byte>^ a = gcnew array<Byte>(static_cast<int>(ss_.size()));
    System::Runtime::InteropServices::Marshal::Copy(IntPtr((void*)(ss_.data())), a, 0, static_cast<int>(ss_.size()));
    return a;
  }
}