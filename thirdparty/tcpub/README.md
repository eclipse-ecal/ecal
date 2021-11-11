
# tcpub - TCP Publish/Subscribe library

tcpub is a minimal publish-subscribe library that transports data via TCP. The project is CMake based. The dependencies are integrated as git submodules. In your own Project you can either use those submodules, or provide the dependencies in your own manner.

tcpub does not define a message format but only transports binary blobs. It does however define a protocol around that, which is kept as lightweight as possible.

Dependencies:

- [asio](https://github.com/chriskohlhoff/asio.git)
- [recycle](https://github.com/steinwurf/recycle.git)

## Hello World Example

A very similar Example is also provided in the repository.

### Publisher

```cpp
#include <thread>

#include <tcpub/executor.h>
#include <tcpub/publisher.h>

int main()
{
  // Create a "Hello World" buffer
  std::string data_to_send = "Hello World";
  
  // Create an Executor with a thread-pool size of 6. If you create multiple
  // publishers and subscribers, they all should share the same Executor.
  std::shared_ptr<tcpub::Executor> executor = std::make_shared<tcpub::Executor>(6);
  
  // Create a publisher that will offer the data on port 1588
  tcpub::Publisher hello_world_publisher(executor, 1588);

  for (;;)
  {
    // Send the "Hello World" string by passing the pointer to the first
    // character and the length.
    hello_world_publisher.send(&data_to_send[0], data_to_send.size());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return 0;
}
```

### Subscriber

```cpp
#include <iostream>
#include <thread>

#include <tcpub/executor.h>
#include <tcpub/subscriber.h>

int main()
{
  // Create an Executor with a thread-pool size of 6. If you create multiple
  // publishers and subscribers, they all should share the same Executor.
  std::shared_ptr<tcpub::Executor> executor = std::make_shared<tcpub::Executor>(6);

  // Create a subscriber
  tcpub::Subscriber hello_world_subscriber(executor);
  
  // Add a session to the subscriber that connects to port 1588 on localhost. A 
  // subscriber will aggregate traffic from multiple source, if you add multiple
  // sessions.
  hello_world_subscriber.addSession("127.0.0.1", 1588);

  // Create a Callback that will be called each time a data packet is received.
  // This function will create an std::string from the packet and print it to
  // the console.
  std::function<void(const tcpub::CallbackData& callback_data)> callback_function
        = [](const tcpub::CallbackData& callback_data) -> void
          {
            std::cout << "Received playload: "
                      << std::string(callback_data.buffer_->data(), callback_data.buffer_->size())
                      << std::endl;
          };

  // Set the callback to the subsriber
  hello_world_subscriber.setCallback(callback_function);
    
  // Prevent the application from exiting immediatelly
  for (;;) std::this_thread::sleep_for(std::chrono::milliseconds(500));
  return 0;
}
```

## How to checkout and build

There are several examples provided that aim to show you the functionality.

1. Install cmake and git / git-for-windows

2. Checkout this repo and the asio submodule
	```console
	git clone https://github.com/continental/tcpub.git
	cd tcpub
	git submodule init
	git submodule update
	```

3. CMake the project *(Building as debug will add some debug output)*
	```console
	mkdir _build
	cd _build
	cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=_install
	```

4. Build the project
	- Linux: `make`
	- Windows: Open `_build\fineftp.sln` with Visual Studio and build the example project

5. Start either of the example pairs on the same machine.
	- `hello_world_publisher /.exe` + `hello_world_subscriber /.exe`
	  *or*
	- `performance_publisher /.exe` + `performance_subscriber /.exe`

## The Protocol (Version 0)

When using this library, you do not need to know how the protocol works. Both the subscriber and receiver are completely implemented and ready for you to use. This section is meant for advanced users that are interested in the underlying protocol.

<details>
<summary>Show</summary>

### Message flow

The Protocol is quite simple:

1. The **Subsriber** connects to the publisher and sends a ProtocolHandshakeRequest. This Message contains the maximum protocol Version the Subscriber supports

2. The **Publisher** returns a ProtocolHandshakeResponse. This message contains the protocol version that will be used from now on. The version must not be higher than the version sent by the subsriber.

3. The **Publisher** starts sending data to the subsriber.

_The ProtocolHandshake is meant to provide future-proof expansions. At the moment the only available protocol version is 0._

```
Subscriber                     Publisher
   |                               |
   |  -> ProtocolHandshakeReq  ->  |
   |                               |
   |  <- ProtocolHandshakeResp <-  |
   |                               |
   |  <--------- DATA <----------  |
   |  <--------- DATA <----------  |
   |  <--------- DATA <----------  |
   |              ...              |
```

### Message layout

The protocol uses the following message layout. Values that are not sent by the sender are to be interpreted as 0.

- **General Message header**
	Each message will have a message header as follows. Values are to be interpreted little-endian.
	This header is defined in [tcpub/src/tcp_header.h](tcpub/src/tcp_header.h)

	- 16 bit: Header size
	- 8 bit: Type
		- 0 = Regular Payload
		- 1 = Handshake Message
	- 8 bit: Reserved
		- Must be 0
	- 64bit: Payload size

2. **ProtocolHandshakeReq & ProtocolHandshakeResp**
	The layout of ProtocolHandshakeReq / ProtocolHandshakeResp is the same.  Values are to be interpreted little-endian
	This message is defined in [tcpub/src/protocol_handshake_message.h](tcpub/src/protocol_handshake_message.h)
	
	- Message Header (size given in the first 16 bit)
	- 8 bit: Protocol Version

</details>
