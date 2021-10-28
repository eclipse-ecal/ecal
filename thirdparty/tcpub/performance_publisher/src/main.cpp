// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include <iostream>
#include <thread>
#include <atomic>
#include <thread>
#include <vector>

#include <tcpub/executor.h>
#include <tcpub/publisher.h>

std::atomic<int> messages_sent;

void printLog()
{
  for (;;)
  {
    int messages_sent_temp = messages_sent.exchange(0);
    std::cout << "Sent " << messages_sent_temp << " messages in 1 second" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main() {
  std::shared_ptr<tcpub::Executor> executor = std::make_shared<tcpub::Executor>(6, tcpub::logger::logger_no_verbose_debug);

  tcpub::Publisher publisher(executor, "0.0.0.0", 1588);

  std::thread print_thread(printLog);

  std::vector<char> big_buffer;
  big_buffer.resize(16 * 1024 * 1024);

  auto next_send_time        = std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(0));
  auto delay_between_sending = std::chrono::milliseconds(10);

  for (;;)
  {
    {
      auto now = std::chrono::steady_clock::now();

      if (now >= next_send_time)
      {
        next_send_time = now + delay_between_sending;
      }
      else
      {
        auto time_to_sleep = next_send_time - now;
        next_send_time = next_send_time + delay_between_sending;
        std::this_thread::sleep_for(time_to_sleep);
      }
    }
    {
      auto now  = std::chrono::steady_clock::now();

      publisher.send(&big_buffer[0], big_buffer.size());
      messages_sent++;
    }
  }
}
