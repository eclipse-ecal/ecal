/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

#include <ecal_c/ecal.h>
#include <ecal/ecal.h>
#include <gtest/gtest.h>

class config_test_c : public ::testing::Test 
{
    protected:
        eCAL_Configuration* configuration0;

        void SetUp() override 
        {
            configuration0 = eCAL_Configuration_New();
            eCAL_Configuration_InitFromConfig(configuration0);
            eCAL_Initialize(NULL, NULL, configuration0);
        }

        void TearDown() override 
        {
            eCAL_Finalize();
            eCAL_Configuration_Delete(configuration0);
        }
};

TEST_F(config_test_c, GetConfiguration)
{
    EXPECT_NE(nullptr, eCAL_GetConfiguration());
    EXPECT_NE(nullptr, eCAL_GetSubscriberConfiguration());
    EXPECT_NE(nullptr, eCAL_GetPublisherConfiguration());
}

TEST_F(config_test_c, Network)
{
    EXPECT_EQ(configuration0->communication_mode, eCAL_Config_IsNetworkEnabled());
}

TEST_F(config_test_c, Registration) 
{
    EXPECT_EQ(configuration0->registration.registration_timeout, eCAL_Config_GetRegistrationTimeoutMs());
    EXPECT_EQ(configuration0->registration.registration_timeout, eCAL_GetConfiguration()->registration.registration_timeout);
    EXPECT_EQ(configuration0->registration.registration_refresh, eCAL_Config_GetRegistrationRefreshMs());
    EXPECT_EQ(configuration0->registration.registration_refresh, eCAL_GetConfiguration()->registration.registration_refresh);
    //EXPECT_EQ(configuration0->registration.local.transport_type , eCAL_Config_IsShmRegistrationEnabled());
    EXPECT_STREQ(configuration0->registration.shm_transport_domain, eCAL_Config_GetShmTransportDomain());
    EXPECT_STREQ(configuration0->registration.shm_transport_domain, eCAL_GetConfiguration()->registration.shm_transport_domain);
    EXPECT_EQ(configuration0->registration.local.shm.queue_size, eCAL_Config_GetShmMonitoringQueueSize());
    EXPECT_EQ(configuration0->registration.local.shm.queue_size, eCAL_GetConfiguration()->registration.local.shm.queue_size);
    EXPECT_STREQ(configuration0->registration.local.shm.domain, eCAL_Config_GetShmMonitoringDomain());
    EXPECT_STREQ(configuration0->registration.local.shm.domain, eCAL_GetConfiguration()->registration.local.shm.domain);

    EXPECT_EQ(configuration0->registration.local.udp.port, eCAL_GetConfiguration()->registration.local.udp.port);
    EXPECT_EQ(configuration0->registration.loopback, eCAL_GetConfiguration()->registration.loopback);
    EXPECT_EQ(configuration0->registration.network.transport_type, eCAL_GetConfiguration()->registration.network.transport_type);
    EXPECT_EQ(configuration0->registration.network.udp.port, eCAL_GetConfiguration()->registration.network.udp.port);
    EXPECT_STREQ(configuration0->registration.shm_transport_domain, eCAL_GetConfiguration()->registration.shm_transport_domain);
}

