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

/**
 * @brief  eCAL memory file mutex
**/

#pragma once

#include <ecal/ecal_os.h>
#include <string>

#ifdef ECAL_OS_WINDOWS
#include "win32/ecal_memfile_mtx.h"
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
#include "linux/ecal_memfile_mtx.h"
#endif /* ECAL_OS_LINUX */
