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
 * @file   ecal_config_types.h
 * @brief  eCAL config interface using structs
**/

#pragma once

#include <string>
#include <vector>
#include <regex>
#include <optional>
#include <iostream>
#include <chrono>
#include <limits>
#include <ecal/ecal_tlayer.h>
#include "ecal/ecal_os.h"
#include "ecal/ecal_log_level.h"

namespace eCAL
{
    namespace Config
    {
        // Type definitions
        class IpAddressV4
        {
        public:
            IpAddressV4() {};
            IpAddressV4(std::string ip_address_)
            {
                if (checkIpString(ip_address_))
                {
                    m_ip_address = ip_address_;
                }
                else
                {
                    std::cout << "IpAddressV4 error: check your IpAddress settings." << std::endl;
                }
            }            

            std::string get() { return m_ip_address; }

        private:            
            bool checkIpString(std::string ip_address_)
            {
                if (std::regex_match(ip_address_, std::regex("(([0-9]|[0-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])")))
                {
                    return true;
                }
                else if (std::regex_match(ip_address_, std::regex("(([0-9a-fA-F]|[0-9a-fA-F][0-9a-fA-F])\\.){3}([0-9a-fA-F]|[0-9a-fA-F][0-9a-fA-F])")))
                {
                    return true;
                }
                else
                {
                    return false;
                }

            }
            std::string     m_ip_address;

        };

        template<int MIN = 0, int STEP = 1, long long MAX = std::numeric_limits<int>::max()>
        class LimitSize
        {
        public:
            LimitSize(int size_ = 0)
            {
                if (size_ >= m_size_min && size_ <= m_size_max && size_ % m_size_step == 0)
                {
                    m_size = size_;
                }
                else
                {
                    std::cout << "LimitSize: faulty size configuration or assignment - using minimum size " << MIN << std::endl;
                }
            };

            int get() { return m_size; };

        private:
            int       m_size_min = MIN;
            long long m_size_max = MAX;
            int       m_size_step = STEP;

            int m_size = MIN;
        };

        enum class UdpConfigVersion
        {
            V1 = 1, // Legacy
            V2 = 2
        };

        // ecal config types
        struct UdpMulticastOptions
        {
            UdpConfigVersion         config_version;        
            IpAddressV4              group;
            IpAddressV4              mask;
            LimitSize<14000, 10>     port;
            unsigned int             ttl;
            LimitSize<5242880, 1024> sndbuf;
            LimitSize<5242880, 1024> recbuf;
            bool                     join_all_interfaces;

            int bandwidth_max_udp;
            bool npcap_enabled;
        }; 

        struct TCPubsubOptions
        {
            size_t num_executor_reader;
            size_t num_executor_writer;
            size_t max_reconnections;
        };

        struct ProcessOptions
        {
            std::string terminal_emulator;
        };
        
        struct SHMOptions
        {
            std::string                host_group_name;
            LimitSize<4096, 4096>      memfile_minsize;
            LimitSize<50, 1, 100>      memfile_reserve;
            int                        memfile_ack_timeout;
            LimitSize<0, 1>            memfile_buffer_count;
            bool                       drop_out_of_order_messages;
            bool                       memfile_zero_copy;
        };

        struct TransportLayerOptions
        {
            bool                network_enabled = false;            // correct here?
            bool                drop_out_of_order_messages = false;
            UdpMulticastOptions mc_options;
            TCPubsubOptions     tcp_options;
            SHMOptions          shm_options;
        };

        enum MonitoringMode
        {
            none = 0,
            udp_monitoring = 1 << 0,
            shm_monitoring = 1 << 1
        };

        typedef char eCAL_MonitoringMode_Filter;

        struct UDPMonitoringOptions
        {

            // what is here?
        };

        struct SHMMonitoringOptions
        {
            std::string shm_monitoring_domain;
            size_t      shm_monitoring_queue_size;  
        };

        struct MonitoringOptions
        {
            eCAL_MonitoringMode_Filter monitoring_mode;                     // (int)MonitoringMode::UDP_MONITORING | (int)MonitoringMode::SHM_MONITORING
            LimitSize<1000, 1000>      monitoring_timeout;           // 1000 + (x * 1000)       in ms     
            bool                       network_monitoring_disabled; // disabled/enabled?
            UDPMonitoringOptions       udp_options;
            SHMMonitoringOptions       shm_options;

            std::string         filter_excl;                                 // regex for blacklisting, default __.*
            std::string         filter_incl;                                   // regex for whitelisting, default ""
            eCAL_Logging_Filter filter_log_con;   // default? options: all, info, warning, error, fatal, debug1, debug2, debug3, debug4 
            eCAL_Logging_Filter filter_log_file;                           // How to use the loglevels? BitOperator? Vector?
            eCAL_Logging_Filter filter_log_udp; // enum in ecal_log_level.h
        };

        struct ReceivingOptions          // -> sind receiving options
        {
            bool inproc_recv_enabled;
            bool shm_recv_enabled;
            bool tcp_recv_enabled;
            bool udp_mc_recv_enabled;
        };

