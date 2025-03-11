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

#include "../include/mma.h"
#include "ecal/app/pb/mma/mma.pb.h"

#ifdef _WIN32
#include "../include/windows/mma_windows.h"
#endif // _WIN32

#ifdef __unix__
#include "linux/mma_linux.h"
#endif // __unix__

MMA::MMA()
{
#ifdef _WIN32
  pImpl = new MMAWindows();
#endif // _WIN32

#ifdef __unix__
  pImpl = new MMALinux();
#endif // __unix__
}

MMA::~MMA()
{
  if (pImpl != nullptr)
  {
    delete pImpl;
  }
}

bool MMA::Get(eCAL::pb::mma::State& state)
{
 return pImpl->Get(state);
}
