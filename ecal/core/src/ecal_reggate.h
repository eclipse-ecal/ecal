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
 * @brief  eCAL registration gateway class
**/

#pragma once

#include "ecal_global_accessors.h"

#include <ecal/ecal.h>

#include "ecal_def.h"
#include "ecal_thread.h"

#include "io/rcv_sample.h"

#include <string>
#include <atomic>


namespace eCAL
{
  class CUdpRegistrationReceiver : public CSampleReceiver
  {
    bool HasSample(const std::string& sample_name_);
    size_t ApplySample(const eCAL::pb::Sample& ecal_sample_, eCAL::pb::eTLayerType layer_);
  };

  class CRegGate
  {
  public:
    CRegGate();
    ~CRegGate();

    void Create();
    void Destroy();

    void EnableLoopback(bool state_);
    bool LoopBackEnabled() { return m_loopback; };

    bool HasSample(const std::string& /* sample_name_ */) {return(true);};
    size_t ApplySample(const eCAL::pb::Sample& ecal_sample_);

    bool AddRegistrationCallback(enum eCAL_Registration_Event event_, RegistrationCallbackT callback_);
    bool RemRegistrationCallback(enum eCAL_Registration_Event event_);

  protected:
    bool IsLocalHost(const eCAL::pb::Sample & ecal_sample_);

    static std::atomic<bool>  m_created;
    bool                      m_network;
    bool                      m_loopback;

    RegistrationCallbackT     m_callback_pub;
    RegistrationCallbackT     m_callback_sub;
    RegistrationCallbackT     m_callback_service;
    RegistrationCallbackT     m_callback_client;
    RegistrationCallbackT     m_callback_process;

    CUDPReceiver              m_reg_rcv;
    CThread                   m_reg_rcv_thread;
    CUdpRegistrationReceiver  m_reg_rcv_process;
  };
};
