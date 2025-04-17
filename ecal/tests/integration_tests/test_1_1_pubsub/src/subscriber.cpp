#include <ecal/ecal.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

// Callback function for incoming messages
void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size < 1) return;

  // Extract first byte as integer value
  int value = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);

  // Log to file
  std::ofstream log_file("/app/received.log", std::ios::out | std::ios::trunc);
  log_file << value;
  log_file.close();

  std::cout << "[Subscriber] Received value: " << value << std::endl;
}

int main()
{
  eCAL::Initialize("sub_test_1_1");

  eCAL::CSubscriber sub("test_topic");
  sub.SetReceiveCallback(OnReceive);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  eCAL::Finalize();
  return 0;
}
