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

#pragma once

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4800 4200 4100)
#endif // _MSC_VER
#include <IPReassembly.h>   // Pcap++ de-fragmentation of IP packets
#ifdef _MSC_VER
#pragma warning( pop )
#endif // _MSC_VER

#include <chrono>
#include <unordered_map>
#include <memory>

namespace Udpcap
{
  class IpReassembly
  {

  /////////////////////////////////////////
  /// Constructor & Destructor
  /////////////////////////////////////////
  public: // TODO: Document
	/**
	  * A c'tor for this class.
	  * 
	  * @param[in] max_package_age The maximum age that a packet may have for being used for reassembly.
	  * @param[in] max_packets_to_store Set the capacity limit of the IP reassembly mechanism. Default capacity is #PCPP_IP_REASSEMBLY_DEFAULT_MAX_PACKETS_TO_STORE
	  */
	IpReassembly(std::chrono::nanoseconds max_package_age, size_t max_packets_to_store = PCPP_IP_REASSEMBLY_DEFAULT_MAX_PACKETS_TO_STORE);

	~IpReassembly() = default;

	// Disable copy and move construction, as we need the this pointer as callback cookie. So this must never change.
	IpReassembly ( const IpReassembly & ) = delete;
	void operator=( const IpReassembly & ) = delete;
	IpReassembly ( IpReassembly && ) = delete;
	void operator=( const IpReassembly && ) = delete;


  /////////////////////////////////////////
  /// API borrowed from pcpp::IPReassembly
  /////////////////////////////////////////
  
  public:
	/**
	  * The main API that drives IPReassembly. This method should be called whenever a fragment arrives. This method finds the relevant
	  * packet this fragment belongs to and runs the IP reassembly logic that is described in IPReassembly.h.
	  * @param[in] fragment The fragment to process (IPv4 or IPv6). Please notice that the reassembly logic doesn't change or manipulate
	  * this object in any way. All of its data is copied to internal structures and manipulated there
	  * @param[out] status An indication of the packet reassembly status following the processing of this fragment. Possible values are:
	  * - The input fragment is not a IPv4 or IPv6 packet
	  * - The input fragment is not a IPv4 or IPv6 fragment packet
	  * - The input fragment is the first fragment of the packet
	  * - The input fragment is not the first or last fragment
	  * - The input fragment came out-of-order, meaning that wasn't the fragment that was currently expected (it's data is copied to
	  *   the out-of-order fragment list)
	  * - The input fragment is malformed and will be ignored
	  * - The input fragment is the last one and the packet is now fully reassembled. In this case the return value will contain
	  *   a pointer to the reassebmled packet
	  * @param[in] parse_until Optional parameter. Parse the reassembled packet until you reach a certain protocol (inclusive). Can be useful for cases when you need to parse only up to a
	  * certain layer and want to avoid the performance impact and memory consumption of parsing the whole packet. Default value is ::UnknownProtocol which means don't take this
	  * parameter into account
	  * @param[in] parse_until_layer Optional parameter. Parse the reassembled packet until you reach a certain layer in the OSI model (inclusive). Can be useful for cases when you need to
	  * parse only up to a certain OSI layer (for example transport layer) and want to avoid the performance impact and memory consumption of parsing the whole packet.
	  * Default value is ::OsiModelLayerUnknown which means don't take this parameter into account
	  * @return
	  * - If the input fragment isn't an IPv4/IPv6 packet or if it isn't an IPv4/IPv6 fragment, the return value is a pointer to the input fragment
	  * - If the input fragment is the last one and the reassembled packet is ready - a pointer to the reassembled packet is
	  *   returned. Notice it's the user's responsibility to free this pointer when done using it
	  * - If the reassembled packet isn't ready then NULL is returned
	  */
	pcpp::Packet* processPacket(pcpp::Packet* fragment, pcpp::IPReassembly::ReassemblyStatus& status, pcpp::ProtocolType parse_until = pcpp::UnknownProtocol, pcpp::OsiModelLayer parse_until_layer = pcpp::OsiModelLayerUnknown);