TEST_F(config_test_c, TransportLayer) 
{
    EXPECT_EQ(configuration0->transport_layer.udp.config_version, eCAL_Config_GetUdpMulticastConfigVersion());
    EXPECT_EQ(configuration0->transport_layer.udp.config_version, eCAL_GetConfiguration()->transport_layer.udp.config_version);
    EXPECT_STREQ(configuration0->transport_layer.udp.network.group, eCAL_Config_GetUdpMulticastGroup());
    EXPECT_STREQ(configuration0->transport_layer.udp.network.group, eCAL_GetConfiguration()->transport_layer.udp.network.group);
    EXPECT_STREQ(configuration0->transport_layer.udp.mask, eCAL_Config_GetUdpMulticastMask());
    EXPECT_STREQ(configuration0->transport_layer.udp.mask, eCAL_GetConfiguration()->transport_layer.udp.mask);
    EXPECT_EQ(configuration0->transport_layer.udp.port, eCAL_Config_GetUdpMulticastPort());
    EXPECT_EQ(configuration0->transport_layer.udp.port, eCAL_GetConfiguration()->transport_layer.udp.port);
    EXPECT_EQ(configuration0->transport_layer.udp.network.ttl, eCAL_Config_GetUdpMulticastTtl());
    EXPECT_EQ(configuration0->transport_layer.udp.network.ttl, eCAL_GetConfiguration()->transport_layer.udp.network.ttl);
    EXPECT_EQ(configuration0->transport_layer.udp.send_buffer, eCAL_Config_GetUdpMulticastSndBufSizeBytes());
    EXPECT_EQ(configuration0->transport_layer.udp.send_buffer, eCAL_GetConfiguration()->transport_layer.udp.send_buffer);
    EXPECT_EQ(configuration0->transport_layer.udp.receive_buffer, eCAL_Config_GetUdpMulticastRcvBufSizeBytes());
    EXPECT_EQ(configuration0->transport_layer.udp.receive_buffer, eCAL_GetConfiguration()->transport_layer.udp.receive_buffer);
    EXPECT_EQ(configuration0->transport_layer.udp.join_all_interfaces, eCAL_Config_IsUdpMulticastJoinAllIfEnabled());
    EXPECT_EQ(configuration0->transport_layer.udp.join_all_interfaces, eCAL_GetConfiguration()->transport_layer.udp.join_all_interfaces);
    EXPECT_EQ(configuration0->transport_layer.udp.npcap_enabled, eCAL_Config_IsNpcapEnabled());
    EXPECT_EQ(configuration0->transport_layer.udp.npcap_enabled, eCAL_GetConfiguration()->transport_layer.udp.npcap_enabled);

    EXPECT_STREQ(configuration0->transport_layer.udp.local.group, eCAL_GetConfiguration()->transport_layer.udp.local.group);
    EXPECT_EQ(configuration0->transport_layer.udp.local.ttl, eCAL_GetConfiguration()->transport_layer.udp.local.ttl);

    EXPECT_EQ(configuration0->transport_layer.tcp.number_executor_reader, eCAL_Config_GetTcpPubsubReaderThreadpoolSize());
    EXPECT_EQ(configuration0->transport_layer.tcp.number_executor_reader, eCAL_GetConfiguration()->transport_layer.tcp.number_executor_reader);
    EXPECT_EQ(configuration0->transport_layer.tcp.number_executor_writer, eCAL_Config_GetTcpPubsubWriterThreadpoolSize());
    EXPECT_EQ(configuration0->transport_layer.tcp.number_executor_writer, eCAL_GetConfiguration()->transport_layer.tcp.number_executor_writer);
    EXPECT_EQ(configuration0->transport_layer.tcp.max_reconnections, eCAL_Config_GetTcpPubsubMaxReconnectionAttemps());
    EXPECT_EQ(configuration0->transport_layer.tcp.max_reconnections, eCAL_GetConfiguration()->transport_layer.tcp.max_reconnections);
}

TEST_F(config_test_c, Subscriber)
{
    EXPECT_EQ(configuration0->subscriber.layer.udp.enable, eCAL_Config_IsUdpMulticastRecEnabled());
    EXPECT_EQ(configuration0->subscriber.layer.udp.enable, eCAL_GetConfiguration()->subscriber.layer.udp.enable);
    EXPECT_EQ(configuration0->subscriber.layer.shm.enable, eCAL_Config_IsShmRecEnabled());
    EXPECT_EQ(configuration0->subscriber.layer.shm.enable, eCAL_GetConfiguration()->subscriber.layer.shm.enable);
    EXPECT_EQ(configuration0->subscriber.layer.tcp.enable, eCAL_Config_IsTcpRecEnabled());
    EXPECT_EQ(configuration0->subscriber.layer.tcp.enable, eCAL_GetConfiguration()->subscriber.layer.tcp.enable);
    EXPECT_EQ(configuration0->subscriber.drop_out_of_order_messages, eCAL_Config_GetDropOutOfOrderMessages());
    EXPECT_EQ(configuration0->subscriber.drop_out_of_order_messages, eCAL_GetConfiguration()->subscriber.drop_out_of_order_messages);
}

TEST_F(config_test_c, Time)
{
    EXPECT_STREQ(configuration0->timesync.timesync_module_rt, eCAL_Config_GetTimesyncModuleName());
    EXPECT_STREQ(configuration0->timesync.timesync_module_rt, eCAL_GetConfiguration()->timesync.timesync_module_rt);
    EXPECT_STREQ(configuration0->timesync.timesync_module_replay, eCAL_Config_GetTimesyncModuleReplay());
    EXPECT_STREQ(configuration0->timesync.timesync_module_replay, eCAL_GetConfiguration()->timesync.timesync_module_replay);
}

