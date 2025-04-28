#include "ecal_config_helper.h"
#include <ecal/ecal.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

std::atomic<bool> g_received(false);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size > 0)
  {
    int value = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);
    std::ofstream log_file("/app/received.log", std::ios::out | std::ios::trunc);
    log_file << value;
    log_file.close();

    std::cout << "[Subscriber] Received value: " << value << std::endl;
    g_received = true;
  }
}

int main(int argc, char* argv[])
{
  eCAL::Initialize("sub_test_1_1");
  
  if (argc < 2)
  {
    std::cerr << "[Subscriber] Usage: subscriber <mode>" << std::endl;
    return 1;
  }
  
  const std::string mode = argv[1];
  setup_ecal_configuration(mode, false); // false = Subscriber

 
  eCAL::CSubscriber sub("test_topic");
  sub.SetReceiveCallback(OnReceive);

  std::this_thread::sleep_for(std::chrono::seconds(10));
  eCAL::Finalize();

  if (g_received)
  {
    std::cout << "[Subscriber] Communication successful!" << std::endl;
    return 0;
  }
  else
  {
    std::cerr << "[Subscriber] Communication failed!" << std::endl;
    return 1;
  }
}