	/**
	  * The main API that drives IPReassembly. This method should be called whenever a fragment arrives. This method finds the relevant
	  * packet this fragment belongs to and runs the IPv4 reassembly logic that is described in IPReassembly.h.
	  * @param[in] fragment The fragment to process (IPv4 or IPv6). Please notice that the reassembly logic doesn't change or manipulate
	  * this object in any way. All of its data is copied to internal structures and manipulated there
	  * @param[out] status An indication of the packet reassembly status following the processing of this fragment. Possible values are:
	  * - The input fragment is not a IPv4 or IPv6 packet
	  * - The input fragment is not a IPv4 or IPv6 fragment packet
	  * - The input fragment is the first fragment of the packet
	  * - The input fragment is not the first or last fragment
	  * - The input fragment came out-of-order, meaning that wasn't the fragment that was currently expected (it's data is copied to
	  *   the out-of-order fragment list)
	  * - The input fragment is malformed and will be ignored
	  * - The input fragment is the last one and the packet is now fully reassembled. In this case the return value will contain
	  *   a pointer to the reassebmled packet
	  * @param[in] parse_until Optional parameter. Parse the raw and reassembled packets until you reach a certain protocol (inclusive). Can be useful for cases when you need to parse only up to a
	  * certain layer and want to avoid the performance impact and memory consumption of parsing the whole packet. Default value is ::UnknownProtocol which means don't take this
	  * parameter into account
	  * @param[in] parse_until_layer Optional parameter. Parse the raw and reassembled packets until you reach a certain layer in the OSI model (inclusive). Can be useful for cases when you need to
	  * parse only up to a certain OSI layer (for example transport layer) and want to avoid the performance impact and memory consumption of parsing the whole packet.
	  * Default value is ::OsiModelLayerUnknown which means don't take this parameter into account
	  * @return
	  * - If the input fragment isn't an IPv4/IPv6 packet or if it isn't an IPv4/IPv6 fragment, the return value is a pointer to a Packet object
	  *   wrapping the input fragment RawPacket object. It's the user responsibility to free this instance
	  * - If the input fragment is the last one and the reassembled packet is ready - a pointer to the reassembled packet is
	  *   returned. Notice it's the user's responsibility to free this pointer when done using it
	  * - If the reassembled packet isn't ready then NULL is returned
	  */
	pcpp::Packet* processPacket(pcpp::RawPacket* fragment, pcpp::IPReassembly::ReassemblyStatus& status, pcpp::ProtocolType parse_until = pcpp::UnknownProtocol, pcpp::OsiModelLayer parse_until_layer = pcpp::OsiModelLayerUnknown);
	// TODO: Implement rawPacket function

	/**
	  * Get the maximum capacity as determined in the c'tor
	  */
	size_t getMaxCapacity() const { return ip_reassembly_.getMaxCapacity(); }

	/**
	  * Get the current number of packets being processed
	  */
	size_t getCurrentCapacity() const { return ip_reassembly_.getCurrentCapacity(); }

  /////////////////////////////////////////
  /// Helper functions
  /////////////////////////////////////////
  private:

	std::unique_ptr<pcpp::IPReassembly::PacketKey> getPacketKey(pcpp::Packet* packet);

	void removeOldPackages();
	void removePackageFromTimestampMap(std::unique_ptr<pcpp::IPReassembly::PacketKey> packet_key);
	void updatePackageInTimestampMap  (std::unique_ptr<pcpp::IPReassembly::PacketKey> packet_key);

	static void onFragmentsCleanCallback(const pcpp::IPReassembly::PacketKey* packt_key, void* this_ptr);

  /////////////////////////////////////////
  /// Member variables
  /////////////////////////////////////////
  private:
    const std::chrono::nanoseconds                            max_package_age_;
    pcpp::IPReassembly                                        ip_reassembly_;
    std::unordered_map<int32_t, std::pair<std::unique_ptr<pcpp::IPReassembly::PacketKey>, std::chrono::steady_clock::time_point>> timestamp_map_;
  };
}