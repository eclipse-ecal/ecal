
# eCAL - enhanced Communication Abstraction Layer


[![Windows](https://github.com/eclipse-ecal/ecal/actions/workflows/build-windows.yml/badge.svg)](https://github.com/eclipse-ecal/ecal/actions/workflows/build-windows.yml) [![Ubuntu](https://github.com/eclipse-ecal/ecal/actions/workflows/build-ubuntu.yml/badge.svg)](https://github.com/eclipse-ecal/ecal/actions/workflows/build-ubuntu.yml) [![macOS](https://github.com/eclipse-ecal/ecal/actions/workflows/build-macos.yml/badge.svg)](https://github.com/eclipse-ecal/ecal/actions/workflows/build-macos.yml)

[![License](https://img.shields.io/github/license/continental/ecal.svg?style=flat)](LICENSE.txt)

The **e**nhanced **C**ommunication **A**bstraction **L**ayer (eCAL) is a middleware that enables scalable, high performance interprocess communication on a single computer node or between different nodes in a computer network.
eCAL provides **publish - subscribe** and **server - client** pattern to connect different nodes in the network with almost no configuration required.

eCAL automatically chooses the best available data transport mechanism for each link, it supports:
- **Shared memory** for local communication _(incredibly fast!)_
- **UDP** and **TCP** for network communication

Visit the eCAL Documentation at 🌐 https://ecal.io for more information.

## Architecture

![eCAL Architecture](doc/rst/getting_started/img/ecal_architecture.png)

## Facts about eCAL

* eCAL is fast (1 - 20 GB/s, depends on payload size. Check the measured performance [here](https://eclipse-ecal.github.io/ecal/advanced/performance.html))
* eCAL provides both publish-subscribe and server-client patterns
* eCAL is brokerless
* eCAL provides a C++ and C interface for easy integration into other languages (like python, csharp or [rust](https://github.com/kopernikusai/ecal-rs))
* eCAL can be used in conjunction with Matlab Simulink as [eCAL Simulink Toolbox](https://de.mathworks.com/matlabcentral/fileexchange/92825-ecal-toolbox) for simulation and prototyping
* eCAL has powerful tools for [recording](https://eclipse-ecal.github.io/ecal/getting_started/recorder.html), [replay](https://eclipse-ecal.github.io/ecal/getting_started/player.html) and [monitoring](https://eclipse-ecal.github.io/ecal/getting_started/monitor.html) all your data flows - decentralized
* eCAL is simple and zero-conf. No complex configuration for communication details and QOS settings are needed
* eCAL is message protocol agnostic. You choose the message protocol that fits to your needs like [Google Protobuf](https://developers.google.com/protocol-buffers), [CapnProto](https://capnproto.org/), [Flatbuffers](https://google.github.io/flatbuffers/)...
* eCAL uses the standardized recording format [HDF5](https://www.hdfgroup.org/solutions/hdf5/)
* eCAL integrates gently into your ROS2 environment with the [eCAL RMW](https://github.com/eclipse-ecal/rmw_ecal)
* eCAL supports Intel and arm platforms

* eCAL runs on a wide variety of operating systems:
  * Windows (stable)
  * Linux (stable)
  * QNX (experimental)
  * MacOS (experimental)
  * FreeBSD (experimental)

## Installation

We provide binary installers for Windows and Ubuntu. If you need further help installing and configuring eCAL, please refer to the [documentation](https://eclipse-ecal.github.io/ecal/getting_started/setup.html).

### Windows

[Download](https://eclipse-ecal.github.io/ecal/releases/) the latest eCAL Installer and follow the Setup Wizard

![eCAL Setup](doc/rst/getting_started/img/setup.png)
    
*We only support 64bit Windows 10 / 11*

### Ubuntu

Install eCAL from our PPA:

```bash
sudo add-apt-repository ppa:ecal/ecal-latest
sudo apt-get update
sudo apt-get install ecal
```
This PPA will always upgrade you to the latest eCAL Release (-> Rolling Release PPA). If you intend to stay on an specific release, check out other PPAs [here](https://eclipse-ecal.github.io/ecal/getting_started/setup.html#fa-ubuntu-automatically-install-ecal-from-a-ppa).

*Ubuntu 18.04, 20.04, 22.04, 24.04 for CPU architectures amd64, armhf, arm64 are supported at the time of writing. Non-LTS versions of Ubuntu are usually supported, too.*

## Example

Using eCAL in your project to exchange data is simple. After you have [downloaded eCAL](http://ecal.io) and installed CMake, you are good to go.

Check out the [Hello World](https://eclipse-ecal.github.io/ecal/getting_started/hello_world.html) example from the eCAL documentation for further details.

``` cpp
#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>

#include <thread>

int main(int argc, char** argv)
{
  // Initialize eCAL. The name of our process will be "Hello World Publisher"
  eCAL::Initialize(argc, argv, "Hello World Publisher");

  // Create a String Publisher that publishes on the topic "hello_world_topic"
  eCAL::string::CPublisher publisher("hello_world_topic");

  // Infinite loop
  while (eCAL::Ok())
  {
    // Publish a "Hello World" message
    publisher.Send("Hello World");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
```

## Tools from the eCAL ecosystem

eCAL comes with a set of read-to-use tools that will help you with developing, testing and debugging your software. **Command line interface** versions and easy to use **GUI** applications are available.

- The **eCAL Monitor** to visualize the data flow and inspect messages sent between publishers and subscribers 

- The **eCAL Recorder** to record the data sent between your eCAL nodes

- The **eCAL Player** to replay the eCAL recordings later on

- **eCAL Sys** to define your system configuration and monitor your applications

![eCAL Mon](gfx/app/monitor_imagevisu.png)

## eCAL eco system tools / utilities / interfaces
* [ecal-toolbox](https://github.com/mathworks/ecal-toolbox) - Mathworks simulink toolbox for eCAL
* [ecal-mongraph](https://github.com/ecal-io/ecal-mongraph) - Simple graph visualization for eCAL
* [ecal-gpsd-client](https://github.com/eclipse-ecal/ecal-gpsd-client) - eCAL gpsd client

## eCAL & Foxglove
* [ecal-foxglove-bridge](https://github.com/eclipse-ecal/ecal-foxglove-bridge) - Visualize eCAL messages with Foxglove Studio
* [ecal-mcap-tools](https://github.com/eclipse-ecal/ecal-mcap-tools) - Interoperability tools between eCAL HDF5 and MCAP measurement format

## eCAL & ROS
* [rmw_ecal](https://github.com/eclipse-ecal/rmw_ecal) - eCAL / ROS2 middleware layer
* [rosidl_typesupport_protobuf](https://github.com/eclipse-ecal/rosidl_typesupport_protobuf) - Protobuf based rosidl typesupport

## Other projects related to eCAL
* [fineftp-server](https://github.com/eclipse-ecal/fineftp-server) - FTP functionality to collect distributed measurement artifacts over network
* [tcp_pubsub](https://github.com/eclipse-ecal/tcp_pubsub) - Additional TCP transport layer for higher reliability for publication/subscription over network
* [udpcap](https://github.com/eclipse-ecal/udpcap) - Npcap based UDP-socket emulation to increase the eCAL performance on MS Windows
* [docker-ecal](https://github.com/Blutkoete/docker-ecal) - Inoffical dockerfiles for eCAL
* [golang-ecal](https://github.com/Blutkoete/golang-ecal) - eCAL go language binding
* [ecal-rs](https://github.com/kopernikusai/ecal-rs) - eCAL rust language binding
* [nodejs-ecal](https://www.npmjs.com/package/nodejs-ecal) - eCAL wrapped for Nodejs
* [trellis](https://github.com/agtonomy/trellis) - General purpose middleware framework for distributed applications
* [ADMA2eCAL](https://github.com/GeneSysElektronik/ADMA2eCAL) - Software gateway for connection to ADMA hardware from GeneSys Elekronik GmbH 

## License

eCAL is licensed under Apache License 2.0. You are free to

- Use eCAL commercially
- Modify eCAL
- Distribute eCAL

eCAL is provided on an “as is” basis without warranties or conditions of any kind.


## Contributors

<a href="https://github.com/eclipse-ecal/ecal/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=eclipse-ecal/ecal" />
</a>
