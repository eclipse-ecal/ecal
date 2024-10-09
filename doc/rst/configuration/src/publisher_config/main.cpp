#include <ecal/ecal.h>
#include <ecal/msg/string/publisher.h>
#include <string>
#include <stdexcept>
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "PublisherConfig", eCAL::Init::All);

  // create publisher config
  eCAL::Publisher::Configuration pub_config;

  // disable all layers except for SHM
  pub_config.layer.shm.enable = true;
  pub_config.layer.udp.enable = false;
  pub_config.layer.tcp.enable = false;

  // create publisher 1
  eCAL::string::CPublisher<std::string> pub_1("topic_1", pub_config);

  // enable for the second publisher also tcp
  pub_config.layer.tcp.enable = true;

  // create publisher 2
  eCAL::string::CPublisher<std::string> pub_2("topic_2", pub_config);

  int counter {0};
  while (eCAL::Ok())
  {
    std::string msg = "Send message number: " + std::to_string(counter++);
    
    // send message
    pub_1.Send(msg);
    pub_2.Send(msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // finalize eCAL API
  eCAL::Finalize();
}