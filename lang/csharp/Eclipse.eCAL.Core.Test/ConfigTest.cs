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

/**
 * @file ConfigTest.cs
 * @brief Tests for type wrapper and initializing.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using Eclipse.eCAL.Core;
using Eclipse.eCAL.Core.Config;
using Microsoft.VisualStudio.TestTools.UnitTesting;

[TestClass]
public class ConfigTest
{
  private const int TcsWaitTime = 2500;

  [TestMethod]
  public void TestConfigPassing()
  {
    // Create config with every value set differently than the default values
    var config = new Eclipse.eCAL.Core.Config.Configuration();

    // Communication mode
    config.CommunicationMode = eCommunicationMode.Network;

    // Registration
    config.Registration.RegistrationRefresh = 500;
    config.Registration.RegistrationTimeout = 2000;
    config.Registration.Loopback = false;
    config.Registration.ShmTransportDomain = "shm_transport_domain";
    config.Registration.Local.TransportType = 0; // eCAL::Registration::Local::eTransportType::shm
    config.Registration.Local.SHM.Domain = "ecal_don";
    config.Registration.Local.SHM.QueueSize = 2048;
    config.Registration.Local.UDP.Port = 15000;
    config.Registration.Network.TransportType = 0; // eCAL::Registration::Network::eTransportType::udp
    config.Registration.Network.UDP.Port = 16000;

    // Transport Layer
    config.TransportLayer.Udp.ConfigVersion = TypesUdpConfigVersion.V1;
    config.TransportLayer.Udp.Port = 17000;
    config.TransportLayer.Udp.Mask = "255.254.254.242";
    config.TransportLayer.Udp.SendBuffer = 6242880;
    config.TransportLayer.Udp.ReceiveBuffer = 6242881;
    config.TransportLayer.Udp.JoinAllInterfaces = true;
    config.TransportLayer.Udp.NpcapEnabled = true;
    config.TransportLayer.Udp.Local.Group = "129.255.255.254";
    config.TransportLayer.Udp.Local.Ttl = 7;
    config.TransportLayer.Udp.Network.Group = "238.1.2.3";
    config.TransportLayer.Udp.Network.Ttl = 8;
    config.TransportLayer.Tcp.NumberExecutorReader = 9;
    config.TransportLayer.Tcp.NumberExecutorWriter = 10;
    config.TransportLayer.Tcp.MaxReconnections = 11;

    // Publisher
    config.Publisher.Layer.SHM.Enable = false;
    config.Publisher.Layer.SHM.ZeroCopyMode = true;
    config.Publisher.Layer.SHM.AcknowledgeTimeoutMs = 12;
    config.Publisher.Layer.SHM.MemfileBufferCount = 13;
    config.Publisher.Layer.SHM.MemfileMinSizeBytes = 8192;
    config.Publisher.Layer.SHM.MemfileReservePercent = 14;
    config.Publisher.Layer.UDP.Enable = false;
    config.Publisher.Layer.TCP.Enable = false;
    config.Publisher.LayerPriorityLocal.Clear();
    config.Publisher.LayerPriorityLocal.AddRange(new eTransportLayerType[] { eTransportLayerType.Tcp, eTransportLayerType.Shm, eTransportLayerType.UdpMc });
    config.Publisher.LayerPriorityRemote.Clear();
    config.Publisher.LayerPriorityRemote.AddRange(new eTransportLayerType[] { eTransportLayerType.Tcp, eTransportLayerType.UdpMc });

    // Subscriber
    config.Subscriber.Layer.SHM.Enable = false;
    config.Subscriber.Layer.UDP.Enable = false;
    config.Subscriber.Layer.TCP.Enable = true;
    config.Subscriber.DropOutOfOrderMessages = false;

    // TimeSync
    config.TimeSync.TimeSyncModuleReplay = "my_replay";
    config.TimeSync.TimeSyncModuleRT = "my_rt";

    // Application
    config.Application.Startup.TerminalEmulator = "term_emulator";
    config.Application.Sys.FilterExcl = "filter_excl";

    // Logging
    config.Logging.Provider.Console.Enable = false;
    config.Logging.Provider.Console.LogLevel = 1; // log_level_debug1
    config.Logging.Provider.File.Enable = true;
    config.Logging.Provider.File.LogLevel = 2 | 3; // log_level_debug2 | log_level_debug3
    config.Logging.Provider.UDP.Enable = false;
    config.Logging.Provider.UDP.LogLevel = 4; // log_level_debug4
    config.Logging.Provider.FileConfig.Path = "file_config_path";
    config.Logging.Provider.UDPConfig.Port = 18000;
    config.Logging.Receiver.Enable = true;
    config.Logging.Receiver.UDPConfig.Port = 19000;

    Core.Initialize(config, "TestConfigPassing", Init.Flags.Default);

    var ecalConfig = Config.GetConfiguration();

    // Communication mode
    Assert.AreEqual(config.CommunicationMode, ecalConfig.CommunicationMode, "CommunicationMode mismatch");

    // Registration
    Assert.AreEqual(config.Registration.RegistrationRefresh, ecalConfig.Registration.RegistrationRefresh, "RegistrationRefresh mismatch");
    Assert.AreEqual(config.Registration.RegistrationTimeout, ecalConfig.Registration.RegistrationTimeout, "RegistrationTimeout mismatch");
    Assert.AreEqual(config.Registration.Loopback, ecalConfig.Registration.Loopback, "Loopback mismatch");
    Assert.AreEqual(config.Registration.ShmTransportDomain, ecalConfig.Registration.ShmTransportDomain, "ShmTransportDomain mismatch");
    Assert.AreEqual(config.Registration.Local.TransportType, ecalConfig.Registration.Local.TransportType, "Local.TransportType mismatch");
    Assert.AreEqual(config.Registration.Local.SHM.Domain, ecalConfig.Registration.Local.SHM.Domain, "Local.SHM.Domain mismatch");
    Assert.AreEqual(config.Registration.Local.SHM.QueueSize, ecalConfig.Registration.Local.SHM.QueueSize, "Local.SHM.QueueSize mismatch");
    Assert.AreEqual(config.Registration.Local.UDP.Port, ecalConfig.Registration.Local.UDP.Port, "Local.UDP.Port mismatch");
    Assert.AreEqual(config.Registration.Network.TransportType, ecalConfig.Registration.Network.TransportType, "Network.TransportType mismatch");
    Assert.AreEqual(config.Registration.Network.UDP.Port, ecalConfig.Registration.Network.UDP.Port, "Network.UDP.Port mismatch");

    // Transport Layer
    Assert.AreEqual(config.TransportLayer.Udp.ConfigVersion, ecalConfig.TransportLayer.Udp.ConfigVersion, "Udp.ConfigVersion mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.Port, ecalConfig.TransportLayer.Udp.Port, "Udp.Port mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.Mask, ecalConfig.TransportLayer.Udp.Mask, "Udp.Mask mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.SendBuffer, ecalConfig.TransportLayer.Udp.SendBuffer, "Udp.SendBuffer mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.ReceiveBuffer, ecalConfig.TransportLayer.Udp.ReceiveBuffer, "Udp.ReceiveBuffer mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.JoinAllInterfaces, ecalConfig.TransportLayer.Udp.JoinAllInterfaces, "Udp.JoinAllInterfaces mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.NpcapEnabled, ecalConfig.TransportLayer.Udp.NpcapEnabled, "Udp.NpcapEnabled mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.Local.Group, ecalConfig.TransportLayer.Udp.Local.Group, "Udp.Local.Group mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.Local.Ttl, ecalConfig.TransportLayer.Udp.Local.Ttl, "Udp.Local.Ttl mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.Network.Group, ecalConfig.TransportLayer.Udp.Network.Group, "Udp.Network.Group mismatch");
    Assert.AreEqual(config.TransportLayer.Udp.Network.Ttl, ecalConfig.TransportLayer.Udp.Network.Ttl, "Udp.Network.Ttl mismatch");
    Assert.AreEqual(config.TransportLayer.Tcp.NumberExecutorReader, ecalConfig.TransportLayer.Tcp.NumberExecutorReader, "Tcp.NumberExecutorReader mismatch");
    Assert.AreEqual(config.TransportLayer.Tcp.NumberExecutorWriter, ecalConfig.TransportLayer.Tcp.NumberExecutorWriter, "Tcp.NumberExecutorWriter mismatch");
    Assert.AreEqual(config.TransportLayer.Tcp.MaxReconnections, ecalConfig.TransportLayer.Tcp.MaxReconnections, "Tcp.MaxReconnections mismatch");

    // Publisher
    Assert.AreEqual(config.Publisher.Layer.SHM.Enable, ecalConfig.Publisher.Layer.SHM.Enable, "Publisher.Layer.SHM.Enable mismatch");
    Assert.AreEqual(config.Publisher.Layer.SHM.ZeroCopyMode, ecalConfig.Publisher.Layer.SHM.ZeroCopyMode, "Publisher.Layer.SHM.ZeroCopyMode mismatch");
    Assert.AreEqual(config.Publisher.Layer.SHM.AcknowledgeTimeoutMs, ecalConfig.Publisher.Layer.SHM.AcknowledgeTimeoutMs, "Publisher.Layer.SHM.AcknowledgeTimeoutMs mismatch");
    Assert.AreEqual(config.Publisher.Layer.SHM.MemfileBufferCount, ecalConfig.Publisher.Layer.SHM.MemfileBufferCount, "Publisher.Layer.SHM.MemfileBufferCount mismatch");
    Assert.AreEqual(config.Publisher.Layer.SHM.MemfileMinSizeBytes, ecalConfig.Publisher.Layer.SHM.MemfileMinSizeBytes, "Publisher.Layer.SHM.MemfileMinSizeBytes mismatch");
    Assert.AreEqual(config.Publisher.Layer.SHM.MemfileReservePercent, ecalConfig.Publisher.Layer.SHM.MemfileReservePercent, "Publisher.Layer.SHM.MemfileReservePercent mismatch");
    Assert.AreEqual(config.Publisher.Layer.UDP.Enable, ecalConfig.Publisher.Layer.UDP.Enable, "Publisher.Layer.UDP.Enable mismatch");
    Assert.AreEqual(config.Publisher.Layer.TCP.Enable, ecalConfig.Publisher.Layer.TCP.Enable, "Publisher.Layer.TCP.Enable mismatch");
    CollectionAssert.AreEqual(config.Publisher.LayerPriorityLocal, ecalConfig.Publisher.LayerPriorityLocal, "Publisher.LayerPriorityLocal mismatch");
    CollectionAssert.AreEqual(config.Publisher.LayerPriorityRemote, ecalConfig.Publisher.LayerPriorityRemote, "Publisher.LayerPriorityRemote mismatch");

    // Subscriber
    Assert.AreEqual(config.Subscriber.Layer.SHM.Enable, ecalConfig.Subscriber.Layer.SHM.Enable, "Subscriber.Layer.SHM.Enable mismatch");
    Assert.AreEqual(config.Subscriber.Layer.UDP.Enable, ecalConfig.Subscriber.Layer.UDP.Enable, "Subscriber.Layer.UDP.Enable mismatch");
    Assert.AreEqual(config.Subscriber.Layer.TCP.Enable, ecalConfig.Subscriber.Layer.TCP.Enable, "Subscriber.Layer.TCP.Enable mismatch");
    Assert.AreEqual(config.Subscriber.DropOutOfOrderMessages, ecalConfig.Subscriber.DropOutOfOrderMessages, "Subscriber.DropOutOfOrderMessages mismatch");

    // TimeSync
    Assert.AreEqual(config.TimeSync.TimeSyncModuleReplay, ecalConfig.TimeSync.TimeSyncModuleReplay, "TimeSync.TimeSyncModuleReplay mismatch");
    Assert.AreEqual(config.TimeSync.TimeSyncModuleRT, ecalConfig.TimeSync.TimeSyncModuleRT, "TimeSync.TimeSyncModuleRT mismatch");

    // Application
    Assert.AreEqual(config.Application.Startup.TerminalEmulator, ecalConfig.Application.Startup.TerminalEmulator, "Application.Startup.TerminalEmulator mismatch");
    Assert.AreEqual(config.Application.Sys.FilterExcl, ecalConfig.Application.Sys.FilterExcl, "Application.Sys.FilterExcl mismatch");

    // Logging
    Assert.AreEqual(config.Logging.Provider.Console.Enable, ecalConfig.Logging.Provider.Console.Enable, "Logging.Provider.Console.Enable mismatch");
    Assert.AreEqual(config.Logging.Provider.Console.LogLevel, ecalConfig.Logging.Provider.Console.LogLevel, "Logging.Provider.Console.LogLevel mismatch");
    Assert.AreEqual(config.Logging.Provider.File.Enable, ecalConfig.Logging.Provider.File.Enable, "Logging.Provider.File.Enable mismatch");
    Assert.AreEqual(config.Logging.Provider.File.LogLevel, ecalConfig.Logging.Provider.File.LogLevel, "Logging.Provider.File.LogLevel mismatch");
    Assert.AreEqual(config.Logging.Provider.UDP.Enable, ecalConfig.Logging.Provider.UDP.Enable, "Logging.Provider.UDP.Enable mismatch");
    Assert.AreEqual(config.Logging.Provider.UDP.LogLevel, ecalConfig.Logging.Provider.UDP.LogLevel, "Logging.Provider.UDP.LogLevel mismatch");
    Assert.AreEqual(config.Logging.Provider.FileConfig.Path, ecalConfig.Logging.Provider.FileConfig.Path, "Logging.Provider.FileConfig.Path mismatch");
    Assert.AreEqual(config.Logging.Provider.UDPConfig.Port, ecalConfig.Logging.Provider.UDPConfig.Port, "Logging.Provider.UDPConfig.Port mismatch");
    Assert.AreEqual(config.Logging.Receiver.Enable, ecalConfig.Logging.Receiver.Enable, "Logging.Receiver.Enable mismatch");
    Assert.AreEqual(config.Logging.Receiver.UDPConfig.Port, ecalConfig.Logging.Receiver.UDPConfig.Port, "Logging.Receiver.UDPConfig.Port mismatch");

    Core.Terminate();
  }

  [TestMethod]
  public void TestPubSubConfig()
  {
    Core.Initialize("TestPubSubConfig");

    // Setting up SHM publisher and subscriber
    var publisherConfiguration = Config.GetPublisherConfiguration();
    publisherConfiguration.Layer.SHM.Enable = true;
    publisherConfiguration.Layer.UDP.Enable = false;
    publisherConfiguration.Layer.TCP.Enable = false;

    var subscriberConfiguration = Config.GetSubscriberConfiguration();
    subscriberConfiguration.Layer.SHM.Enable = true;
    subscriberConfiguration.Layer.UDP.Enable = false;
    subscriberConfiguration.Layer.TCP.Enable = false;

    Publisher publisherSHM = new Publisher("PubSubConfig", null, null, publisherConfiguration);
    Subscriber subscriberSHM = new Subscriber("PubSubConfig", null, null, subscriberConfiguration);

    var tcsSHM = new TaskCompletionSource<(string message, long timestamp)>();

    subscriberSHM.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      tcsSHM.TrySetResult((received, data.SendTimestamp));
    });

    // Setting up UDP publisher and subscriber
    publisherConfiguration.Layer.SHM.Enable = false;
    publisherConfiguration.Layer.UDP.Enable = true;
    publisherConfiguration.Layer.TCP.Enable = false;

    subscriberConfiguration.Layer.SHM.Enable = false;
    subscriberConfiguration.Layer.UDP.Enable = true;
    subscriberConfiguration.Layer.TCP.Enable = false;

    Publisher publisherUDP = new Publisher("PubSubConfig", null, null, publisherConfiguration);
    Subscriber subscriberUDP = new Subscriber("PubSubConfig", null, null, subscriberConfiguration);

    var tcsUDP = new TaskCompletionSource<(string message, long timestamp)>();

    subscriberUDP.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      tcsUDP.TrySetResult((received, data.SendTimestamp));
    });

    // Setting up TCP publisher and subscriber
    publisherConfiguration.Layer.SHM.Enable = false;
    publisherConfiguration.Layer.UDP.Enable = false;
    publisherConfiguration.Layer.TCP.Enable = true;

    subscriberConfiguration.Layer.SHM.Enable = false;
    subscriberConfiguration.Layer.UDP.Enable = false;
    subscriberConfiguration.Layer.TCP.Enable = true;

    Publisher publisherTCP = new Publisher("PubSubConfig", null, null, publisherConfiguration);
    Subscriber subscriberTCP = new Subscriber("PubSubConfig", null, null, subscriberConfiguration);

    var tcsTCP = new TaskCompletionSource<(string message, long timestamp)>();

    subscriberTCP.SetReceiveCallback((publisherId, dtInfo, data) =>
    {
      string received = Encoding.UTF8.GetString(data.Buffer);
      tcsTCP.TrySetResult((received, data.SendTimestamp));
    });

    string messageSHM = "Sending via SHM";
    string messageUDP = "Sending via UDP";
    string messageTCP = "Sending via TCP";

    // Sleep a moment for connections to be established
    Thread.Sleep(2000);

    publisherSHM.Send(Encoding.UTF8.GetBytes(messageSHM));

    if (!tcsSHM.Task.Wait(TcsWaitTime))
    {
      Assert.Fail("Timeout waiting for SHM message.");
    }

    var resultSHM = tcsSHM.Task.Result;

    Assert.IsTrue(tcsSHM.Task.IsCompleted, "SHM Task not completed.");
    Assert.AreEqual(messageSHM, resultSHM.message, "SHM Received message mismatch");

    // Check that UDP and TCP was not called
    Assert.IsFalse(tcsUDP.Task.IsCompleted, "UDP callback should not have been called.");
    Assert.IsFalse(tcsTCP.Task.IsCompleted, "TCP callback should not have been called.");

    publisherUDP.Send(Encoding.UTF8.GetBytes(messageUDP));

    if (!tcsUDP.Task.Wait(TcsWaitTime))
    {
      Assert.Fail("Timeout waiting for UDP message.");
    }

    var resultUDP = tcsUDP.Task.Result;
    Assert.IsTrue(tcsUDP.Task.IsCompleted, "UDP Task not completed.");
    Assert.AreEqual(messageUDP, resultUDP.message, "UDP received message mismatch.");

    // Check that TCP was not called
    Assert.IsFalse(tcsTCP.Task.IsCompleted, "TCP callback should not have been called.");

    publisherTCP.Send(Encoding.UTF8.GetBytes(messageTCP));

    if (!tcsTCP.Task.Wait(TcsWaitTime))
    {
      Assert.Fail("Timeout waiting for TCP message.");
    }

    var resultTCP = tcsTCP.Task.Result;
    Assert.IsTrue(tcsTCP.Task.IsCompleted, "TCP Task not completed.");
    Assert.AreEqual(messageTCP, resultTCP.message, "TCP received message mismatch.");

    Core.Terminate();
  }

  [TestMethod]
  public void TestConfigConvenienceGetter()
  {
    var ecalConfig = Config.GetConfiguration();

    // Now check all convenience getters against the configuration object
    Assert.AreEqual(ecalConfig.Registration.RegistrationTimeout, Config.GetRegistrationTimeoutMs(), "RegistrationTimeout");
    Assert.AreEqual(ecalConfig.Registration.RegistrationRefresh, Config.GetRegistrationRefreshMs(), "RegistrationRefresh");
    Assert.AreEqual(ecalConfig.CommunicationMode == eCommunicationMode.Local, Config.IsNetworkEnabled(), "IsNetworkEnabled");
    Assert.AreEqual(ecalConfig.TimeSync.TimeSyncModuleRT, Config.GetTimesyncModuleName(), "TimeSyncModuleRT");
    Assert.AreEqual(ecalConfig.TimeSync.TimeSyncModuleReplay, Config.GetTimesyncModuleReplay(), "TimeSyncModuleReplay");
    Assert.AreEqual(ecalConfig.Application.Startup.TerminalEmulator, Config.GetTerminalEmulatorCommand(), "TerminalEmulator");
    Assert.AreEqual(ecalConfig.Application.Sys.FilterExcl, Config.GetEcalSysFilterExcludeList(), "FilterExcl");
    Assert.AreEqual(ecalConfig.Subscriber.DropOutOfOrderMessages, Config.GetDropOutOfOrderMessages(), "DropOutOfOrderMessages");
    Assert.AreEqual(ecalConfig.Registration.Local.SHM.QueueSize, Config.GetShmMonitoringQueueSize(), "ShmMonitoringQueueSize");
    Assert.AreEqual(ecalConfig.Registration.Local.SHM.Domain, Config.GetShmMonitoringDomain(), "ShmMonitoringDomain");
    Assert.AreEqual(ecalConfig.Registration.Local.TransportType == 0 && ecalConfig.CommunicationMode == 0, Config.IsShmRegistrationEnabled(), "IsShmRegistrationEnabled");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.ConfigVersion, Config.GetUdpMulticastConfigVersion(), "UdpMulticastConfigVersion");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.Network.Group, Config.GetUdpMulticastGroup(), "UdpMulticastGroup");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.Mask, Config.GetUdpMulticastMask(), "UdpMulticastMask");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.Port, Config.GetUdpMulticastPort(), "UdpMulticastPort");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.Network.Ttl, Config.GetUdpMulticastTtl(), "UdpMulticastTtl");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.SendBuffer, Config.GetUdpMulticastSndBufSizeBytes(), "UdpMulticastSndBufSizeBytes");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.ReceiveBuffer, Config.GetUdpMulticastRcvBufSizeBytes(), "UdpMulticastRcvBufSizeBytes");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.JoinAllInterfaces, Config.IsUdpMulticastJoinAllIfEnabled(), "IsUdpMulticastJoinAllIfEnabled");
    Assert.AreEqual(ecalConfig.Subscriber.Layer.UDP.Enable, Config.IsUdpMulticastRecEnabled(), "IsUdpMulticastRecEnabled");
    Assert.AreEqual(ecalConfig.Subscriber.Layer.SHM.Enable, Config.IsShmRecEnabled(), "IsShmRecEnabled");
    Assert.AreEqual(ecalConfig.Subscriber.Layer.TCP.Enable, Config.IsTcpRecEnabled(), "IsTcpRecEnabled");
    Assert.AreEqual(ecalConfig.TransportLayer.Udp.NpcapEnabled, Config.IsNpcapEnabled(), "IsNpcapEnabled");
    Assert.AreEqual(ecalConfig.TransportLayer.Tcp.NumberExecutorReader, Config.GetTcpPubsubReaderThreadpoolSize(), "GetTcpPubsubReaderThreadpoolSize");
    Assert.AreEqual(ecalConfig.TransportLayer.Tcp.NumberExecutorWriter, Config.GetTcpPubsubWriterThreadpoolSize(), "GetTcpPubsubWriterThreadpoolSize");
    Assert.AreEqual(ecalConfig.TransportLayer.Tcp.MaxReconnections, Config.GetTcpPubsubMaxReconnectionAttemps(), "GetTcpPubsubMaxReconnectionAttemps");
    Assert.AreEqual(ecalConfig.Registration.ShmTransportDomain, Config.GetShmTransportDomain(), "GetShmTransportDomain");
  }
}
