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

#include "ip_reassembly.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4100 4200)
#endif // _MSC_VER
#include <IPv4Layer.h>
#include <IPv6Layer.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

#include <iostream>

namespace Udpcap
{
  /////////////////////////////////////////
  /// Constructor & Destructor
  /////////////////////////////////////////
  
  IpReassembly::IpReassembly(std::chrono::nanoseconds max_package_age, size_t max_packets_to_store)
    : max_package_age_(max_package_age)
    , ip_reassembly_(&IpReassembly::onFragmentsCleanCallback, static_cast<void*>(this), max_packets_to_store)
  {}

  /////////////////////////////////////////
  /// API borrowed from pcpp::IPReassembly
  /////////////////////////////////////////
  
  pcpp::Packet* IpReassembly::processPacket(pcpp::Packet* fragment, pcpp::IPReassembly::ReassemblyStatus& status, pcpp::ProtocolType parse_until, pcpp::OsiModelLayer parse_until_layer)
  {
    removeOldPackages();

    pcpp::Packet* packet = ip_reassembly_.processPacket(fragment, status, parse_until, parse_until_layer);

    if ((status & pcpp::IPReassembly::REASSEMBLED) != 0)
    {
      // We have reassembled an entire packet.
      auto packet_key = getPacketKey(fragment);
      removePackageFromTimestampMap(std::move(packet_key));
    }
    else if ((status & (pcpp::IPReassembly::FIRST_FRAGMENT | pcpp::IPReassembly::FRAGMENT | pcpp::IPReassembly::OUT_OF_ORDER_FRAGMENT)) != 0)
    {
      // The input was a fragment an will be kept in the buffer
      auto packet_key = getPacketKey(fragment);
      updatePackageInTimestampMap(std::move(packet_key));
    }

    return packet;
  }

  pcpp::Packet* IpReassembly::processPacket(pcpp::RawPacket* fragment, pcpp::IPReassembly::ReassemblyStatus& status, pcpp::ProtocolType parse_until, pcpp::OsiModelLayer parse_until_layer)
  {
    pcpp::Packet* parsedFragment = new pcpp::Packet(fragment, false, parse_until, parse_until_layer);
    pcpp::Packet* result         = this->processPacket(parsedFragment, status, parse_until, parse_until_layer);

    if (result != parsedFragment)
      delete parsedFragment;

    return result;
  }

  /////////////////////////////////////////
  /// Helper functions
  /////////////////////////////////////////

  std::unique_ptr<pcpp::IPReassembly::PacketKey> IpReassembly::getPacketKey(pcpp::Packet* packet)
  {
    {
      pcpp::IPv4Layer* ipv4_layer  = packet->getLayerOfType<pcpp::IPv4Layer>();
      if (ipv4_layer)
      {
        return std::make_unique<pcpp::IPReassembly::IPv4PacketKey>(_byteswap_ushort(ipv4_layer->getIPv4Header()->ipId), ipv4_layer->getSrcIPv4Address(), ipv4_layer->getDstIPv4Address());
      }
    }
    
    {
      pcpp::IPv6Layer* ipv6_layer  = packet->getLayerOfType<pcpp::IPv6Layer>();
      if (ipv6_layer)
      {
        auto frag_header = ipv6_layer->getExtensionOfType<pcpp::IPv6FragmentationHeader>();
        if (frag_header)
          return std::make_unique<pcpp::IPReassembly::IPv6PacketKey>(ntohl(frag_header->getFragHeader()->id), ipv6_layer->getSrcIPv6Address(), ipv6_layer->getDstIPv6Address());
        else
          return nullptr;
      }
    }

    return nullptr;
  }

  void IpReassembly::removeOldPackages()
  {
    auto now = std::chrono::steady_clock::now();

    for (auto package_timestamp_it = timestamp_map_.begin()
      ; package_timestamp_it != timestamp_map_.end()
      ; package_timestamp_it++)
    {
      if (package_timestamp_it->second.second < (now - max_package_age_))
      {
        ip_reassembly_.removePacket(*package_timestamp_it->second.first);
        package_timestamp_it = timestamp_map_.erase(package_timestamp_it);
      }
    }

    if (timestamp_map_.size() != ip_reassembly_.getCurrentCapacity())
    {
      std::cerr << "Udpcap: Pcap++ IpReassembly and custom IP Reassembly are outof sync! Pcap++ reassembly capacity: " << ip_reassembly_.getCurrentCapacity() << ", timestamp_map size: " << timestamp_map_.size() << std::endl;
    }
  }

  void IpReassembly::removePackageFromTimestampMap(std::unique_ptr<pcpp::IPReassembly::PacketKey> packet_key)
  {
    auto package_it = timestamp_map_.find(packet_key->getHashValue());
    if (package_it != timestamp_map_.end())
    {
      timestamp_map_.erase(package_it);
    }
  }

  void IpReassembly::updatePackageInTimestampMap(std::unique_ptr<pcpp::IPReassembly::PacketKey> packet_key)
  {
    auto now = std::chrono::steady_clock::now();

    auto package_it = timestamp_map_.find(packet_key->getHashValue());
    if (package_it != timestamp_map_.end())
    {
      package_it->second.second = now;
    }
    else
    {
      timestamp_map_.emplace(packet_key->getHashValue(), std::make_pair(std::move(packet_key), now));
    }
  }

  void IpReassembly::onFragmentsCleanCallback(const pcpp::IPReassembly::PacketKey* packt_key, void* this_ptr)
  {
    IpReassembly* this_ = static_cast<IpReassembly*>(this_ptr);
    auto it_to_erase = this_->timestamp_map_.find(packt_key->getHashValue());

    if(it_to_erase != this_->timestamp_map_.end())
    {
      this_->timestamp_map_.erase(it_to_erase);
    }
    else
    {
      std::cerr << "Udpcap: Pcap++ deleted an IP Fragment that we did not know anything about!" << std::endl;
    }
  }
}
