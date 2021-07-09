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

#include "udpcap_socket_private.h"

#include "udpcap/npcap_helpers.h"
#include "log_debug.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <ntddndis.h>       // User-space defines for NDIS driver communication

#include <sstream>
#include <iostream>
#include <algorithm>

#include <asio.hpp>

namespace Udpcap
{
  //////////////////////////////////////////
  //// Socket API
  //////////////////////////////////////////

  UdpcapSocketPrivate::UdpcapSocketPrivate()
    : is_valid_(false)
    , bound_state_(false)
    , bound_port_(0)
    , multicast_loopback_enabled_(true)
    , receive_buffer_size_(-1)
  {
    is_valid_ = Udpcap::Initialize();
  }

  UdpcapSocketPrivate::~UdpcapSocketPrivate()
  {
    // @todo: reinvestigate why it crashes on close. (Maybe check if i have implemented copy / move constructors properly)
    //close();
  }

  bool UdpcapSocketPrivate::isValid() const
  {
    return is_valid_;
  }


  bool UdpcapSocketPrivate::bind(const HostAddress& local_address, uint16_t local_port)
  {
    if (!is_valid_)
    {
      // Invalid socket, cannot bind => fail!
      LOG_DEBUG("Bind error: Socket is invalid");
      return false;
    }

    if (bound_state_)
    {
      // Already bound => fail!
      LOG_DEBUG("Bind error: Socket is already in bound state");
      return false;
    }

    if (!local_address.isValid())
    {
      // Invalid address => fail!
      LOG_DEBUG("Bind error: Host address is invalid");
      return false;
    }


    // Valid address => Try to bind to address!
    
    if (local_address.isLoopback())
    {
      // Bind to localhost (We cannot find it by IP 127.0.0.1, as that IP is technically not even assignable to the loopback adapter).
      LOG_DEBUG(std::string("Opening Loopback device ") + GetLoopbackDeviceName());

      if (!openPcapDevice(GetLoopbackDeviceName()))
      {
        LOG_DEBUG(std::string("Bind error: Unable to bind to ") + GetLoopbackDeviceName());
        close();
        return false;
      }
    }
    else if (local_address == HostAddress::Any())
    {
      // Bind to all adapters
      auto devices = getAllDevices();

      if (devices.empty())
      {
        LOG_DEBUG("Bind error: No devices found");
        close();
        return false;
      }

      for (const auto& dev : devices)
      {
        LOG_DEBUG(std::string("Opening ") + dev.first + " (" + dev.second + ")");

        if (!openPcapDevice(dev.first))
        {
          LOG_DEBUG(std::string("Bind error: Unable to bind to ") + dev.first);
        }
      }
    }
    else
    {
      // Bind to adapter specified by the IP address
      auto dev = getDeviceByIp(local_address);

      if (dev.first.empty())
      {
        LOG_DEBUG("Bind error: No local device with address " + local_address.toString());
        close();
        return false;
      }
      
      LOG_DEBUG(std::string("Opening ") + dev.first + " (" + dev.second + ")");

      if (!openPcapDevice(dev.first))
      {
        LOG_DEBUG(std::string("Bind error: Unable to bind to ") + dev.first);
        close();
        return false;
      }

      // Also open loopback adapter. We always have to expect the local machine sending data to its own IP address.
      LOG_DEBUG(std::string("Opening Loopback device ") + GetLoopbackDeviceName());

      if (!openPcapDevice(GetLoopbackDeviceName()))
      {
        LOG_DEBUG(std::string("Bind error: Unable to open ") + GetLoopbackDeviceName());
        close();
        return false;
      }
    }
    
    bound_address_ = local_address;
    bound_port_    = local_port;
    bound_state_   = true;

    for (auto& pcap_dev : pcap_devices_)
    {
      updateCaptureFilter(pcap_dev);
    }

    return true;
  }

  bool UdpcapSocketPrivate::isBound() const
  {
    return bound_state_;
  }

  HostAddress UdpcapSocketPrivate::localAddress() const
  {
    return bound_address_;
  }

  uint16_t UdpcapSocketPrivate::localPort() const
  {
    return bound_port_;
  }

