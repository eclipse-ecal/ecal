/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include <ecalsys/task/ecal_sys_task.h>
#include <sys_client_core/task.h>

namespace eCAL
{
  namespace sys
  {
    namespace task_helpers
    {
      eCAL::sys_client::Task                ToSysClientTask_NoLock           (const std::shared_ptr<EcalSysTask>& task);
      eCAL::sys_client::StartTaskParameters ToSysClientStartParameters_NoLock(const std::shared_ptr<EcalSysTask>& task);
      eCAL::sys_client::StopTaskParameters  ToSysClientStopParameters_NoLock (const std::shared_ptr<EcalSysTask>& task, bool by_name);
    }
  }
}