
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

eCAL is simplifying the data transport as much as possible, It uses different mechanism to transport a topic from a publisher to a connected subscriber. On the same computer node the data are exchanged by using memory mapped files. Between different computing nodes UDP multicast can be used for high performance data throughput, rtps can be used for reliable data transport.


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

## Setup on Linux Systems

Update gcc to 5.3.x or newer and install cmake.

### Installation

#### Install toolchain

```bash
sudo apt-get install cmake
sudo apt-get install doxygen
sudo apt-get install graphviz
```

*Note:* building cmake including documentation requires using a fairly new version of CMake, so the version installed with your system (e.g. 3.5.1 for Ubuntu 16.04) might not be sufficient. You can install newer versions of CMake by adding a ppa as described [here]( https://blog.kitware.com/ubuntu-cmake-repository-now-available/) or downloading it from the [CMake website](https://cmake.org/download/)

#### Install third party dependencies:
  
```bash
sudo apt-get install build-essential
sudo apt-get install zlib1g-dev
sudo apt-get install qt5-default
```

#### Build and install eCAL

Build eCAL, let CMake create a debian package and install that one

```bash
mkdir _build
cd _build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
cpack -G DEB
sudo dpkg -i eCAL*
```
### UDP network configuration

setup the correct ip address - here for adapter eth0, ip address 192.168.0.1

```bash
sudo ifconfig eth0 192.168.0.1 netmask 255.255.255.0
```

Restart the ethernet interface or the whole machine to apply changes, check if you can ping to each other.

After the ipc configuration setup the multicast route for udp multicasting

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

### Preparation

You need

  • CMake (https://cmake.org)

  • Doxygen (http://www.doxygen.nl)

  • Qt5 (https://www1.qt.io/download-open-source/#section-2)
 
  • WIX (http://wixtoolset.org/)

  to create the Visual Studio eCAL solution, to build it and to create a windows setup file.

#### Qt 5

Install Qt5 by starting the installer and selecting `msvc2015 32-bit` and `msvc2015 64-bit` (VS2015) or `msvc2017 32-bit` and `msvc2017 64-bit` (VS2017) from the latest Qt5 version. Create an environment variable `QT5_ROOT_DIRECTORY` that points to the directory containing the architecture-specific folders. It should look like this:

    %QT5_ROOT_DIRECTORY%
      ├ msvc2015
      ├ msvc2015_64
      ├ msvc2017
      └ msvc2017_64
      

#### Build eCAL

Run the following batch files to create the Visual Studio 2015 (2017) solutions for 32 and 64 bit and to build both.

```bat
build_win\win_make_cmake.bat v140 (v141)
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

to cmake eCAL build all configurations and create an installer. You will find the installer in the _setup subfolder finally.

### UDP network configuration

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

## CMake build options

When configuring with CMake, you can turn on / off specific features of eCAL.

- `HAS_QT5`, default: `ON`  
  Platform supports Qt 5 library, necessary to build monitoring tool
- `BUILD_DOCS`, default `ON`
  Build the eCAL documentation, requires the installation of doxygen and a recent CMake version (>= 3.14 preferred, but some lower versions might work)
- `BUILD_APPS`, default `ON`,  
  Build the eCAL applications, such as the Monitor
- `BUILD_SAMPLES`, default `OFF`
  Build the eCAL sample applications
- `BUILD_TIME`, default `ON`
  Build the eCAL time interfaces, necessary if you want to use ecal in time synchronized mode
- `ECAL_LAYER_FASTRTPS`, default `OFF`
  Provide fast rtps as communication layer, requires fast-rtps and fast-cdr installations
- `ECAL_INSTALL_SAMPLE_SOURCES`, default: `ON`
  Install the sources of eCAL samples
- `ECAL_JOIN_MULTICAST_TWICE`, default: `OFF`
  Specific Multicast Network Bug Workaround
- `ECAL_NPCAP_SUPPORT`, default `OFF`
  Enable the eCAL Npcap Receiver (i.e. the Win10 performance fix)
- `ECAL_THIRDPARTY_BUILD_PROTOBUF`, default `ON`
  Build Protobuf with eCAL, included as a submodule in the thirdparty folder. You can always use your custom protobuf installation, this is only for convenience. Note, at least protobuf 3.0 is required to compile eCAL, we recommend using 3.5.1 or newer (tested with 3.5.1).

All options can be passed on the command line `cmake -D<option>=<value>` or in the CMake GUI application.

## Initial Test

To initially check the functionality of a fresh installed eCAL system, just follow these steps.

* start the two installed sample applications "ecal_sample_person_snd" and "ecal_sample_person_rec"
* start the eCAL monitor application (for windows via Start Menu / eCAL / Start eCAL Monitor", for linux via command line "ecal_mon")

Now you should see the two running application "person publisher" and "person subscriber". Expand their content and double click on the topic "person". A new reflection windows should open and you can check the content of the person topic on the fly.

![eCAL monitor showing topic 'person'](gfx/app/monitor_person.png?raw=true "eCAL monitor showing topic 'person'")

## Performance

The following table shows the latency in µs between a single publisher / subscriber connection for different payload sizes (two different processes running on the same host). You can simply measure the latency on your own machine by running the ecal_latency_snd and ecal_latency_rec sample applications.

First start ecal_sample_latency_rec. This application will receive the published payloads, send them back to the sender and print out the average receive time, the message frequency and the data throughput over all received messages. The sending application ecal_latency_snd can be configured that way ..
  
     ecal_latency_snd  -s <payload_size [kB]> -r <send loops>
  
The table shows the results for a Windows and a Linux platform (20000 samples, zero drops).

```
-------------------------------
 Platform Windows 10 (AMD64)
-------------------------------
OS Name:                            Microsoft Windows 10 Enterprise
OS Version:                         10.0.16299
OS Manufacturer:                    Microsoft Corporation
OS Build Type:                      Multiprocessor Free
System Manufacturer:                HP
System Model:                       HP ZBook 15 G5
System Type:                        x64-based PC
Processor(s):                       1 Prozessor(s) Installed.
                                    [01]: Intel64 Family 6 Model 158 Stepping 10 GenuineIntel ~2592 MHz
Total Physical Memory:              32.579 MB

-------------------------------
 Platform Ubuntu 16 (AMD64)
-------------------------------
H/W path      Device    Class       Description
===============================================
                        system      HP ZBook 15 G3 (M9R63AV)
/0                      bus         80D5
/0/0                    memory      128KiB L1 Cache
/0/1                    memory      128KiB L1 Cache
/0/2                    memory      1MiB L2 Cache
/0/3                    memory      8MiB L3 Cache
/0/4                    processor   Intel(R) Core(TM) i7-6820HQ CPU @ 2.70GHz
/0/5                    memory      16GiB System Memory
/0/5/0                  memory      8GiB SODIMM Synchron 2133 MHz (0,5 ns)
/0/5/1                  memory      8GiB SODIMM Synchron 2133 MHz (0,5 ns)

```

|Payload Size (kB)|Win10 AMD64|Ubuntu16 AMD64|
|----------------:|----------:|-------------:|
|              1  |       25  |          14  |
|              2  |       25  |          14  |
|              4  |       26  |          15  |
|              8  |       28  |          16  |
|             16  |       33  |          18  |
|             32  |       37  |          22  |
|             64  |       47  |          26  |
|            128  |       68  |          40  |
|            256  |      107  |          66  |
|            512  |      190  |         134  |
|           1024  |      401  |         720  |
|           2048  |      937  |        1500  |
|           4096  |     1868  |        3600  |

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
  tlayer_udp_uc     = 2,  // udp unicast   (eCAL)
  tlayer_shm        = 4,  // shared memory (eCAL)
  tlayer_lcm        = 5,  // lcm           (Google)
  tlayer_rtps       = 6,  // rtps          (eProsima - a standard DDS transport layer)
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
    rcv = eCAL_Sub_Receive(sub, &rcv_buf, ECAL_ALLOCATE_4ME, &time, 100);
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

args.push_back("--default-ini-file");
args.push_back("/home/share/ecal.ini");

args.push_back("--set_config_key");
args.push_back("network/network_enabled:true");

eCAL::Initialize(args, "unit name");
```

To check all available command line options and their syntax just call

```bat
foo.exe --help
```

The output will look like this

  USAGE: 
  
     foo.exe  [-s <string>] ...  [-n <string>] [-d <string>] [-c] [--]
                     [--version] [-h]
  
  
  Where: 
  
     -s <string>,  --set_config_key <string>  (accepted multiple times)
       Overwrite a specific configuration key (set_config_key
       "section/key:value".
  
     -d <string>,  --default-ini-file <string>
       Load default configuration from that file.
  
     -c,  --dump-config
       Dump current configuration.
  
     --,  --ignore_rest
       Ignores the rest of the labeled arguments following this flag.
  
     --version
       Displays version information and exits.
  
     -h,  --help
       Displays usage information and exits.
  

### The --default-ini-file (-d) option

Load default configuration from that file.

Usage:

```bat
foo.exe --default-ini-file "/home/share/my_ecal.ini"
```

Will load `/home/share/my_ecal.ini` file instead the eCAL default ini file (located in the `%APPDATA%` folder under windows or in `~/.ecal` under Linux)

### The --set_config_key (-s) option

Overwrite a specific configuration key. This option can be used multiple times.

Usage:

```bat
foo.exe --set_config_key "network/network_enabled:true" --set_config_key "network/multicast_ttl:3"
```

Will overwrite the 2 network settings network_enabled and multicast_ttl (defined in ecal.ini file) with the values "true" and "3".

### The --dump-config (-d) option

Usage:

```bat
foo.exe --dump-config
```

Will dump the whole currently used configuration. This option is very useful in combination with the options mentioned above. For example

```bat
foo.exe --dump-config --default-ini-file "/home/share/my_ecal.ini"
```

Will load the specified default eCAL ini file and print out the loaded configuration.

## Licensing

*eCAL* is licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE.txt) for the full license text.

> Written with [StackEdit](https://stackedit.io/).
