// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include <iostream>
#include <thread>

#include <tcpub/executor.h>
#include <tcpub/publisher.h>

int main()
{
  std::shared_ptr<tcpub::Executor> executor = std::make_shared<tcpub::Executor>(6);

  int counter = 0;
  tcpub::Publisher hello_world_publisher(executor, 1588);

  for (;;)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::string data_to_send = "Hello World " + std::to_string(++counter);
    auto now = std::chrono::steady_clock::now();

    std::cout << "Sending " << data_to_send << std::endl;
    hello_world_publisher.send(&data_to_send[0], data_to_send.size());
  }

  return 0;
}
