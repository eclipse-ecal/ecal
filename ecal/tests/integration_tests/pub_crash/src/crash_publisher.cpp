#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Crash Publisher", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<int> total_arg("c", "count", "Total messages", false, 30, "int");
    TCLAP::ValueArg<int> crash_at_arg("", "crash_at", "Crash after N messages", false, 10, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(total_arg);
    cmd.add(crash_at_arg);
    cmd.parse(argc, argv);

    std::string node_name = std::string(argv[0]);
    setup_ecal_configuration(mode_arg.getValue(), true, node_name);

    eCAL::CPublisher pub(topic_arg.getValue());
    std::vector<unsigned char> buf(10, 42);
    wait_for_subscriber(topic_arg.getValue(), 1, 5000);

    std::cout << "[Crash Publisher] Starting and will crash after 10 messages..." << std::endl;
    for (int i = 0; i < total_arg.getValue(); ++i)
    {
      pub.Send(buf.data(), buf.size());
      std::cout << "[Crash Publisher] Sent buffer with content: 42 (message " << i + 1 << ")" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      if (i == crash_at_arg.getValue())
      {
        std::cout << "[Crash Publisher] Crashing after message " << i << std::endl;
        std::abort();
      }
    }
    std::cout << "[Crash Publisher] Crashing now." << std::endl;
    std::abort();
  }
  catch (...)
  {
    std::cerr << "Unexpected crash!" << std::endl;
    return 1;
  }
}
