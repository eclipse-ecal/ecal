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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/layer.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "io/ecal_memfile_pool.h"
#include "readwrite/ecal_reader_shm.h"

#include <iostream>

namespace eCAL
{
  template<> std::shared_ptr<CSHMLayer> CReaderLayer<eCAL::CSHMLayer>::layer(nullptr);

  void CSHMLayer::SetConnectionParameter(SReaderLayerPar& par_)
  {
    // list of memory file to register
    std::vector<std::string> memfile_names;

    // ----------------------------------------------------------------------
    // REMOVE ME IN VERSION 6
    // ----------------------------------------------------------------------
    // check for old behaviour
    bool              par_shm(false);
    const std::string par_shm_prefix("#PAR_SHM#");
    if (par_.parameter.size() > par_shm_prefix.size())
    {
      std::string prefix = par_.parameter.substr(0, par_shm_prefix.size());
      if (prefix == par_shm_prefix)
      {
        std::string memfile_name = par_.parameter.substr(par_shm_prefix.size(), par_.parameter.size());
        memfile_names.push_back(memfile_name);
        par_shm = true;
      }
    }
    // ----------------------------------------------------------------------
    // REMOVE ME IN VERSION 6
    // ----------------------------------------------------------------------

    if (!par_shm)
    {
      // new behaviour (eCAL version > 5.8.13/5.9.0)
      // layer parameter google protobuf message
      eCAL::pb::ConnnectionPar connection_par;
      if (connection_par.ParseFromString(par_.parameter))
      {
        for (auto memfile_name : connection_par.layer_par_shm().memory_file_list())
        {
          memfile_names.push_back(memfile_name);
        }
      }
      else
      {
        std::cout << "FATAL ERROR: Could not parse layer connection parameter ! Did you mix up different eCAL versions on the same host ?" << std::endl;
        return;
      }
    }

    for (auto memfile_name : memfile_names)
    {
      // start memory file receive thread if topic is subscribed in this process
      if (g_memfile_pool())
      {
        std::string process_id = std::to_string(Process::GetProcessID());
        std::string memfile_event = memfile_name + "_" + process_id;
        g_memfile_pool()->ObserveFile(memfile_name, memfile_event, par_.topic_name, par_.topic_id);
      }
    }
  }
}
