/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

#include <core/py_config.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

// Include the header that declares the eCAL functions and types.
#include "ecal/config.h"  // Adjust this include to your actual header file

namespace nb = nanobind;

void AddConfig(nb::module_& module)
{
  // we intentionally return a copy, not a reference.
  // Top-level functions in the eCAL namespace
  module.def("get_configuration",
    &eCAL::GetConfiguration,
    nb::rv_policy::copy,
    "Return the eCAL configuration.");

  // we intentionally return a copy, not a reference.
  module.def("get_subscriber_configuration",
    &eCAL::GetSubscriberConfiguration,
    nb::rv_policy::copy,
    "Return the eCAL subscriber configuration.");

  // we intentionally return a copy, not a reference.
  module.def("get_publisher_configuration",
    &eCAL::GetPublisherConfiguration,
    nb::rv_policy::copy,
    "Return the eCAL publisher configuration.");

  // Create a submodule for the functions in the eCAL::Config namespace.
  auto config = module.def_submodule("config", "eCAL configuration functions");

  // Common configuration functions
  config.def("get_loaded_ecal_ini_path",
    &eCAL::Config::GetLoadedEcalIniPath,
    "Return the path of the loaded eCAL ini file.");
  config.def("get_registration_timeout_ms",
    &eCAL::Config::GetRegistrationTimeoutMs,
    "Return the registration timeout in milliseconds.");
  config.def("get_registration_refresh_ms",
    &eCAL::Config::GetRegistrationRefreshMs,
    "Return the registration refresh interval in milliseconds.");

  // Network-related configuration
  config.def("is_network_enabled",
    &eCAL::Config::IsNetworkEnabled,
    "Return True if the network is enabled.");
  config.def("is_shm_registration_enabled",
    &eCAL::Config::IsShmRegistrationEnabled,
    "Return True if shared memory registration is enabled.");
  config.def("get_udp_multicast_config_version",
    &eCAL::Config::GetUdpMulticastConfigVersion,
    "Return the UDP multicast configuration version.");
  config.def("get_udp_multicast_group",
    &eCAL::Config::GetUdpMulticastGroup,
    "Return the UDP multicast group.");
  config.def("get_udp_multicast_mask",
    &eCAL::Config::GetUdpMulticastMask,
    "Return the UDP multicast mask.");
  config.def("get_udp_multicast_port",
    &eCAL::Config::GetUdpMulticastPort,
    "Return the UDP multicast port.");
  config.def("get_udp_multicast_ttl",
    &eCAL::Config::GetUdpMulticastTtl,
    "Return the UDP multicast time-to-live.");
  config.def("get_udp_multicast_snd_buf_size_bytes",
    &eCAL::Config::GetUdpMulticastSndBufSizeBytes,
    "Return the UDP multicast send buffer size in bytes.");
  config.def("get_udp_multicast_rcv_buf_size_bytes",
    &eCAL::Config::GetUdpMulticastRcvBufSizeBytes,
    "Return the UDP multicast receive buffer size in bytes.");
  config.def("is_udp_multicast_join_all_if_enabled",
    &eCAL::Config::IsUdpMulticastJoinAllIfEnabled,
    "Return True if joining all interfaces for UDP multicast is enabled.");
  config.def("is_udp_multicast_rec_enabled",
    &eCAL::Config::IsUdpMulticastRecEnabled,
    "Return True if the UDP multicast receiver is enabled.");
  config.def("is_shm_rec_enabled",
    &eCAL::Config::IsShmRecEnabled,
    "Return True if the shared memory receiver is enabled.");
  config.def("is_tcp_rec_enabled",
    &eCAL::Config::IsTcpRecEnabled,
    "Return True if the TCP receiver is enabled.");
  config.def("is_npcap_enabled",
    &eCAL::Config::IsNpcapEnabled,
    "Return True if NPCAP is enabled.");
  config.def("get_tcp_pubsub_reader_threadpool_size",
    &eCAL::Config::GetTcpPubsubReaderThreadpoolSize,
    "Return the TCP pub/sub reader thread pool size.");
  config.def("get_tcp_pubsub_writer_threadpool_size",
    &eCAL::Config::GetTcpPubsubWriterThreadpoolSize,
    "Return the TCP pub/sub writer thread pool size.");
  config.def("get_tcp_pubsub_max_reconnection_attempts",
    &eCAL::Config::GetTcpPubsubMaxReconnectionAttemps,
    "Return the maximum number of TCP pub/sub reconnection attempts.");
  config.def("get_shm_transport_domain",
    &eCAL::Config::GetShmTransportDomain,
    "Return the shared memory transport domain.");

  // Time configuration
  config.def("get_timesync_module_name",
    &eCAL::Config::GetTimesyncModuleName,
    "Return the timesync module name.");
  config.def("get_timesync_module_replay",
    &eCAL::Config::GetTimesyncModuleReplay,
    "Return the timesync module replay configuration.");

  // Process configuration
  config.def("get_terminal_emulator_command",
    &eCAL::Config::GetTerminalEmulatorCommand,
    "Return the terminal emulator command.");

  // System configuration
  config.def("get_ecal_sys_filter_exclude_list",
    &eCAL::Config::GetEcalSysFilterExcludeList,
    "Return the eCAL system filter exclude list.");

  // Subscriber configuration
  config.def("get_drop_out_of_order_messages",
    &eCAL::Config::GetDropOutOfOrderMessages,
    "Return True if dropping out-of-order messages is enabled.");

  // Registration configuration
  config.def("get_shm_monitoring_queue_size",
    &eCAL::Config::GetShmMonitoringQueueSize,
    "Return the shared memory monitoring queue size.");
  config.def("get_shm_monitoring_domain",
    &eCAL::Config::GetShmMonitoringDomain,
    "Return the shared memory monitoring domain.");
}