TEST_F(config_test_c, Application)
{
    EXPECT_STREQ(configuration0->application.startup.terminal_emulator, eCAL_Config_GetTerminalEmulatorCommand());
    EXPECT_STREQ(configuration0->application.startup.terminal_emulator, eCAL_GetConfiguration()->application.startup.terminal_emulator);
    EXPECT_STREQ(configuration0->application.sys.filter_excl, eCAL_Config_GetEcalSysFilterExcludeList());
    EXPECT_STREQ(configuration0->application.sys.filter_excl, eCAL_GetConfiguration()->application.sys.filter_excl);
}

TEST_F(config_test_c, Logging)
{
    EXPECT_EQ(configuration0->logging.provider.console.enable, eCAL_GetConfiguration()->logging.provider.console.enable);
    EXPECT_EQ(configuration0->logging.provider.console.log_level, eCAL_GetConfiguration()->logging.provider.console.log_level);
    EXPECT_EQ(configuration0->logging.provider.file.enable, eCAL_GetConfiguration()->logging.provider.file.enable);
    EXPECT_EQ(configuration0->logging.provider.file.log_level, eCAL_GetConfiguration()->logging.provider.file.log_level);
    EXPECT_STREQ(configuration0->logging.provider.file_config.path, eCAL_GetConfiguration()->logging.provider.file_config.path);
    EXPECT_EQ(configuration0->logging.provider.udp.enable, eCAL_GetConfiguration()->logging.provider.udp.enable);
    EXPECT_EQ(configuration0->logging.provider.udp.log_level, eCAL_GetConfiguration()->logging.provider.udp.log_level);
    EXPECT_EQ(configuration0->logging.provider.udp_config.port, eCAL_GetConfiguration()->logging.provider.udp_config.port);
    EXPECT_EQ(configuration0->logging.receiver.enable, eCAL_GetConfiguration()->logging.receiver.enable);
    EXPECT_EQ(configuration0->logging.receiver.udp_config.port, eCAL_GetConfiguration()->logging.receiver.udp_config.port);
}

TEST_F(config_test_c, Publisher)
{
    EXPECT_EQ(configuration0->publisher.layer.shm.acknowledge_timeout_ms, eCAL_GetConfiguration()->publisher.layer.shm.acknowledge_timeout_ms);
    EXPECT_EQ(configuration0->publisher.layer.shm.enable, eCAL_GetConfiguration()->publisher.layer.shm.enable);
    EXPECT_EQ(configuration0->publisher.layer.shm.memfile_buffer_count, eCAL_GetConfiguration()->publisher.layer.shm.memfile_buffer_count);
    EXPECT_EQ(configuration0->publisher.layer.shm.memfile_min_size_bytes, eCAL_GetConfiguration()->publisher.layer.shm.memfile_min_size_bytes);
    EXPECT_EQ(configuration0->publisher.layer.shm.memfile_reserve_percent, eCAL_GetConfiguration()->publisher.layer.shm.memfile_reserve_percent);
    EXPECT_EQ(configuration0->publisher.layer.shm.zero_copy_mode, eCAL_GetConfiguration()->publisher.layer.shm.zero_copy_mode);
    EXPECT_EQ(configuration0->publisher.layer.tcp.enable, eCAL_GetConfiguration()->publisher.layer.tcp.enable);
    EXPECT_EQ(configuration0->publisher.layer.udp.enable, eCAL_GetConfiguration()->publisher.layer.udp.enable);

    EXPECT_EQ(configuration0->publisher.layer_priority_local_length, eCAL_GetConfiguration()->publisher.layer_priority_local_length);
    EXPECT_EQ(configuration0->publisher.layer_priority_remote_length, eCAL_GetConfiguration()->publisher.layer_priority_remote_length);

    for (int i = 0; i < int(configuration0->publisher.layer_priority_local_length); i++)
    {
        EXPECT_EQ(configuration0->publisher.layer_priority_local[i], eCAL_GetConfiguration()->publisher.layer_priority_local[i]);
    }

    for (int i = 0; i < int(configuration0->publisher.layer_priority_remote_length); i++)
    {
        EXPECT_EQ(configuration0->publisher.layer_priority_remote[i], eCAL_GetConfiguration()->publisher.layer_priority_remote[i]);
    }
}