# eCAL - enhanced Communication Abstraction Layer

Copyright (c) 2019, Continental Corporation.

[![Build Status](https://travis-ci.org/continental/ecal.svg?branch=master)](https://travis-ci.org/continental/ecal)
[![Build status](https://ci.appveyor.com/api/projects/status/2hnl7p9cedimrkox/branch/master?svg=true)](https://ci.appveyor.com/project/rex-schilasky/ecal/branch/master)
[![License](https://img.shields.io/github/license/continental/ecal.svg?style=flat)](LICENSE.txt)

## Preface

The enhanced communication abstraction layer (eCAL) is a middleware that enables scalable, high performance interprocess communication on a single computer node or between different nodes in a computer network. The design is inspired by known Data Distribution Service for Real-Time Systems (see Data distribution service on wikipedia). The current eCAL implementation realizes a subset of such a DDS system, there is only a basic support for Quality of Service (QoS) driven data transport (best effort and reliable).

eCAL is designed for typical cloud computing scenarios where different processes exchange their I/O's using a publisher/subscriber pattern. The data exchange is based on so called topics. A topic wraps the payload that should be exchanged with
additional informations like a unique name, a type and a description. A topic can be connected to more than one publisher and/or subscriber. These are the basic elements of the eCAL API.

  • Topic: The most basic description of the data to be published and subscribed.

  • Publisher: A Publisher is the object responsible for the actual dissemination of publications.

  • Subscriber: A Subscriber is the object responsible for the actual reception of the data resulting from its subscriptions.

  • Service: A collection of methods with arguments (requests) and return types (responses) realized as a server to be called from connected clients.

  • Client: Remote interface to a specific service (server) to call it's methods.

  • Callback: A Callback can be used to react on time events, on incoming messages, on service requests or service responses.

eCAL is simplifying the data transport as much as possible, It uses different mechanism to transport a topic from a publisher to a connected subscriber. On the same computer node the data are exchanged by using memory mapped files. Between different computing nodes UDP multicast can be used for high performance data throughput.


## Checkout the repository

Because eCAL is using some thirdparty libraries as git submodules you need to clone the repository recursively

```bash
git clone --recursive git://github.com/continental/ecal.git
```

For older git versions or if the repo is already cloned you can also use
```bash
git clone git://github.com/continental/ecal.git
cd ecal
git submodule init
git submodule update
```

## (Optional) Resolve dependencies using Conan

eCAL provides basic support for the [Conan](https://conan.io/) package manager that uses (re)packaged
thirdparty dependencies to allow for building eCAL using Conan non-intrusively. In order to build this
project using Conan, one has to install this package manager first and also add so called remotes that
point to pre-built thirdparty dependencies:

```bash
# Install Conan
pip install --upgrade pip
pip install --upgrade conan
conan config set general.revisions_enabled=True

# Add Conan remotes with pre-compiled dependencies
conan remote add -f kwc_bintray https://api.bintray.com/conan/kwc/conan
conan remote add -f conan-center https://bintray.com/conan/conan-center
conan remote add -f bincrafters https://api.bintray.com/conan/bincrafters/public-conan
```

## CMake build options

eCAL is using CMake as build system. When configuring with CMake, you can turn on / off the following features.

| CMake option                     | Default | Description |
|----------------------------------|---------|------------ |
| `HAS_HDF5`                       | `ON`    | Platform supports HDF5 library, necessary to build eCAL recording / replay tools |
| `HAS_QT5`                        | `ON`    | Platform supports Qt 5 library, necessary to build eCAL monitoring tool |
| `HAS_CURL`                       | `ON`    | Build with CURL (i.e. upload support in the recorder app) |
| `HAS_CAPNPROTO`                  | `OFF`   | Platform supports Cap'n Proto library, necessary to use capnp serialization as message system and to enable eCAL monitoring capnp message reflection. eCAL does not add Cap'n Proto as a submodule. If you set this option to `ON`, please make sure that the library is installed on your system and CMake can find it (consider setting CMAKE_PREFIX_PATH to point to the library). |
| `BUILD_DOCS`                     | `OFF`   | Build the eCAL documentation, requires the installation of doxygen and a recent CMake version (>= 3.14 preferred, but some lower versions might work) |
| `BUILD_APPS`                     | `ON`    | Build the eCAL applications, such as the monitoring tool |
| `BUILD_SAMPLES`                  | `OFF`   | Build the eCAL sample applications |
| `BUILD_TIME`                     | `ON`    | Build the eCAL time interfaces, necessary if you want to use ecal in time synchronized mode (based on ptp for example) |
| `BUILD_PY_BINDING`               | `OFF`   | Build the eCAL python language binding |
| `BUILD_CSHARP_BINDING`           | `OFF`   | Build the eCAL C# language binding |
| `BUILD_TESTS`                    | `OFF`   | Build the eCAL google tests |
| `ECAL_LAYER_ICEORYX`             | `OFF`   | Use iceoryx shared memory as local communication layer, requires [eclipse/iceoryx](https://github.com/eclipse/iceoryx) installation |
| `ECAL_INCLUDE_PY_SAMPLES`        | `OFF`   | Include python language sample projects into CMake |
| `ECAL_INSTALL_SAMPLE_SOURCES`    | `ON`    | Install the sources of eCAL samples |
| `ECAL_JOIN_MULTICAST_TWICE`      | `OFF`   | Specific multicast network bug workaround |
| `ECAL_NPCAP_SUPPORT`             | `OFF`   | Enable the eCAL to use Npcap for udp socket communication (i.e. the Win10 performance fix) |
| `ECAL_THIRDPARTY_BUILD_PROTOBUF` | `ON`    | Build Protobuf with eCAL, included as a submodule in the thirdparty folder. You can always use your custom protobuf installation, this is only for convenience. Note, at least protobuf 3.0 is required to compile eCAL, we recommend using 3.11.4 or newer (tested with 3.11.4). |
| `ECAL_THIRDPARTY_BUILD_SPDLOG`   | `ON`    | Build Spdlog with eCAL, included as a submodule in the thirdparty folder. You can always use your custom spdlog installation, this is only for convenience. |
| `ECAL_THIRDPARTY_BUILD_TINYXML2` | `ON`    | Build tinyxml2 with eCAL, included as a submodule in the thirdparty folder. |
| `ECAL_THIRDPARTY_BUILD_FINEFTP`  | `ON`    | Build fineFTP with eCAL, included as a submodule in the thirdparty folder. |
| `ECAL_THIRDPARTY_BUILD_CURL`     | `ON`    | Build CURL with eCAL, included as a submodule in the thirdparty folder. |
| `ECAL_THIRDPARTY_BUILD_GTEST`    | `OFF`   | Build GoogleTest with eCAL, included as a submodule in the thirdparty folder. You can always use your custom gtest installation, this is only for convenience. |
| `ECAL_THIRDPARTY_BUILD_HDF5`     | `ON`    | Build HDF5 with eCAL, included as a submodule in the thirdparty folder. |

All options can be passed on the command line `cmake -D<option>=<value>` or in the CMake GUI application.

## Setup on Linux Systems

### Dependencies on Ubuntu 16.04
1. Add the [official cmake repository](https://apt.kitware.com/), as eCAL needs cmake >= 3.13:
	```bash
	wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
	sudo apt-add-repository -y 'deb https://apt.kitware.com/ubuntu/ xenial main'
	sudo apt-get -y update
	sudo apt-get -y install kitware-archive-keyring
	sudo apt-key --keyring /etc/apt/trusted.gpg del C1F34CDD40CD72DA
	```

2. Add a ppa for protobuf >= 3.1. The following (unofficial) ppa will be sufficient:
	```bash
	sudo add-apt-repository -y ppa:maarten-fonville/protobuf
	sudo apt-get -y update
	```

3. Install the dependencies from the ordinary Ubuntu 16.04 repositories and the ppa we just added:
	```bash
	sudo apt-get -y install git cmake doxygen graphviz build-essential zlib1g-dev qt5-default libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev
	```

### Dependencies on Ubuntu 18.04
1. Add the [official cmake repository](https://apt.kitware.com/), as eCAL needs cmake >= 3.13:
	```bash
	wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
	sudo apt-add-repository -y 'deb https://apt.kitware.com/ubuntu/ bionic main'
	sudo apt-get -y update
	sudo apt-get -y install kitware-archive-keyring
	sudo apt-key --keyring /etc/apt/trusted.gpg del C1F34CDD40CD72DA
	```

2. Install all dependencies:
	```bash
	sudo apt-get -y install git cmake doxygen graphviz build-essential zlib1g-dev qt5-default libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev
	```

3. If you plan to create the eCAL python language extension (here as an example for the python 3.6 version):
	```bash
	sudo apt-get install python3.6-dev
	sudo apt-get install python3-pip
	python3 -m pip install setuptools
	```

### Compile eCAL
1. Check out the repository as described [here](#checkout-the-repository).

2. Compile eCAL:
_(Disable the `THIRDPARTY_BUILD_PROTOBUF` cmake option to not get a conflict with your installed protobuf version!)_
	```bash
	mkdir _build
	cd _build
	cmake .. -DCMAKE_BUILD_TYPE=Release -DECAL_THIRDPARTY_BUILD_PROTOBUF=OFF -DECAL_THIRDPARTY_BUILD_CURL=OFF -DECAL_THIRDPARTY_BUILD_HDF5=OFF
	make -j4
	```
	
3. Create a debian package and install it, if desired:
	```bash
	cpack -G DEB
	sudo dpkg -i _deploy/eCAL-*
	```
	
4. Create and install the eCAL python egg:
_(Enable the `BUILD_PY_BINDING` cmake option in step 2 (`-DBUILD_PY_BINDING=ON`) to build the eCAL core including the python language interface)_
	```bash
	cmake --build . --target create_python_egg --config Release
	sudo python3 -m easy_install _deploy/ecal-*
	```

### UDP network configuration

#### Setup the ip address

First you need to setup the correct ip address for sure - here for adapter eth0, ip address 192.168.0.1

```bash
sudo ifconfig eth0 192.168.0.1 netmask 255.255.255.0
```

Restart the ethernet interface or the whole machine to apply changes, check if you can ping to each other.

#### Setup the multicast routes

After the ip configuration you need to setup the multicast route for udp multicasting

```bash
ifconfig eth0 multicast
route add -net 239.0.0.0 netmask 255.0.0.0 dev eth0
```

Replace eth0 with lo for local communication only.

IF you want to fix these settings for the next system startup you can put the above 2 lines into the interfaces config file.

```bash
sudo gedit /etc/network/interfaces
```

## Setup on Windows Systems

### Install dependencies

1. Download dependencies. For creating the Visual Studio eCAL solution, build it and create a setup, you need:
	- CMake (https://cmake.org)
	- Doxygen (http://www.doxygen.nl)
	- Qt5 (>= 5.5) (https://www.qt.io/download)
	- WIX (http://wixtoolset.org/)

2. Install Qt5 by starting the installer and selecting `msvc2015 32-bit` or `msvc2015 64-bit` (VS2015) or `msvc2017 32-bit` and `msvc2017 64-bit` (VS2017) from the latest Qt5 version. Create an environment variable `QT5_ROOT_DIRECTORY` that points to the directory containing the architecture-specific folders. It should look like this:
	```
	 %QT5_ROOT_DIRECTORY%
	   ├ msvc2015
	   ├ msvc2015_64
	   ├ msvc2017
	   └ msvc2017_64
	```
	e.g.:
	```
	QT5_ROOT_DIRECTORY = C:\Qt\5.11.1
	```

4. Install [Python for Windows](https://www.python.org/downloads/) (64 Bit, Version 3.x) if you plan to build the eCAl python language extension.

5. Checkout the eCAL repository as described [here](#checkout-the-repository). Note that it has **submodules**, so use [Git for Windows](https://git-scm.com/download/win) to check out the repo.

#### Build eCAL

Run the following batch files to create the Visual Studio 2015 (2017, 2019) solutions for 32 and 64 bit and to build both.

```bat
build_win\win_make_cmake.bat v140 (v141, v142)
build_win\win_make_build.bat
```

#### Create Setup

Run the following batch files to create the eCAL SDK (third party libraries used by eCAL language support) and finally to create an eCAL windows setup file. The first batch file needs to be run only once and if you update your eCAL dependencies repo.

```bat
build_win\win_make_setup.bat
```

If you installed all mentioned tools and dependencies correctly you can also simply run

```bat
win_make_all.bat
```

to cmake eCAL build all configurations and create an installer. You will find the installer in the _deploy subfolder inside your build folder finally.

#### Create eCAL python extension

If you configured cmake to build the python extension by setting `BUILD_PY_BINDING` to `ON` you can create the ecal python egg by calling

```bat
build_win\win_make_python_egg.bat
```

Afterwards you will find the python eCAL egg in the _deploy subfolder inside your build folder. Install the extesnion by

```bat
python -m easy_install ecal-X.Y.Z-pyX.Y.egg
```

#### Create eCAL csharp extension

To build the eCAL csharp extension you need to set the CMake option `BUILD_CSHARP_BINDING` to `ON`. In order to make the CSharp Google::Protobuf extesnsion work you need to install additionally the Microsoft package management tool [Nuget](https://www.nuget.org/downloads). Please
ensure that the installation path is part of your windows user or system PATH environment variable. Nuget will be used to download the .Net Google.Protobuf package automatically when building the extension.

### UDP network configuration

#### Setup the multicast routes

You need to open a command window with administrative (!) privileges. Then run

```bat
route -p add 239.0.0.0 mask 255.0.0.0 192.x.x.x
  (192.x.x.x = ip address of the adapter to route udp to)
```

You can check the result using

```bat
route print
```

You can delete the route if needed using

```bat
route delete 239.0.0.0 mask 255.0.0.0 192.x.x.x
```

In the ecal.ini file configure the udp ttl (time to live) parameter according your needs (local / cloud). If you want to
communicate in a network set a value greater then 1 (depends how many network devices should be passed).
For local communication use 0 for ttl.

You can find the ecal.ini configuration file under %APPDATA%\eCAL.

Don't forget to disable any windows firewall.


#### Network performance issue on Windows 10 Systems - Npcap

##### Background

The "normal" eCAL Multicast communication may lead to low performance on Windows 10. This is not an eCAL issue but related to the Windows Firewall Service which cannot be deactivated since Win 10, any more. eCAL however includes an option to work around that issue. For that you have to install the Npcap driver and configure eCAL to use it.

##### How to use Npcap

  1. Download Npcap: https://nmap.org/npcap/

  2. Install Npcap with default Options:

  ![Npcap installer'](gfx/configuration/npcap_installer.png?raw=true "Npcap installer")

  3. Remove all elements / protocols except NPCAP and NPF from the Npcap Loopback Adapter

   **This is important, as you may not be able to connect to your corporate Wifi any more, if you skip this step!**

   The NPF element is only available if you installed Npcap in WinPcap compatibility mode

  ![Npcap settings'](gfx/configuration/npcap_settings.png?raw=true "Npcap settings")

  4. Edit `[C:/Program Data/eCAL/]ecal.ini`:

```
   npcap_enabled = true
```

  5. Check eCAL Mon

  ![Npcap eCALMon'](gfx/configuration/npcap_ecalmon.png?raw=true "Npcap eCALMon")


##### Troubleshooting

If you enable Npcap but didn't install it, installed it with the wrong settings or your installation is broken, eCAL Mon will tell you that the Npcap Initialization has failed. eCAL will still work, but not use Npcap.

Npcap seems to break itself if you reinstall it, as the uninstaller does not remove the Loopback Adapter properly. The new Npcap installation will fail to create a new Loopback Adapter and the installation will be corrupt!

Please do a clean install of Npcap:

  1. Uninstall Npcap

  2. Manually uninstall all Npcap Loopback Adapters using the Windows device manager.

  3. Manually uninstall all Microsoft KM-TEST Loopback adapters with the device manager

  4. Install Npcap again with the settings above


## Transport Layer Concept

eCAL is able to communicate on different so called transport layers, but first of all you have to decide if you want to communicate in a network or in a local host only mode. To configure this you need to set the ecal.ini [network/network_enabled] parameter to true (network communication mode) or false (local host only communication mode).

After this you can fine tune the way of message transport for inner-process, interprocess and the interhost connections.
There are different ways to configure these layers. They can be set up for a whole machine using the central configuration file (ecal.ini) or for a single eCAL process passed by command line arguments or finally for a single publish-subscribe connection using the C++ or python publisher API (see "Setup the transport layer"). Every single builtin transport layer has it's specific communication properties.

| Layer           | Ini parameter            | Physical Layer     | Comment                                                                                   |
|-----------------|--------------------------|--------------------|-------------------------------------------------------------------------------------------|
| inproc          | [publisher/use_inproc]   | inner process      | inner process, zeroy copy communication (pointer forwarding)                              |
| shm             | [publisher/use_shm]      | shared memory      | interprocess, shared memory communication, supports N:M connections, 2 memory copies      |
| udp_mc          | [publisher/use_udp_mc]   | udp multicast      | interhost, topic name based dynamic multicast grouping to optimize pub/sub socket payload |

Every layer can set up in 3 different activation modes. Every mode can be configured as default in the ecal.ini file and can be overwritten by the C++/Python publisher API. This is the activation logic

  • off: layer is swicthed off

  • on: layer is always switched on (i.e. payload will be send no matter if there is any local or network subscription)

  • auto: layer will be switched on autmatically

 - inproc = 2 : layer used automatically for inner process subscribers

 - shm = 2 : layer used automatically for inter process subscribers

 - udp_mc = 2 : layer used autmatically for inter host (network) subcribers

Independent from this publisher setting you can switch on/off the receiving (subscription) logic for every layer. That means you can prevent incoming payload on specific layers. This can be done in the ecal.ini file [network] section.

  • inproc_rec_enabled = true / false : enable / disable inner process subscriptions

  • shm_rec_enabled = true / false : enable / disable inter process subscriptions

  • udp_mc_rec_enabled = true / false : enable / disable inter host subscriptions

## Applications

Besides the communication core eCAL comes with some high-level applications for monitoring, recording and replaying messages. The monitoring application is used to provide an overview of all existing entities (publisher, subscriber, services) that are using the eCAL API to communicate. Recorder and player are designed to record messages in a distributed system efficiently and to replay them for post processing, analysis or simulation.

### Monitor

To start the monitor please open the application from the windows start menu or type
`ecal_mon_gui` on a linux system.

The monitor provides different kinds of sorted views for all eCAL publications, subscriptions or service instances. The content of messages is visualized with plugins. Currently the monitor is shipped with ready to use plugins for two serialization formats ([google protobuf](https://developers.google.com/protocol-buffers) and [capnproto](https://capnproto.org/).), as well as simple string data.

The image shows an eCAL monitor showing two running application "person publisher" and "person subscriber". The content of the protobuf person message is shown in the protobuf reflection plugin (just double click on the topic name to open the reflection window).

![eCAL monitor showing topic 'person'](gfx/app/monitor_person.png?raw=true "eCAL monitor showing topic 'person'")

The plugin concept also allows anybody to develop 2D or 3D visualization plugins for specific data types and messages. The plugins are developed with Qt5 and the eCAL Monitor Plugin SDK. On start the eCAL Monitor will scan the plugin folder for new plugins and load them. The following image shows a plugin visualizing a camera image.

![eCAL monitor showing image](gfx/app/monitor_imagevisu.png?raw=true "eCAL monitor showing image")

### Recorder

Recording eCAL messages is essential for many use cases. A Recording can be used as simulated input for other applications, for debugging and analysis. If required, the recorded data can be post-processed in C++ (python and matlab wrappers will be released with a future release).

The recorded data is stored in the standardized [HDF5](https://www.hdfgroup.org/) data format, a portable, cross platform, hierarchical container format, well designed for large data sets. To minimize network usage in a distributed system, each machine can record only its own data, so the measurement can be merged later (by just copying all files in one directory). Distributed recordings can be started from the eCAL Rec GUI application (a CLI version will come with a future release). When clicking the REC button, it will connect to the eCAL Rec instances running on all selected machines and start the recording.

This image shows the eCAL Rec GUI recording 3 topic on the same host.

![eCAL recorder recording 3 topics](gfx/app/recorder.png?raw=true "eCAL recorder recording 3 topics")

### Player

The eCAL Player can replay the recordings from the eCAL Recorder. It comes as GUI, as CLI and as C++ library for custom applications. The player can speed up or slow down the recording and step through a recording on a frame or channel base.

The following image shows a running replay, publishing 2 messages in realtime.

![eCAL player](gfx/app/player.png?raw=true "eCAL player")


## Performance

The following table shows the latency in µs between a single publisher / subscriber connection for different payload sizes (two different processes running on the same host). You can simply measure the latency on your own machine by running the ecal_latency_snd and ecal_latency_rec_cb sample applications. The first two columns are showing the performance for the eCAL builtin shared memory layer and the last column for the iceoryx shared memory layer (configured by cmake option ECAL_LAYER_ICEORYX).

First start ecal_sample_latency_rec_cb. This application will receive the published payloads, send them back to the sender and print out the average receive time, the message frequency and the data throughput over all received messages. The sending application ecal_latency_snd can be configured that way ..

     ecal_latency_snd  -s <payload_size [kB]> -r <send loops>

The table shows the results for a Windows and a Linux platform (200000 samples 1kB - 512kB / 10000 samples > 512 kB, zero drops).

```
-------------------------------
 Platform
-------------------------------
OS Name:                            Microsoft Windows 10 Pro
OS Version:                         10.0.18363
OS Manufacturer:                    Microsoft Corporation
OS Build Type:                      Multiprocessor Free
System Manufacturer:                HP
System Model:                       HP ZBook 15 G5
System Type:                        x64-based PC
Processor(s):                       1 Prozessor(s) Installed.
                                    [01]: Intel64 Family 6 Model 158 Stepping 10 GenuineIntel ~2592 MHz
Total Physical Memory:              32.614 MB

```

|Payload Size (kB)|Win10 AMD64 (µs)|Ubuntu18 AMD64 (µs)|Ubuntu18 AMD64 (µs)|
|----------------:|---------------:|------------------:|------------------:|
|                 |      eCAL SHM  |         eCAL SHM  |      Iceoryx SHM  |
|              1  |            10  |                4  |                6  |
|              2  |            10  |                4  |                6  |
|              4  |            10  |                5  |                6  |
|              8  |            11  |                5  |                6  |
|             16  |            12  |                6  |                6  |
|             32  |            13  |                7  |                8  |
|             64  |            16  |               10  |               10  |
|            128  |            21  |               15  |               13  |
|            256  |            32  |               33  |               19  |
|            512  |            56  |               50  |               28  |
|           1024  |           103  |              154  |               82  |
|           2048  |           363  |              392  |              177  |
|           4096  |           867  |              877  |              420  |
|           8192  |          1814  |             1119  |              534  |
|          16384  |          3956  |             2252  |             1060  |

## Usage

### The Publish-Subscribe Pattern

Let's implement a minimal publisher/subscriber example exchanging the famous "hello world" message :-). First the publisher ..

##### Listing 1

```cpp
#include <ecal/ecal.h>

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "minimal c++ publisher");

  // create publisher
  eCAL::CPublisher pub("foo", "std::string");

  // send the content
  pub.Send("HELLO WORLD FROM C++");

  // finalize eCAL API
  eCAL::Finalize();
}
```

The eCAL::Initialize function is initializing / creating all eCAL background mechanisms like the registration threads or the global memory file pool handling. That function has at least 3 arguments. The first two (argc, argv) are the forwarded application command line arguments. They are used to overwrite specific eCAL configuration settings for a single eCAL process (see command line options). The third argument is the so called unit name. The unit name is used for the monitoring API to identify your process by a human readable identifier. It can be a unique name for every single process or you can group different processes with the same name.

In line 6 an eCAL::CPublisher is created to register the topic "foo" as data source. In this example we use a simple CPublisher that can be used to send binary buffers or std::string data objects. There are more specialized publishers to send google protocol buffers, google flatbuffers, msgpack messages and much more. The only (technically) important parameter is the topic name (in our example "foo"). This name is used to match the connection with an eCAL::CSubscriber anywhere outside in the eCAL cloud. That means this name has to be unique and it is interpreted case sensitive.

After creating the publisher everything is prepared to send the content. This is done in line 12. Here we use the overloaded Send method for a std::string. There is another variant to send a binary buffer with address pointer and length. The second argument
"std::string" is the so called topic type name. It will be checked by eCAL between the connected publishers and subscribers and a warning is created if they do not match to inform the user that there could be a possible system configuration problem.

To cleanup everything (stop all background threading, release memory) you should call eCAL::Finalize before your application exits to shut down eCAL API.

Now let's realize the matching subscriber to listing 1.

##### Listing 2

```cpp
#include <ecal/ecal.h>

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "minimal c++ subscriber");

  // create subscriber
  eCAL::CSubscriber sub("foo", "std::string");

  // receive content (infinite timeout)
  std::string msg;
  sub.Receive(msg, nullptr, -1);

  // finalize eCAL API
  eCAL::Finalize();
}
```

Lines 1 to 6 are equivalent to listing 1 and have the same functionality. Only the unit name ("minimal c++ subscriber") is different. In Line 9
we create a simple eCAL::CSubscriber that is able to receive binary buffers or std::strings. It's very important to use the same
topic name ("foo") like in listing 1 to establish the pub / sub connection. In the lines 12/13 we use the CSubcriber::Receive method
to wait (blocking) for incoming messages and write them to the string "msg". The full signature of eCAL::CSubscriber::Receive looks like
this

```cpp
size_t eCAL::CSubscriber::Receive( std::string & buf_,
                                   long long *   time_ = nullptr,
                                   int           rcv_timeout_ = 0
                                 ) const;
```

The function returns the number of received bytes and can write the publisher time stamp to "time_". The receive timeout can used to define how long the call should wait for incoming data in milliseconds. A time 0 means the Receive call checks if there is a new received buffer and returns immediately. An argument -1 would block the call until a new message arrives.

The rest of listing 2 is equivalent to listing 1 and is shutting down the eCAL API.

Listing 2 showed how to receive data with a polling strategy. This is useful if the users application has it's own threading model and the receive action is placed anywhere in an existing background thread. Another receive strategy is to use a so called eCAL::CSubscriber message callback. Because eCAL itself is using receive threads in the background you can register your own receive function and eCAL will call it when a matching message for your subscriber arrives. Listing 3 shows how to use it.

##### Listing 3

```cpp
#include <ecal/ecal.h>
#include <ecal/msg/subscriber.h>

#include <chrono>
#include <thread>

// define a subscriber callback function
void OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
{
  printf("We received %s on topic %s\n.", (char*)data_->buf, topic_name_);
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "minimal c++ subscriber (callback)");

  // create subscriber
  eCAL::CSubscriber sub("foo", "std::string");

  // register subscriber callback function
  sub.AddReceiveCallback(OnReceive);

  // idle main thread
  while (eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
```

There are different variants for registering a message callback. Here we use a low level ("old school") function pointer to intercept incoming raw messages. A more elegant and powerful method is to use std::bind to connect the eCAL receive logic
to any user function including class member functions. In general it would look like this

##### Listing 4

```cpp
// define a subscriber callback function
void OnReceive(const char* topic_name_, const std::string& message_)
{
  printf("We received %s on topic %s\n.", message_.c_str(), topic_name_);
}

// create subscriber
eCAL::string::CSubscriber<std::string> sub("foo");

// register subscriber callback function
auto callback = std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2);
sub.AddReceiveCallback(callback);
```

The main different to listing 3 is that we use a specialized eCAL::string::Subscriber for std::strings and redirect the received messages on the function OnReceive using std::bind. The receive function signature is defined by the std::placeholders. In our example we are only interested in the topic name and the message string. A third placeholder is available to forward the timestamp to the callback function too.

#### Google::Protobuf

The eCAL middleware does not provide it's own serialization format but supports different standards. There are specialized (templated) publisher and subscriber classes to simplify the usage of existing message protocols (see google flatbuffers, cap'n proto or google protobuf for more details).

To support a specific message serialization format you need to include the appropriate serialization message header. For google protobuf you need the following eCAL header.

##### Listing 5
```cpp
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>
```

Listing 6 shows a typical example using google protobuf. First you have to define a so called .proto file for the message you want to serialize and deserialize. A simple Shape struct will be defined as follows ..

##### Listing 6 (shape.proto)

```cpp
syntax = "proto3";

message Shape
{
  enum ShapeType
  {
    CIRCLE    = 0;
    TRIANGLE  = 1;
    RECTANGLE = 2;
  }

  ShapeType type = 1 [default = CIRCLE];
  int32     size = 2;
}
```

After compiling that .proto file using the protobuf compiler you will receive a shape.pb.cc and shape.pb.h file that you have to include into your project. The serialization and deserialization is quite simple ..

##### Listing 7

```cpp
// create protobuf template publisher for our Shape message (topic name "shape")
eCAL::protobuf::CPublisher<Shape> pub("shape");

// generate a class instance of Shape
Shape shape;

// modify type and size of the shape object
shape.set_type(Shape_ShapeType_CIRCLE);
shape.set_size(42);

// and send the shape object
pub.Send(shape);
```

The Send call of our publisher "pub" is sending the message in two steps. At first the message object is serialized in a binary stream and this binary stream is then forwarded to the underlying "raw" eCAL::CPublisher to finally send the binary message blob. So the user doesn't have to care how to serialize the message and how to define the right message type. This is done as a "service" of the eCAL::protobuf::CPublisher implementation.

There is another interesting feature provided by the eCAL::protobuf::CPublisher. It is automatically forwarding the so called protobuf message descriptor to the matching eCAL::protobuf::CSubscriber. This descriptor can be used to dynamic deserialize messages using googles protobuf reflection API (for details see protobuf documentation). The eCAL monitor is using this information to deserialize protobuf messages on the fly (message reflection).

Finally let's show how to receive our Shape message with an eCAL::protobuf::CSubscriber. First the polling receive variant ..

##### Listing 8

```cpp
// create protobuf template subscriber for our Shape message (topic name "shape")
eCAL::protobuf::CSubscriber<Shape> sub("shape");

// generate a class instance of Shape
Shape shape;

// and receive the shape object
sub.Receive(shape);
```

The logic is the same like in listing 2, except that we do not receive a std::strimg, but a google message class object.

The callback variant is also very similar to the logic of listing 4. Here it is ..

##### Listing 9

```cpp
// define a subscriber callback function
void OnReceive(const char* topic_name_, const Shape& message_)
{
  // print whole message content
  std::cout << message_.DebugString();
}

// create subscriber
eCAL::protobuf::CSubscriber<Shape> sub("shape");

// register subscriber callback function
auto callback = std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2);
sub.AddReceiveCallback(callback);
```

#### Setup the transport layer

In general eCAL provides a very simple API to establish interprocess / interhost communication using the publish / subscribe pattern. How the data is transported from process A to process B is completely transparent and eCAL is using different
strategies to realize a fast message delivery with low latency. For connections on the same host memory files are used to exchange data. Between different hosts the UDP multicast protocol is used. For special purposes it can make sense to switch of this "automatic" and to define which transport layer should be used or should never be used. This can be done by defining the eCAL transport layer using the eCAL::CPublisher method "SetLayerMode":

```cpp
bool eCAL::CPublisher::SetLayerMode( eTransportLayer layer_,
                                     eSendMode       mode_
                                   )
```

The currently supported layers are

```cpp
enum eTransportLayer
{
  tlayer_udp_mc     = 1,  // udp multicast (eCAL - using multiple multicast groups for high efficient data transport)
  tlayer_shm        = 4,  // shared memory (eCAL or Iceoryx)
  tlayer_inproc     = 42, // inner process (eCAL - deterministic inner process communication)
};
```

You can switch on / off every single transport layer independently. This is done by the mode_ argument.

```cpp
enum eSendMode
{
  smode_off = 0,
  smode_on,
  smode_auto
};
```

The "smode_auto" is activated by default for shared memory and udp multicast (eCAL spec.). If you want to ensure that messages will never leave your machine then you can switch off the udp multicast layer by

```cpp
pub.SetLayerMode(tlayer_udp_mc, smode_off);
```

The default "auto" settings for shared memory and udp multicast are recommended. In this mode a local or interhost connection will be created only if there is a matching publisher / subscriber pair detected by the eCAL registration layer. That means no CPU performance is wasted by an open, unconnected publisher when the Send method is called.

### The Client-Server Pattern

Besides the discussed publish-subscribe pattern eCAL supports the classic client-server network pattern. In some scenarios this pattern matches better than the loosely coupled publish-subscribe. A service can realize a parameter database for exacmple. For sure you can mix up publisher, subscriber, server and clients in the same application.

To define a service you have to describe it first in a google protobuf idl file. Let's define a simple "PingService". The service will wait on a "PingRequest" and response with a "PingResponse". A proto schema file for the "PingService" could look like this

##### Listing 10 (ping.proto)
```protobuf
syntax = "proto3";

option cc_generic_services = true;

message PingRequest
{
  string message = 1;
}

message PingResponse
{
  string answer = 1;
}

service PingService
{
  rpc Ping (PingRequest) returns (PingResponse);
}
```

After compiling the proto file using the protoc compiler you can implement the server side that way.

##### Listing 11

```cpp
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/server.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "ping.pb.h" // the google compiler autogenerated service class

// PingService implementation
class PingServiceImpl : public PingService
{
public:
  void Ping(::google::protobuf::RpcController* /* controller_ */, const ::PingRequest* request_, ::PingResponse* response_, ::google::protobuf::Closure* /* done_ */)
  {
    // print request
    std::cout << "Received request PingService / Ping : " << request_->message() << std::endl;
    // and send response
    response_->set_answer("PONG");
  }
};

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ping service server");

  // create PingService server
  std::shared_ptr<PingServiceImpl> ping_service_impl = std::make_shared<PingServiceImpl>();
  eCAL::protobuf::CServiceServer<PingService> ping_server(ping_service_impl);

  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
```

In the lines 11 to 21 a class PingServiceImpl is derived from the autogenerated PingService class from the idl file in listing 10. The PingServiceImpl class has to implement the Ping method that is pure virtual in the PingService base class. The signature of the service methods is standardized with the four parameter to see in line 14, the return value is void. The most interesting parameters are request_ and response_. So every service method is accessing the service parameter passed by request_ and will response by setting the response_ message elements.

To start the service is very simple, just instantiate a PingServiceImpl object and pass it to an eCAL::protobuf::CServiceServer (line 30). The service will start immediately and can be accessed by existing clients.

A matching client can be implemented like this

##### Listing 12

```cpp
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/client.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "ping.pb.h"

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ping client");

  // create ping service client
  eCAL::protobuf::CServiceClient<PingService> ping_service;

  while(eCAL::Ok())
  {
    //////////////////////////////////////
    // Ping service (blocking call)
    //////////////////////////////////////
    eCAL::SServiceInfo service_info;
    PingRequest        ping_request;
    PingResponse       ping_response;
    ping_request.set_message("PING");
    if (ping_service.Call("", "Ping", ping_request, service_info, ping_response))
    {
      std::cout << std::endl << "PingService::Ping method called with message : " << ping_request.message() << std::endl;
      switch (service_info.call_state)
      {
      // service successful executed
      case call_state_executed:
      {
        std::cout << "Received response PingService / Ping : " << ping_response.answer() << " from host " << service_info.host_name << std::endl;
      }
      break;
      // service execution failed
      case call_state_failed:
        std::cout << "Received error PingService / Ping : " << service_info.error_msg << " from host " << service_info.host_name << std::endl;
        break;
      default:
        break;
      }
    }
    else
    {
      std::cout << "PingService::Ping method call failed .." << std::endl << std::endl;
    }

    // sleep a second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
```

There are different variants how to catch the server response, as callback or as a blocking service call. Listing 12 shows the blocking call variant. The interesting line is line 17. Here we define a new service client for the "PingService" defined by the protobuf idl.

To call the service "Ping" we need to instantiate a PingRequest message (line 25), set the elements of the request message (line 27) and finally call the service "Ping" (line 28) with the request message object. The argument service_info will contain detailed informations about the server response.

In line 31 the service_info is checked and in the case of a successfully executed service the response is and printed out (line 36).

The equivalent service client callback variant looks like this

##### Listing 13

```cpp
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/client.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "ping.pb.h"

// callback for ping service response
void OnPingResponse(const struct eCAL::SServiceInfo& service_info_, const std::string& response_)
{
  switch (service_info_.call_state)
  {
  // service successful executed
  case call_state_executed:
  {
    PingResponse ping_response;
    ping_response.ParseFromString(response_);
    std::cout << "Received response PingService / Ping : " << ping_response.answer() << " from host " << service_info_.host_name << std::endl;
  }
  break;
  // service execution failed
  case call_state_failed:
    std::cout << "Received error PingService / Ping : " << service_info_.error_msg << " from host " << service_info_.host_name << std::endl;
    break;
  default:
    break;
  }
}

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "service client");

  // create PingService client and add callback for service response
  eCAL::protobuf::CServiceClient<PingService> ping_client;
  ping_client.AddResponseCallback(OnPingResponse);

  while(eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // finalize eCAL API
  eCAL::Finalize();
}
```

## Other languages

### C

TODO: Describe this ..

##### Listing 14

```c
#include <ecal/ecalc.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  ECAL_HANDLE pub     = 0;
  char        snd_s[] = "HELLO WORLD FROM C";
  int         sent    = 0;

  // initialize eCAL API
  eCAL_Initialize(argc, argv, "minimal c publisher", eCAL_Init_Default);

  // create publisher
  pub = eCAL_Pub_New();
  eCAL_Pub_Create(pub, "foo", "base:std::string", "", 0);

  // send updates
  while(eCAL_Ok())
  {
    // send content
    sent = eCAL_Pub_Send(pub, snd_s, (int)strlen(snd_s), -1);
    if(sent <= 0) printf("Sending topic \"foo\" failed !\n");
    else          printf("Published topic \"foo\" with \"%s\"\n", snd_s);

    // sleep 100 ms
    eCAL_Process_SleepMS(100);
  }

  // destroy publisher
  eCAL_Pub_Destroy(pub);

  // finalize eCAL API
  eCAL_Finalize(eCAL_Init_All);

  return(0);
}
```

##### Listing 15

```c
#include <ecal/ecalc.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  ECAL_HANDLE sub = 0;
  int         rcv = 0;
  void*       rcv_buf;
  long long   time;

  // initialize eCAL API
  eCAL_Initialize(argc, argv, "minimal c subscriber", eCAL_Init_Default);

  // create subscriber
  sub = eCAL_Sub_New();
  eCAL_Sub_Create(sub, "foo", "base:std::string", "", 0);

  // read updates
  while(eCAL_Ok())
  {
    // receive content with 100 ms timeout
    rcv = eCAL_Sub_Receive_Alloc(sub, &rcv_buf, &time, 100);
    if(rcv > 0)
    {
      // print content
      printf("Received topic \"foo\" with \"%.*s\"\n", rcv, (char*)rcv_buf);

      // free buffer allocated by eCAL
      eCAL_FreeMem(rcv_buf);
    }
  }

  // destroy subscriber
  eCAL_Sub_Destroy(sub);

  // finalize eCAL API
  eCAL_Finalize(eCAL_Init_All);

  return(0);
}
```

##### Listing 16

```c
#include <ecal/ecalc.h>
#include <stdio.h>

void OnReceive(const char* topic_name_, const struct SReceiveCallbackDataC* data_, void* par_)
{
  par_ = par_;
  printf("Received topic \"%s\" with ", topic_name_);
  printf("\"%.*s\"\n", (int)(data_->size), (char*)(data_->buf));
}

int main(int argc, char **argv)
{
  ECAL_HANDLE sub = 0;

  // initialize eCAL API
  eCAL_Initialize(argc, argv, "minimal c subscriber (callback)", eCAL_Init_Default);

  // create subscriber
  sub = eCAL_Sub_New();
  eCAL_Sub_Create(sub, "foo", "base:std::string", "", 0);

  // add callback
  eCAL_Sub_AddReceiveCallbackC(sub, OnReceive, NULL);

  // idle main thread
  while(eCAL_Ok())
  {
    // sleep 100 ms
    eCAL_Process_SleepMS(100);
  }

  // destroy subscriber
  eCAL_Sub_Destroy(sub);

  // finalize eCAL API
  eCAL_Finalize(eCAL_Init_All);

  return(0);
}
```

### Python

#### The Publish-Subscribe Pattern

The eCAL API is fully wrapped into python script language. The usage is quite simple ('help ecal' in your python shell ;-)). You can find samples for all common use cases in the eCAL installation. Let's demonstrate the famous publish-subscribe "hello world".

##### Listing 17

```python
import sys
import time

# import ecal core and string publishing
import ecal.core.core as ecal_core
from ecal.core.publisher import StringPublisher

# initialize eCAL API
ecal_core.initialize(sys.argv, "minimal python publisher")

# create publisher
pub = StringPublisher("foo")
msg = "HELLO WORLD FROM PYTHON"

# sending hello's
while ecal_core.ok():
  pub.send(current_message)
  time.sleep(0.01)

# finalize eCAL API
ecal_core.finalize()
```

eCAL is wrapped into python in different packages. ecal.core contains all basic functionalities, ecal.core.publisher and ecal.core.subscriber the publish/subscribe interface. In listing 17 we use a simple string publisher. After initializing the core in line 9 we instanciate a publisher with the topic name "foo" and start pubishing a hello message every 10 ms. This sample is fully communication compatible to the c++ hello world subscribers in the Listings 1-4.

##### Listing 18

```python
import sys

# import ecal core and string subscribing
import ecal.core.core as ecal_core
from ecal.core.subscriber import StringSubscriber

# initialize eCAL API
ecal_core.initialize(sys.argv, "minimal python subscriber")

# create subscriber
sub = StringSubscriber("foo")

# receive messages
while ecal_core.ok():
  ret, msg, time = sub.receive(500)
  if ret > 0: print("Received:  {} ms   {}".format(time, msg))
  else:       print("Subscriber timeout ..")

# finalize eCAL API
ecal_core.finalize()

```

Listing 18 shows the python variant of the hello world subscriber. It uses a 500 ms blocking receive call to get the payload in the python msg variable and print it out. Finally Listing 19 shows the python hello world subscriber realized with a receive callback function.

##### Listing 19

```python
import sys
import time

# import ecal core and string subscribing
import ecal.core.core as ecal_core
from ecal.core.subscriber import StringSubscriber

# eCAL receive callback
def onreceive(topic_name, msg, time):
  print("Received:  {} ms   {}".format(time, msg))

# initialize eCAL API
ecal_core.initialize(sys.argv, "minimal python subscriber (callback)")

# create subscriber and connect callback
sub = StringSubscriber("foo")
sub.set_callback(onreceive)

# idle main thread
while ecal_core.ok():
  time.sleep(0.1)

# finalize eCAL API
ecal_core.finalize()
```

### CSharp

TODO: Describe this ..

#### The Publish-Subscribe Pattern

##### Listing 20

```csharp
using System;
using Continental.eCAL.Core;

public class minimal_snd
{
  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("minimal_snd");

    // create a publisher (topic name "Hello", type "base:std::string", description "")
    Publisher publisher = new Publisher("Hello", "base:std::string", "");

    // idle main thread
    int loop = 0;
    while (Util.Ok())
    {
      // message to send
      string message = String.Format("HELLO WORLD FROM C# {0,6}", ++loop);

      // send the content
      publisher.Send(message, -1);

      // cool down
      System.Threading.Thread.Sleep(100);
    }

    // dispose publisher
    publisher.Dispose();

    // finalize eCAL API
    Util.Terminate();
  }
}
```

##### Listing 21

```csharp
using System;
using Continental.eCAL.Core;

public class minimal_rcv
{
  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("minimal_rcv");

    // create a subscriber (topic name "Hello", type "base:std::string")
    Subscriber subscriber = new Subscriber("Hello", "base:std::string", "");

    // idle main thread
    while (Util.Ok())
    {
      // receive content with 100 ms timeout
      Subscriber.ReceiveCallbackData message = subscriber.Receive(100);

      // print message
      if (message != null) System.Console.WriteLine(String.Format("Received:  {0}", message.data));
    }

    // dispose subscriber
    subscriber.Dispose();

    // finalize eCAL API
    Util.Terminate();
  }
}
```

##### Listing 22

```csharp
using System;
using Continental.eCAL.Core;

public class minimal_rcv_cb
{
  static void MyCallback(String topic, Subscriber.ReceiveCallbackData data)
  {
    System.Console.WriteLine("Topic name " + topic);
    System.Console.WriteLine("Topic content " + data.data);
  }

  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("minimal_rcv_cb");

    // create a subscriber (topic name "Hello", type "base:std::string")
    Subscriber subscriber = new Subscriber("Hello", "base:std::string", "");
    Subscriber.ReceiverCallback callback = MyCallback;
    subscriber.AddReceiveCallback(callback);

    // idle main thread
    while (Util.Ok())
    {
      System.Threading.Thread.Sleep(100);
    }

    // dispose subscriber
    subscriber.Dispose();

    // finalize eCAL API
    Util.Terminate();
  }
}
```

## Command Line Interface

If you pass the argc, argv command line arguments to the eCAL::Initialize API function (see Listing 23) you can set / overwrite all eCAL runtime parameter defined in the eCAL parameter ini file (ecal.ini). Moreover you can force your application to not load the default eCAL ini file but to specify your own parameter file for the system wide default settings.

##### Listing 23

```cpp
#include <ecal/ecal.h>

int main(int argc, char **argv)
{
  // Pass argc, argv to eCAL API
  eCAL::Initialize(argc, argv, "unit name");
  ...
}
```

If you use your own command line handling and dont want to forward anything to eCAL just use

```cpp
eCAL::Initialize(0, nullptr, "unit name");
```

Sometimes you want to handle your own arguments but still forward eCAL specific command line arguments to the eCAL API. In this case handle the command line parsing yourself and forward all eCAL specific parameters by a vector of strings (std::vector<std::string>).

```cpp
std::vector<std::string> args;

args.push_back("--ecal-ini-file");
args.push_back("/home/share/ecal.ini");

args.push_back("--ecal-set-config-key");
args.push_back("network/network_enabled:true");

eCAL::Initialize(args, "unit name");
```

To check all available command line options and their syntax just call

```bat
foo.exe --help
```

The output will look like this

  USAGE:

     ecal_sample_person_snd.exe  [--ecal-set-config-key <string>] ...
                              [--ecal-ini-file <string>] [--ecal-dump-config]
                              [--] [--version] [-h]


  Where:

     --ecal-set-config-key <string>  (accepted multiple times)
       Overwrite a specific configuration key (ecal-set-config-key
       "section/key:value".

     --ecal-ini-file <string>
       Load default configuration from that file.

     --ecal-dump-config
       Dump current configuration.

     --,  --ignore_rest
       Ignores the rest of the labeled arguments following this flag.

     --version
       Displays version information and exits.

     -h,  --help
       Displays usage information and exits.


### The --ecal-ini-file option

Load default configuration from that file.

Usage:

```bat
foo.exe --ecal-ini-file "/home/share/my_ecal.ini"
```

Will load `/home/share/my_ecal.ini` file instead the eCAL default ini file (located in the `%APPDATA%` folder under windows or in `~/.ecal` under Linux)

### The --ecal-set-config-key option

Overwrite a specific configuration key. This option can be used multiple times.

Usage:

```bat
foo.exe --ecal-set-config-key "network/network_enabled:true" --ecal-set-config-key "network/multicast_ttl:3"
```

Will overwrite the 2 network settings network_enabled and multicast_ttl (defined in ecal.ini file) with the values "true" and "3".

### The --ecal-dump-config option

Usage:

```bat
foo.exe --ecal-dump-config
```

Will dump the whole currently used configuration. This option is very useful in combination with the options mentioned above. For example

```bat
foo.exe --ecal-dump-config --ecal-ini-file "/home/share/my_ecal.ini"
```

Will load the specified default eCAL ini file and print out the loaded configuration.

## Licensing

*eCAL* is licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE.txt) for the full license text.

> Written with [StackEdit](https://stackedit.io/).

