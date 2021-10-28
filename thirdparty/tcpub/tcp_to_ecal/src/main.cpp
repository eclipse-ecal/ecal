// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include <iostream>
#include <thread>
#include <atomic>
#include <thread>
#include <map>

#include <tcpub/executor.h>
#include <tcpub/subscriber.h>

#include <ecal/ecal.h>
#include <ecal/msg/publisher.h>

std::map<std::string, std::atomic<int>> messages_received;

void printLog()
{
  for (;;)
  {
    std::cout << "Messages received:" << std::endl;
    for (auto& message_sent_pair : messages_received)
    {
      int messages_sent_temp = message_sent_pair.second.exchange(0);
      std::cout << "  " << message_sent_pair.first << ": " << messages_sent_temp << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main(int argc, char** argv)
{
  std::string                         ip;
  uint16_t                            start_port;
  std::vector<std::string>            topics;
  std::vector<tcpub::Subscriber> tcp_subscribers;
  std::vector<eCAL::CPublisher>       ecal_publishers;

  if (argc < 4)
  {
    std::cerr << "tcp_to_ecal <ip> <start_port> <topic_1> [topic_2 topic_3...]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  ip:         The IP address (or hostname) that the ecal_to_tcp application is running on." << std::endl;
    std::cerr << "  start_port: The port that will be used for the first topic. It will then be increased by 1 for each additional topic." << std::endl;
    std::cerr << "  topic_x:    The eCAL topic to publish the received data to. Tip: Change the topic name to something different, e.g. append \"_fromtcp\", so you don't receive dupliate messages late." << std::endl;
    std::cerr << std::endl;
    return 1;
  }

  eCAL::Initialize(0, nullptr, "tcp2ecal");

  // Parse command line
  ip = argv[1];
  start_port = static_cast<uint16_t>(std::stoul(argv[2]));
  for (int i = 3; i < argc; i++)
  {
    topics.push_back(std::string(argv[i]));
    messages_received[argv[i]] = 0;
  }

  // Reserver space for publishers and subsribers
  tcp_subscribers.reserve(topics.size());
  ecal_publishers.reserve(topics.size());

  auto executor = std::make_shared<tcpub::Executor>(2);

  // Create publishers
  for (int i = 0; i < topics.size(); i++)
  {
    
    std::cout << "Subscribing " << topics[i] << " on " << ip << ":" << (start_port + i) << std::endl;
    
    ecal_publishers.emplace_back(topics[i]);

    tcp_subscribers.emplace_back(executor);
    tcp_subscribers.back().addSession(ip, start_port + i);
    tcp_subscribers.back().setCallback(
              [&ecal_publisher = ecal_publishers.back(), topic_name = topics[i]](const tcpub::CallbackData& callback_data)
              {
                ecal_publisher.Send(callback_data.buffer_->data(), callback_data.buffer_->size());
                messages_received[topic_name]++;
              });
  }

  printLog();
}
