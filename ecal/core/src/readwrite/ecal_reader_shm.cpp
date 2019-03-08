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
 * @brief  memory file data reader
**/

#include <ecal/ecal.h>

#include "io/ecal_memfile_pool.h"
#include "readwrite/ecal_reader_shm.h"

namespace eCAL
{
  template<> std::shared_ptr<CSHMLayer> CReaderLayer<eCAL::CSHMLayer>::layer(nullptr);

  CDataReaderSHM::CDataReaderSHM()
  {
  }
  
  CDataReaderSHM::~CDataReaderSHM()
  {
  }

  void CSHMLayer::ApplyLayerParameter(SReaderLayerPar& par_)
  {
    std::string memfile_name = par_.parameter;
    if (!memfile_name.empty())
    {
      // start memory file receive thread if topic is subscribed in this process
      if (g_memfile_pool())
      {
        std::string process_id = std::to_string(Process::GetProcessID());
        std::string memfile_event = memfile_name + "_" + process_id;
        g_memfile_pool()->AssignThread(par_.topic_id, memfile_event, memfile_name, par_.topic_name);
      }
    }
  }
}
