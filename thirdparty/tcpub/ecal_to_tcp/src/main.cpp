// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#include <iostream>
#include <thread>
#include <atomic>
#include <thread>
#include <map>

#include <tcpub/executor.h>
#include <tcpub/publisher.h>

#include <ecal/ecal.h>
#include <ecal/msg/subscriber.h>

std::map<std::string, std::atomic<int>> messages_sent;

void printLog()
{
  for (;;)
  {
    std::cout << "Messages sent:" << std::endl;
    for (auto& message_sent_pair : messages_sent)
    {
      int messages_sent_temp = message_sent_pair.second.exchange(0);
      std::cout << "  " << message_sent_pair.first << ": " << messages_sent_temp << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

int main(int argc, char** argv)
{
  uint16_t                           start_port;
  std::vector<std::string>           topics;
  std::vector<eCAL::CSubscriber>     ecal_subscribers_;
  std::vector<tcpub::Publisher>      tcp_publishers_;

  if (argc < 3)
  {
    std::cerr << "ecal_to_tcp <start_port> <topic_1> [topic_2 topic_3...]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "  start_port: The port that will be used for the first topic. It will then be increased by 1 for each additional topic." << std::endl;
    std::cerr << "  topic_x:    The eCAL topics this appliation shall subsribe to and forward via TCP." << std::endl;
    std::cerr << std::endl;
    return 1;
  }

  eCAL::Initialize(0, nullptr, "ecal2tcp");

  // Parse command line
  start_port = static_cast<uint16_t>(std::stoul(argv[1]));
  topics.reserve(argc - 1);
  for (int i = 2; i < argc; i++)
  {
    topics.push_back(std::string(argv[i]));
  }

  // Reserve space for publishers and subsribers
  ecal_subscribers_.reserve(topics.size());
  tcp_publishers_  .reserve(topics.size());

  // Create executor
  auto executor = std::make_shared<tcpub::Executor>(2);

  // Create publishers
  for (int i = 0; i < topics.size(); i++)
  {
    std::cout << "Publishing " << topics[i] << " on port " << (start_port + i) << std::endl;

    tcp_publishers_  .emplace_back(executor, start_port + i);
    ecal_subscribers_.emplace_back(topics[i]);

    eCAL::ReceiveCallbackT callback
              = [&tcp_publisher = tcp_publishers_.back()](const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
                {
                  tcp_publisher.send(static_cast<char*>(data_->buf), data_->size);
                  messages_sent[topic_name_]++;
                };

    ecal_subscribers_.back().AddReceiveCallback(callback);

    messages_sent[topics[i]] = 0;
  }

  printLog();
}