  bool UdpcapSocketPrivate::setReceiveBufferSize(int buffer_size)
  {
    if (!is_valid_)
    {
      // Invalid socket, cannot bind => fail!
      LOG_DEBUG("Set Receive Buffer Size error: Socket is invalid");
      return false;
    }

    if (bound_state_)
    {
      // Not bound => fail!
      LOG_DEBUG("Set Receive Buffer Size error: Socket is already bound");
      return false;
    }

    if (buffer_size < MAX_PACKET_SIZE)
    {
      // Not bound => fail!
      LOG_DEBUG("Set Receive Buffer Size error: Buffer size is smaller than the maximum expected packet size (" + std::to_string(MAX_PACKET_SIZE) + ")");
      return false;
    }

    receive_buffer_size_ = buffer_size;

    return true;
  }

  bool UdpcapSocketPrivate::hasPendingDatagrams() const
  {
    if (!is_valid_)
    {
      // Invalid socket, cannot bind => fail!
      LOG_DEBUG("Has Pending Datagrams error: Socket is invalid");
      return false;
    }

    if (!bound_state_)
    {
      // Not bound => fail!
      LOG_DEBUG("Has Pending Datagrams error: Socket is not bound");
      return false;
    }

    if (pcap_win32_handles_.size() < 1)
    {
      // No open devices => fail!
      LOG_DEBUG("Has Pending Datagrams error: No open devices");
      return false;
    }

    // Wait 0 ms for data
    DWORD num_handles = static_cast<DWORD>(pcap_win32_handles_.size());
    if (num_handles > MAXIMUM_WAIT_OBJECTS)
    {
      LOG_DEBUG("WARNING: Too many open Adapters. " + std::to_string(num_handles) + " adapters are open, only " + std::to_string(MAXIMUM_WAIT_OBJECTS) + " are supported.");
      num_handles = MAXIMUM_WAIT_OBJECTS;
    }

    DWORD wait_result = WaitForMultipleObjects(num_handles, &pcap_win32_handles_[0], false, 0);

    // Check if any HADNLE was in signaled state
    return((wait_result >= WAIT_OBJECT_0) && wait_result <= (WAIT_OBJECT_0 + num_handles - 1));
  }


  std::vector<char> UdpcapSocketPrivate::receiveDatagram(HostAddress* source_address, uint16_t* source_port)
  {
    return receiveDatagram(INFINITE, source_address, source_port);
  }

  std::vector<char> UdpcapSocketPrivate::receiveDatagram(unsigned long timeout_ms, HostAddress* source_address, uint16_t* source_port)
  {
    if (!is_valid_)
    {
      // Invalid socket, cannot bind => fail!
      LOG_DEBUG("Receive error: Socket is invalid");
      return {};
    }

    if (!bound_state_)
    {
      // Not bound => fail!
      LOG_DEBUG("Receive error: Socket is not bound");
      return{};
    }

    if (pcap_win32_handles_.size() < 1)
    {
      // No open devices => fail!
      LOG_DEBUG("Receive error: No open devices");
      return{};
    }

    DWORD num_handles = static_cast<DWORD>(pcap_win32_handles_.size());
    if (num_handles > MAXIMUM_WAIT_OBJECTS)
    {
      LOG_DEBUG("WARNING: Too many open Adapters. " + std::to_string(num_handles) + " adapters are open, only " + std::to_string(MAXIMUM_WAIT_OBJECTS) + " are supported.");
      num_handles = MAXIMUM_WAIT_OBJECTS;
    }

    bool wait_forever = (timeout_ms == INFINITE);
    auto wait_until = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);

    std::vector<char> datagram;
    CallbackArgsVector callback_args(&datagram, source_address, source_port, bound_port_, pcpp::LinkLayerType::LINKTYPE_NULL);

    do
    {
      unsigned long remaining_time_to_wait_ms = 0;
      if (wait_forever)
      {
        remaining_time_to_wait_ms = INFINITE;
      }
      else
      {
        auto now = std::chrono::steady_clock::now();
        if (now < wait_until)
        {
          remaining_time_to_wait_ms = static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(wait_until - now).count());
        }
      }

      DWORD wait_result = WaitForMultipleObjects(num_handles, &pcap_win32_handles_[0], false, remaining_time_to_wait_ms);

