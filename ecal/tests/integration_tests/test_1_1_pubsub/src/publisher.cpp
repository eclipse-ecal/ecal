#include <ecal/ecal.h>
#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
  eCAL::Initialize("pub_test_1_1");

  // Create binary buffer filled with value 42
  std::vector<unsigned char> bin_buffer(10, 42);

  eCAL::CPublisher pub("test_topic");

  eCAL::Process::SleepMS(5000); // wait for subscriber

  pub.Send(bin_buffer.data(), bin_buffer.size());
  std::cout << "[Publisher] Sent binary buffer with content: 42" << std::endl;

  eCAL::Process::SleepMS(5000); // allow time for send

  eCAL::Finalize();
  return 0;
}