        struct PublisherOptions
        {
            TLayer::eSendMode use_inproc;     // 0 = off, 1 = on, 2 = auto, default = 0
            TLayer::eSendMode use_shm;        // 0 = off, 1 = on, 2 = auto, default = 2
            TLayer::eSendMode use_tcp;        // 0 = off, 1 = on, 2 = auto, default = 0
            TLayer::eSendMode use_udp_mc;     // 0 = off, 1 = on, 2 = auto, default = 2
        };

        struct SysOptions
        {
            std::string filter_excl; // cloud import blacklists - > regex? 
        };

        struct TimesyncOptions
        {
            std::string timesync_module; // ecaltime-localtime, ecaltime-linuxptp, ecaltime-simtime - what about own implementations?
        };

        struct RegistrationOptions
        {
        public:
            typedef std::chrono::milliseconds MS;
            RegistrationOptions() {};
            RegistrationOptions(MS reg_timeout_, MS reg_refresh_)
            {
                if (reg_refresh_ < reg_timeout_)
                {
                    registration_timeout = reg_timeout_;
                    registration_refresh = reg_refresh_;
                }
                else
                {
                    std::cout << "RegistrationOptions: custom registration refresh >= registration timout. Using default values." << std::endl;
                }
            };

            MS getTimeout() const { return registration_timeout; }
            MS getRefresh() const { return registration_refresh; }

            bool share_ttype = true;
            bool share_tdesc = true;

        private:
            MS registration_timeout = MS(60000); // 
            MS registration_refresh = MS(1000);  // refresh < registration timeout
        };

        struct ServiceOptions
        {
            bool protocol_v0;
            bool protocol_v1;
        };

        struct eCALConfig
        {
            TransportLayerOptions   transport_layer_options;
            RegistrationOptions     registration_options;
            MonitoringOptions       monitoring_options;
            ReceivingOptions        receiving_options;
            PublisherOptions        publisher_options;
            SysOptions              sys_options;
            TimesyncOptions         timesync_options;
            ServiceOptions          service_options;
            std::string             loaded_ecal_ini_file;
        };

        ECAL_API eCALConfig  GetDefaultConfig();

        ECAL_API eCALConfig  GetIniConfig();

        ECAL_API eCALConfig* GetCurrentConfig();

	} // end namespace Config


    enum class LayerOptions
    {
        LAYER_NONE       = 0, // needed?
        LAYER_UDP_TCP    = 1 << 1,
        LAYER_UDP_UDP_MC = 1 << 2,
        LAYER_SHM        = 1 << 3,
        LAYER_INMEMORY   = 1 << 4
    };

    constexpr int LAYER_NONE       = 0x0000;
    constexpr int LAYER_UDP_UDP_MC = 0x0010;
    constexpr int LAYER_UDP_TCP    = 0x0011;
    constexpr int LAYER_SHM        = 0x0020;
    constexpr int LAYER_INMEMORY   = 0x0030;

    enum class NetworkTransportLayer
    {
        tlayer_none   = LAYER_NONE,
        tlayer_udp_mc = LAYER_UDP_UDP_MC,
        tlayer_tcp    = LAYER_UDP_TCP
    };

    enum class LocalTransportLayer
    {
        tlayer_none   = LAYER_NONE,
        tlayer_udp_mc = LAYER_UDP_UDP_MC,
        tlayer_tcp    = LAYER_UDP_TCP,
        tlayer_shm    = LAYER_SHM
    };

    enum class InprocTransportLayer
    {
        tlayer_none = LAYER_NONE,
        tlayer_udp_mc = LAYER_UDP_UDP_MC,
        tlayer_tcp = LAYER_UDP_TCP,
        tlayer_shm = LAYER_SHM,
        tlayer_inmemory = LAYER_INMEMORY
    };

    struct eCAL_UDP_MC_Publisher_Options
    {
        long max_bandwidth; // rausschmeißen
        // should we go as far as to put the MC address here?

    };

    struct eCAL_TCP_Publisher_Options
    {
        // should we go as far as to put the  TCP address and port here? - NO only user options
        // both pub/sub need to know
    };

    struct eCAL_SHM_Publisher_Options
    {
        bool      enable_zero_copy = false;

        long      buffer_count = 1;                      // 1 .. x
        long long acknowledge_timeout_ms = -1; // -1 -> no timeout (or directly std::chrono?

        // should we go as far as to put the memory filename (base) here? - No
        // however, part of it will be communicated via the registration layer, invisible to the user
    };

    struct PublisherOptions 
    {
        eCAL_UDP_MC_Publisher_Options udp_mc_options;
        eCAL_TCP_Publisher_Options    tcp_options;
        eCAL_SHM_Publisher_Options    shm_options;

        InprocTransportLayer  inproc_layer;
        LocalTransportLayer   local_layer;
        NetworkTransportLayer network_layer;

        bool share_topic_information = true;
    };

    struct SubscriberOptions
    {
        InprocTransportLayer  inproc_layer;
        LocalTransportLayer   local_layer;
        NetworkTransportLayer network_layer;

        bool share_topic_information = true;
    };

    // TODO PG: Clarify how to handle these
    struct InternalConfig
    {
        SubscriberOptions subscriber_options;
        PublisherOptions  publisher_options;
    };
} // end namespace eCAL