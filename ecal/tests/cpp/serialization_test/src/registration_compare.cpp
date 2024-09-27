/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include "../../serialization/ecal_struct_sample_registration.h"

#include <algorithm>

namespace eCAL
{
  namespace Registration
  {
    // compare two ProcessState objects
    bool CompareProcessState(const ProcessState& state1, const ProcessState& state2)
    {
      return (state1.severity       == state2.severity) &&
             (state1.severity_level == state2.severity_level) &&
             (state1.info           == state2.info);
    }

    // compare two Process objects
    bool CompareProcess(const Process& process1, const Process& process2)
    {
      return (process1.rclock               == process2.rclock) &&
             (process1.hgname               == process2.hgname) &&
             (process1.pname                == process2.pname) &&
             (process1.uname                == process2.uname) &&
             (process1.pparam               == process2.pparam) &&
             CompareProcessState(process1.state, process2.state) &&
             (process1.tsync_state          == process2.tsync_state) &&
             (process1.tsync_mod_name       == process2.tsync_mod_name) &&
             (process1.component_init_state == process2.component_init_state) &&
             (process1.component_init_info  == process2.component_init_info) &&
             (process1.ecal_runtime_version == process2.ecal_runtime_version);
    }

    // compare two Service objects
    bool CompareService(const Service::Service& service1, const Service::Service& service2)
    {
      return (service1.rclock      == service2.rclock) &&
             (service1.pname       == service2.pname) &&
             (service1.uname       == service2.uname) &&
             (service1.sname       == service2.sname) &&
             (service1.methods     == service2.methods) &&
             (service1.version     == service2.version) &&
             (service1.tcp_port_v0 == service2.tcp_port_v0) &&
             (service1.tcp_port_v1 == service2.tcp_port_v1);
    }

    // compare two Client objects
    bool CompareClient(const Service::Client& client1, const Service::Client& client2)
    {
      return (client1.rclock  == client2.rclock) &&
             (client1.pname   == client2.pname) &&
             (client1.uname   == client2.uname) &&
             (client1.sname   == client2.sname) &&
             (client1.methods == client2.methods) &&
             (client1.version == client2.version);
    }

    // compare two DataTypeInformation objects
    bool CompareDataTypeInformation(const SDataTypeInformation& dt1, const SDataTypeInformation& dt2)
    {
      return (dt1.name == dt2.name) && (dt1.encoding == dt2.encoding) && (dt1.descriptor == dt2.descriptor);
    }

    // compare two LayerParUdpMC objects
    bool CompareLayerParUdpMC(const LayerParUdpMC& /*par1*/, const LayerParUdpMC& /*par2*/)
    {
      return true;
    }

    // compare two LayerParTcp objects
    bool CompareLayerParTcp(const LayerParTcp& par1, const LayerParTcp& par2)
    {
      return par1.port == par2.port;
    }

    // compare two LayerParShm objects
    bool CompareLayerParShm(const LayerParShm& par1, const LayerParShm& par2)
    {
      return par1.memory_file_list == par2.memory_file_list;
    }

    // compare two ConnectionPar objects
    bool CompareConnectionPar(const ConnectionPar& par1, const ConnectionPar& par2)
    {
      return CompareLayerParUdpMC(par1.layer_par_udpmc, par2.layer_par_udpmc) &&
             CompareLayerParTcp(par1.layer_par_tcp, par2.layer_par_tcp) &&
             CompareLayerParShm(par1.layer_par_shm, par2.layer_par_shm);
    }

    // compare two TLayer vector objects
    bool CompareTLayer(const Util::CExpandingVector<TLayer>& layer1_vec, const Util::CExpandingVector<TLayer>& layer2_vec)
    {
      // ensure that both vectors have the same size
      if (layer1_vec.size() != layer2_vec.size()) {
        return false;
      }

      // compare vectors element-wise
      return std::equal(layer1_vec.begin(), layer1_vec.end(), layer2_vec.begin(),
        [](const TLayer& layer1, const TLayer& layer2) {
          // compare TLayer objects for equality
          return (layer1.type      == layer2.type) &&
                 (layer1.version   == layer2.version) &&
                 (layer1.enabled   == layer2.enabled) &&
                 (layer1.active == layer2.active) &&
                 CompareConnectionPar(layer1.par_layer, layer2.par_layer);
        });
    }

    // compare two Topic objects
    bool CompareTopic(const Topic& topic1, const Topic& topic2)
    {
      return (topic1.rclock          == topic2.rclock) &&
             (topic1.hgname          == topic2.hgname) &&
             (topic1.pname           == topic2.pname) &&
             (topic1.uname           == topic2.uname) &&
             (topic1.tname           == topic2.tname) &&
             (topic1.direction       == topic2.direction) &&
             CompareDataTypeInformation(topic1.tdatatype, topic2.tdatatype) &&
             CompareTLayer(topic1.tlayer, topic2.tlayer) &&
             (topic1.tsize           == topic2.tsize) &&
             (topic1.connections_loc == topic2.connections_loc) &&
             (topic1.connections_ext == topic2.connections_ext) &&
             (topic1.message_drops   == topic2.message_drops) &&
             (topic1.did             == topic2.did) &&
             (topic1.dclock          == topic2.dclock) &&
             (topic1.dfreq           == topic2.dfreq) &&
             (topic1.attr            == topic2.attr);
    }

    bool CompareIdentifier(const SampleIdentifier& identifier1, const SampleIdentifier& identifier2)
    {
      return (identifier1.entity_id == identifier2.entity_id) &&
        (identifier1.process_id == identifier2.process_id) &&
        (identifier1.host_name == identifier2.host_name);
    }

    // compare two Registration Sample objects
    bool CompareRegistrationSamples(const Sample& sample1, const Sample& sample2)
    {
      return (sample1.cmd_type   == sample2.cmd_type) &&
             (sample1.host.hname == sample2.host.hname) &&
             CompareIdentifier(sample1.identifier, sample2.identifier) &&
             CompareProcess(sample1.process, sample2.process) &&
             CompareService(sample1.service, sample2.service) &&
             CompareClient(sample1.client, sample2.client) &&
             CompareTopic(sample1.topic, sample2.topic);
    }
  }
}
