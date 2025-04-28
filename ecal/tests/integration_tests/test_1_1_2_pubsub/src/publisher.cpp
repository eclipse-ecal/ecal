#include "ecal_config_helper.h"
#include <ecal/ecal.h>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
  eCAL::Initialize("pub_test_1_1_2");

  if (argc < 2)
  {
    std::cerr << "[Publisher] Usage: publisher <mode>" << std::endl;
    return 1;
  }

  const std::string mode = argv[1];
  setup_ecal_configuration(mode, true); // true = Publisher

  std::vector<unsigned char> bin_buffer(10, 42);
  eCAL::CPublisher pub("test_topic");

  eCAL::Process::SleepMS(5000);

  pub.Send(bin_buffer.data(), bin_buffer.size());
  std::cout << "[Publisher] Sent binary buffer with content: 42" << std::endl;

  eCAL::Process::SleepMS(5000);
  eCAL::Finalize();
  return 0;
}