      if ((wait_result >= WAIT_OBJECT_0) && wait_result <= (WAIT_OBJECT_0 + num_handles - 1))
      {
        int dev_index = (wait_result - WAIT_OBJECT_0);
        
        callback_args.link_type_     = static_cast<pcpp::LinkLayerType>(pcap_datalink(pcap_devices_[dev_index].pcap_handle_));
        callback_args.ip_reassembly_ = ip_reassembly_[dev_index].get();

        pcap_dispatch(pcap_devices_[dev_index].pcap_handle_, 1, UdpcapSocketPrivate::PacketHandlerVector, reinterpret_cast<u_char*>(&callback_args));

        if (callback_args.success_)
        {
          // Only return datagram if we successfully received a packet. Otherwise, we will continue receiving data, if there is time left.
          return datagram;
        }
      }
      else if ((wait_result >= WAIT_ABANDONED_0) && wait_result <= (WAIT_ABANDONED_0 + num_handles - 1))
      {
        LOG_DEBUG("Receive error: WAIT_ABANDONED");
      }
      else if (wait_result == WAIT_TIMEOUT)
      {
        // LOG_DEBUG("Receive error: WAIT_TIMEOUT");
      }
      else if (wait_result == WAIT_FAILED)
      {
        LOG_DEBUG("Receive error: WAIT_FAILED: " + std::to_string(GetLastError()));
      }
    } while (wait_forever || (std::chrono::steady_clock::now() < wait_until));

    return{};
  }

  size_t UdpcapSocketPrivate::receiveDatagram(char* data, size_t max_len, HostAddress* source_address, uint16_t* source_port)
  {
    return receiveDatagram(data, max_len, INFINITE, source_address, source_port);
  }

  size_t UdpcapSocketPrivate::receiveDatagram(char* data, size_t max_len, unsigned long timeout_ms, HostAddress* source_address, uint16_t* source_port)
  {
    if (!is_valid_)
    {
      // Invalid socket, cannot bind => fail!
      LOG_DEBUG("Receive error: Socket is invalid");
      return{};
    }

    if (!bound_state_)
    {
      // Not bound => fail!
      LOG_DEBUG("Receive error: Socket is not bound");
      return{};
    }

    if (pcap_win32_handles_.size() < 1)
    {
      // No open devices => fail!
      LOG_DEBUG("Receive error: No open devices");
      return{};
    }

    DWORD num_handles = static_cast<DWORD>(pcap_win32_handles_.size());
    if (num_handles > MAXIMUM_WAIT_OBJECTS)
    {
      LOG_DEBUG("WARNING: Too many open Adapters. " + std::to_string(num_handles) + " adapters are open, only " + std::to_string(MAXIMUM_WAIT_OBJECTS) + " are supported.");
      num_handles = MAXIMUM_WAIT_OBJECTS;
    }

    bool wait_forever = (timeout_ms == INFINITE);
    auto wait_until = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);

    CallbackArgsRawPtr callback_args(data, max_len, source_address, source_port, bound_port_, pcpp::LinkLayerType::LINKTYPE_NULL);

    do
    {
      unsigned long remaining_time_to_wait_ms = 0;
      if (wait_forever)
      {
        remaining_time_to_wait_ms = INFINITE;
      }
      else
      {
        auto now = std::chrono::steady_clock::now();
        if (now < wait_until)
        {
          remaining_time_to_wait_ms = static_cast<unsigned long>(std::chrono::duration_cast<std::chrono::milliseconds>(wait_until - now).count());
        }
      }

      DWORD wait_result = WaitForMultipleObjects(num_handles, &pcap_win32_handles_[0], false, remaining_time_to_wait_ms);

      if ((wait_result >= WAIT_OBJECT_0) && wait_result <= (WAIT_OBJECT_0 + num_handles - 1))
      {
        int dev_index = (wait_result - WAIT_OBJECT_0);

        callback_args.link_type_     = static_cast<pcpp::LinkLayerType>(pcap_datalink(pcap_devices_[dev_index].pcap_handle_));
        callback_args.ip_reassembly_ = ip_reassembly_[dev_index].get();

        pcap_dispatch(pcap_devices_[dev_index].pcap_handle_, 1, UdpcapSocketPrivate::PacketHandlerRawPtr, reinterpret_cast<u_char*>(&callback_args));

        if (callback_args.success_)
        {
          // Only return datagram if we successfully received a packet. Otherwise, we will continue receiving data, if there is time left.
          return callback_args.bytes_copied_;
        }
      }
      else if ((wait_result >= WAIT_ABANDONED_0) && wait_result <= (WAIT_ABANDONED_0 + num_handles - 1))
      {
        LOG_DEBUG("Receive error: WAIT_ABANDONED");
      }
      else if (wait_result == WAIT_TIMEOUT)
      {
        // LOG_DEBUG("Receive error: WAIT_TIMEOUT");
      }
      else if (wait_result == WAIT_FAILED)
      {
        LOG_DEBUG("Receive error: WAIT_FAILED: " + std::to_string(GetLastError()));
      }
    } while (wait_forever || (std::chrono::steady_clock::now() < wait_until));

    return 0;
  }


  bool UdpcapSocketPrivate::joinMulticastGroup(const HostAddress& group_address)
  {
    if (!is_valid_)
    {
      LOG_DEBUG("Join Multicast Group error: Socket invalid");
      return false;
    }

    if (!group_address.isValid())
    {
      LOG_DEBUG("Join Multicast Group error: Address invalid");
      return false;
    }

    if (!group_address.isMulticast())
    {
      LOG_DEBUG("Join Multicast Group error: " + group_address.toString() + " is not a multicast address");
      return false;
    }

    if (!bound_state_)
    {
      LOG_DEBUG("Join Multicast Group error: Sockt is not in bound state");
      return false;
    }

    if (multicast_groups_.find(group_address) != multicast_groups_.end())
    {
      LOG_DEBUG("Join Multicast Group error: Already joined " + group_address.toString());
      return false;
    }

    // Add theg group to the group list
    multicast_groups_.emplace(group_address);

    // Update the capture filters, so the devices will capture the multicast traffic
    updateAllCaptureFilters();

    if (multicast_loopback_enabled_)
    {
      // Trigger the Windows kernel to also send multicast traffic to localhost
      kickstartLoopbackMulticast();
    }

    return true;
  }

  bool UdpcapSocketPrivate::leaveMulticastGroup(const HostAddress& group_address)
  {
    if (!is_valid_)
    {
      LOG_DEBUG("Leave Multicast Group error: Socket invalid");
      return false;
    }

    if (!group_address.isValid())
    {
      LOG_DEBUG("Leave Multicast Group error: Address invalid");
      return false;
    }

    auto group_it = multicast_groups_.find(group_address);
    if (group_it == multicast_groups_.end())
    {
      LOG_DEBUG("Leave Multicast Group error: Not member of " + group_address.toString());
      return false;
    }

    // Remove the group from the group list
    multicast_groups_.erase(group_it);

    // Update all capture filtes
    updateAllCaptureFilters();

    return true;
  }


  void UdpcapSocketPrivate::setMulticastLoopbackEnabled(bool enabled)
  {
    if (multicast_loopback_enabled_ == enabled)
    {
      // Nothing changed
      return;
    }

    multicast_loopback_enabled_ = enabled;

    if (multicast_loopback_enabled_)
    {
      // Trigger the Windows kernel to also send multicast traffic to localhost
      kickstartLoopbackMulticast();
    }

    updateAllCaptureFilters();
  }

  bool UdpcapSocketPrivate::isMulticastLoopbackEnabled() const
  {
    return multicast_loopback_enabled_;
  }

  void UdpcapSocketPrivate::close()
  {
    // TODO: make close thread safe, so one thread can wait for data while another thread closes the socket
    for (auto& pcap_dev : pcap_devices_)
    {
      LOG_DEBUG(std::string("Closing ") + pcap_dev.device_name_);
      pcap_close(pcap_dev.pcap_handle_);
    }
    pcap_devices_      .clear();
    pcap_win32_handles_.clear();
    ip_reassembly_     .clear();

    bound_state_ = false;
    bound_port_ = 0;
    bound_address_ = HostAddress::Invalid();
  }

  //////////////////////////////////////////
  //// Internal
  //////////////////////////////////////////

  std::pair<std::string, std::string> UdpcapSocketPrivate::getDeviceByIp(const HostAddress& ip)
  {
    if (!ip.isValid())
      return{};

    // Retrieve device list
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* alldevs_rawptr;
    pcap_if_t_uniqueptr alldevs(&alldevs_rawptr, [](pcap_if_t** p) { pcap_freealldevs(*p); });

    if (pcap_findalldevs(alldevs.get(), errbuf) == -1)
    {
      fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
      return{};
    }

    for (pcap_if_t* pcap_dev = *alldevs.get(); pcap_dev; pcap_dev = pcap_dev->next)
    {
      // A user may have done something bad like assigning an IPv4 address to
      // the loopback adapter. We don't want to open it in that case. In a real-
      // world szenario this may never happen.
      if (IsLoopbackDevice(pcap_dev->name))
      {
        continue;
      }

      // Iterate through all addresses of the device and check if one of them
      // matches the one we are looking for.
      for (pcap_addr* pcap_dev_addr = pcap_dev->addresses; pcap_dev_addr; pcap_dev_addr = pcap_dev_addr->next)
      {
        if (pcap_dev_addr->addr->sa_family == AF_INET)
        {         
          struct sockaddr_in* device_ipv4_addr = (struct sockaddr_in *)pcap_dev_addr->addr;
          if (device_ipv4_addr->sin_addr.s_addr == ip.toInt())
          {
            // The IPv4 address matches!
            return std::make_pair(std::string(pcap_dev->name), std::string(pcap_dev->description));
          }
        }
      }
    }

    // Nothing found => nullptr
    return{};
  }

  std::vector<std::pair<std::string, std::string>> UdpcapSocketPrivate::getAllDevices()
  {
    // Retrieve device list
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* alldevs_rawptr;
    pcap_if_t_uniqueptr alldevs(&alldevs_rawptr, [](pcap_if_t** p) { pcap_freealldevs(*p); });

    if (pcap_findalldevs(alldevs.get(), errbuf) == -1)
    {
      fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
      return{};
    }

    std::vector<std::pair<std::string, std::string>> alldev_vector;
    for (pcap_if_t* pcap_dev = *alldevs.get(); pcap_dev; pcap_dev = pcap_dev->next)
    {
      alldev_vector.push_back(std::make_pair(std::string(pcap_dev->name), std::string(pcap_dev->description)));
    }
    return alldev_vector;
  }

  std::string UdpcapSocketPrivate::getMac(pcap_t* pcap_handle)
  {
    // Check whether the handle actually is an ethernet device
    if (pcap_datalink(pcap_handle) == DLT_EN10MB)
    {
      // Data for the OID Request
      size_t mac_size = 6;
      std::vector<char> mac(mac_size);

      // Send OID-Get-Request to the driver
      if (pcap_oid_get_request(pcap_handle, OID_802_3_CURRENT_ADDRESS, &mac[0], &mac_size))
      {
        LOG_DEBUG("Error getting MAC address");
        return "";
      }

      // Convert binary mac into human-readble form (we need it this way for the kernel filter)
      std::string mac_string(18, ' ');
      snprintf(&mac_string[0], mac_string.size(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      mac_string.pop_back(); // Remove terminating null char

      return std::move(mac_string);
    }
    else
    {
      // If not on ethernet, we assume that we don't have a MAC
      return "";
    }
  }

  bool UdpcapSocketPrivate::openPcapDevice(const std::string& device_name)
  {
    char errbuf[PCAP_ERRBUF_SIZE];

    pcap_t* pcap_handle = pcap_create(device_name.c_str(), errbuf);

    if (pcap_handle == nullptr)
    {
      fprintf(stderr, "\nUnable to open the adapter: %s\n", errbuf);
      return false;
    }

    pcap_set_snaplen(pcap_handle, MAX_PACKET_SIZE);
    pcap_set_promisc(pcap_handle, 1 /*true*/); // We only want Packets destined for this adapter. We are not interested in others.
    pcap_set_immediate_mode(pcap_handle, 1 /*true*/);

    if (receive_buffer_size_ > 0)
    {
      pcap_set_buffer_size(pcap_handle, receive_buffer_size_);
    }

    int errorcode = pcap_activate(pcap_handle);
    switch (errorcode)
    {
    case 0:
      break; // SUCCESS!
    case PCAP_WARNING_PROMISC_NOTSUP:
      pcap_perror(pcap_handle, ("UdpcapSocket WARNING: Device " + device_name + " does not support promiscuous mode").c_str());
      break;
    case PCAP_WARNING:
      pcap_perror(pcap_handle, ("UdpcapSocket WARNING: Device " + device_name).c_str());
      break;
    case PCAP_ERROR_ACTIVATED:
      fprintf(stderr, "%s", ("UdpcapSocket ERROR: Device " + device_name + " already activated").c_str());
      return false;
    case PCAP_ERROR_NO_SUCH_DEVICE:
      pcap_perror(pcap_handle, ("UdpcapSocket ERROR: Device " + device_name + " does not exist").c_str());
      return false;
    case PCAP_ERROR_PERM_DENIED:
      pcap_perror(pcap_handle, ("UdpcapSocket ERROR: Device " + device_name + ": Permissoin denied").c_str());
      return false;
    case PCAP_ERROR_RFMON_NOTSUP:
      fprintf(stderr, "%s", ("UdpcapSocket ERROR: Device " + device_name + ": Does not support monitoring").c_str());
      return false;
    case PCAP_ERROR_IFACE_NOT_UP:
      fprintf(stderr, "%s", ("UdpcapSocket ERROR: Device " + device_name + ": Interface is down").c_str());
      return false;
    case PCAP_ERROR:
      pcap_perror(pcap_handle, ("UdpcapSocket ERROR: Device " + device_name).c_str());
      return false;
    default:
      fprintf(stderr, "%s", ("UdpcapSocket ERROR: Device " + device_name + ": Unknown error").c_str());
      return false;
    }


    PcapDev pcap_dev(pcap_handle, IsLoopbackDevice(device_name), device_name);
   
    pcap_devices_      .push_back(pcap_dev);
    pcap_win32_handles_.push_back(pcap_getevent(pcap_handle));
    ip_reassembly_     .emplace_back(std::make_unique<Udpcap::IpReassembly>(std::chrono::seconds(5)));

    return true;
  }

  std::string UdpcapSocketPrivate::createFilterString(PcapDev& pcap_dev) const
  {
    std::stringstream ss;

    // No outgoing packets (determined by MAC, loopback packages don't have an ethernet header)
    if (!pcap_dev.is_loopback_)
    {
      std::string mac_string = getMac(pcap_dev.pcap_handle_);
      if (!mac_string.empty())
      {
        ss << "not ether src " << mac_string;
        ss << " and ";
      }
    }

    // IP traffic having UDP payload
    ss << "ip and udp";

    // UDP Port or IPv4 fragmented traffic (in IP fragments we cannot see the UDP port, yet)
    ss << " and (udp port " << bound_port_ << " or (ip[6:2] & 0x3fff != 0))";

    // IP
    // Unicast traffic
    ss << " and (((not ip multicast) ";
    if (bound_address_ != HostAddress::Any() && bound_address_ != HostAddress::Broadcast())
    {
      ss << "and (ip dst " << bound_address_.toString() << ")";
    }
    ss << ")";
      
    // Multicast traffic
    if ((multicast_groups_.size() > 0)
      &&(!pcap_dev.is_loopback_ || multicast_loopback_enabled_))
    {
      ss << " or (ip multicast and (";
      for (auto ip_it = multicast_groups_.begin(); ip_it != multicast_groups_.end(); ip_it++)
      {
        if (ip_it != multicast_groups_.begin())
          ss << " or ";
        ss << "dst " << ip_it->toString();
      }
      ss << "))";
    }

    ss << ")";

    return ss.str();
  }

  void UdpcapSocketPrivate::updateCaptureFilter(PcapDev& pcap_dev)
  {
    // Create new filter
    std::string filter_string = createFilterString(pcap_dev);

    LOG_DEBUG("Setting filter string: " + filter_string);

    bpf_program filter_program;

    // Compile the filter
    int pcap_compile_ret;
    {
      // pcap_compile is not thread safe, so we need a global mutex
      std::lock_guard<std::mutex> pcap_compile_lock(pcap_compile_mutex);
      pcap_compile_ret = pcap_compile(pcap_dev.pcap_handle_, &filter_program, filter_string.c_str(), 1, PCAP_NETMASK_UNKNOWN);
    }

    if (pcap_compile_ret == PCAP_ERROR)
    {
      pcap_perror(pcap_dev.pcap_handle_, ("UdpcapSocket ERROR: Unable to compile filter \"" + filter_string + "\"").c_str()); // TODO: revise error printing
    }
    else
    {
      // Set the filter
      if (pcap_setfilter(pcap_dev.pcap_handle_, &filter_program) == PCAP_ERROR)
      {
        pcap_perror(pcap_dev.pcap_handle_, ("UdpcapSocket ERROR: Unable to set filter \"" + filter_string + "\"").c_str());
        pcap_freecode(&filter_program);
      }
    }
  }

  void UdpcapSocketPrivate::updateAllCaptureFilters()
  {
    for (auto& pcap_dev : pcap_devices_)
    {
      updateCaptureFilter(pcap_dev);
    }
  }

  void UdpcapSocketPrivate::kickstartLoopbackMulticast() const
  {
    uint16_t kickstart_port = 62000;

    asio::io_context iocontext;
    asio::ip::udp::socket kickstart_socket(iocontext);

    // create socket
    asio::ip::udp::endpoint listen_endpoint(asio::ip::make_address("0.0.0.0"), kickstart_port);
    kickstart_socket.open(listen_endpoint.protocol());

    // set socket reuse
    kickstart_socket.set_option(asio::ip::udp::socket::reuse_address(true));

    // bind socket
    kickstart_socket.bind(listen_endpoint);

    // multicast loopback
    kickstart_socket.set_option(asio::ip::multicast::enable_loopback(true));

    // multicast ttl
    kickstart_socket.set_option(asio::ip::multicast::hops(0));

    // Join all multicast groups
    for (const auto& multicast_group : multicast_groups_)
    {
      asio::ip::address asio_mc_group = asio::ip::make_address(multicast_group.toString());
      kickstart_socket.set_option(asio::ip::multicast::join_group(asio_mc_group));
    }

    // Send data to all multicast groups
    for (const auto& multicast_group : multicast_groups_)
    {
      LOG_DEBUG(std::string("Sending loopback kickstart packet to ") + multicast_group.toString() + ":" + std::to_string(kickstart_port));
      asio::ip::address asio_mc_group = asio::ip::make_address(multicast_group.toString());
      asio::ip::udp::endpoint send_endpoint(asio_mc_group, kickstart_port);
      kickstart_socket.send_to(asio::buffer(static_cast<void*>(nullptr), 0), send_endpoint, 0);
    }

    // Close the socket
    kickstart_socket.close();
  }

  void UdpcapSocketPrivate::PacketHandlerVector(unsigned char* param, const struct pcap_pkthdr* header, const unsigned char* pkt_data)
  {
    CallbackArgsVector* callback_args = reinterpret_cast<CallbackArgsVector*>(param);

    pcpp::RawPacket rawPacket(pkt_data, header->caplen, header->ts, false, callback_args->link_type_);
    pcpp::Packet    packet(&rawPacket, pcpp::UDP);

    pcpp::IPv4Layer* ip_layer  = packet.getLayerOfType<pcpp::IPv4Layer>();
    pcpp::UdpLayer*  udp_layer = packet.getLayerOfType<pcpp::UdpLayer>();

    if (ip_layer)
    {
      if (ip_layer->isFragment())
      {
        // Handle fragmented IP traffic
        pcpp::IPReassembly::ReassemblyStatus status;

        // Try to reassemble packet
        pcpp::Packet* reassembled_packet = callback_args->ip_reassembly_->processPacket(&rawPacket, status);

        // If we are done reassembling the packet, we return it to the user
        if (reassembled_packet)
        {
          pcpp::Packet re_parsed_packet(reassembled_packet->getRawPacket(), pcpp::UDP);

          pcpp::IPv4Layer* reassembled_ip_layer  = re_parsed_packet.getLayerOfType<pcpp::IPv4Layer>();
          pcpp::UdpLayer*  reassembled_udp_layer = re_parsed_packet.getLayerOfType<pcpp::UdpLayer>();

          if (reassembled_ip_layer && reassembled_udp_layer)
            FillCallbackArgsVector(callback_args, reassembled_ip_layer, reassembled_udp_layer);

          delete reassembled_packet; // We need to manually delete the packet pointer
        }
      }
      else if (udp_layer)
      {
        // Handle normal IP traffic (un-fragmented)
        FillCallbackArgsVector(callback_args, ip_layer, udp_layer);
      }
    }
  }

  void UdpcapSocketPrivate::FillCallbackArgsVector(CallbackArgsVector* callback_args, pcpp::IPv4Layer* ip_layer, pcpp::UdpLayer* udp_layer)
  {
    auto dst_port = ntohs(udp_layer->getUdpHeader()->portDst);

    if (dst_port == callback_args->bound_port_)
    {
      if (callback_args->source_address_)
        *callback_args->source_address_ = HostAddress(ip_layer->getSrcIPv4Address().toInt());

      if (callback_args->source_port_)
        *callback_args->source_port_ = ntohs(udp_layer->getUdpHeader()->portSrc);

      callback_args->destination_vector_->reserve(udp_layer->getLayerPayloadSize());
      callback_args->destination_vector_->assign(udp_layer->getLayerPayload(), udp_layer->getLayerPayload() + udp_layer->getLayerPayloadSize());
      callback_args->success_ = true;
    }
  }

  void UdpcapSocketPrivate::PacketHandlerRawPtr(unsigned char* param, const struct pcap_pkthdr* header, const unsigned char* pkt_data)
  {
    CallbackArgsRawPtr* callback_args = reinterpret_cast<CallbackArgsRawPtr*>(param);

    pcpp::RawPacket rawPacket(pkt_data, header->caplen, header->ts, false, callback_args->link_type_);
    pcpp::Packet    packet(&rawPacket, pcpp::UDP);

    pcpp::IPv4Layer* ip_layer = packet.getLayerOfType<pcpp::IPv4Layer>();
    pcpp::UdpLayer*  udp_layer = packet.getLayerOfType<pcpp::UdpLayer>();

    if (ip_layer)
    {
      if (ip_layer->isFragment())
      {
        // Handle fragmented IP traffic
        pcpp::IPReassembly::ReassemblyStatus status;

        // Try to reasseble packet
        pcpp::Packet* reassembled_packet = callback_args->ip_reassembly_->processPacket(&rawPacket, status);

        // If we are done reassembling the packet, we return it to the user
        if (reassembled_packet)
        {
          pcpp::Packet re_parsed_packet(reassembled_packet->getRawPacket(), pcpp::UDP);

          pcpp::IPv4Layer* reassembled_ip_layer = re_parsed_packet.getLayerOfType<pcpp::IPv4Layer>();
          pcpp::UdpLayer*  reassembled_udp_layer = re_parsed_packet.getLayerOfType<pcpp::UdpLayer>();

          if (reassembled_ip_layer && reassembled_udp_layer)
            FillCallbackArgsRawPtr(callback_args, reassembled_ip_layer, reassembled_udp_layer);

          delete reassembled_packet; // We need to manually delete the packet pointer
        }
      }
      else if (udp_layer)
      {
        // Handle normal IP traffic (un-fragmented)
        FillCallbackArgsRawPtr(callback_args, ip_layer, udp_layer);
      }
    }

  }

  void UdpcapSocketPrivate::FillCallbackArgsRawPtr(CallbackArgsRawPtr* callback_args, pcpp::IPv4Layer* ip_layer, pcpp::UdpLayer* udp_layer)
  {
    auto dst_port = ntohs(udp_layer->getUdpHeader()->portDst);

    if (dst_port == callback_args->bound_port_)
    {
      if (callback_args->source_address_)
        *callback_args->source_address_ = HostAddress(ip_layer->getSrcIPv4Address().toInt());

      if (callback_args->source_port_)
        *callback_args->source_port_ = ntohs(udp_layer->getUdpHeader()->portSrc);


      size_t bytes_to_copy = std::min(callback_args->destination_buffer_size_, udp_layer->getLayerPayloadSize());

      memcpy_s(callback_args->destination_buffer_, callback_args->destination_buffer_size_, udp_layer->getLayerPayload(), bytes_to_copy);
      callback_args->bytes_copied_ = bytes_to_copy;

      callback_args->success_ = true;
    }

  }
}
