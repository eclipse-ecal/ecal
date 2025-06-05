#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <vector>
#include <thread>

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Multi Publisher", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode (e.g., network_udp)", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<std::string> name_arg("n", "name", "eCAL node name", false, "multi_publisher", "string");
    TCLAP::ValueArg<int> count_arg("c", "count", "Number of messages to send", false, 15, "int");
    TCLAP::ValueArg<int> delay_arg("d", "delay", "Delay between sends in milliseconds", false, 1000, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(name_arg);
    cmd.add(count_arg);
    cmd.add(delay_arg);
    cmd.parse(argc, argv);

    const std::string mode = mode_arg.getValue();
    const std::string topic = topic_arg.getValue();
    const std::string node_name = name_arg.getValue();
    const int count = count_arg.getValue();
    const int delay_ms = delay_arg.getValue();

    setup_ecal_configuration(mode, true, node_name);

    eCAL::CPublisher pub(topic);
    std::vector<unsigned char> buffer(10, 43);
    wait_for_subscriber(topic_arg.getValue(), 2, 5000);

    std::cout << "[Publisher] Started with mode=" << mode
              << ", topic=" << topic
              << ", node=" << node_name
              << ", count=" << count
              << ", delay=" << delay_ms << "ms" << std::endl;

    for (int i = 0; i < count && eCAL::Ok(); ++i)
    {
      pub.Send(buffer.data(), buffer.size());
      //std::cout << "[Publisher] Sent buffer with content: 43 (message " << i + 1 << ")" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    std::cout << "\n[Publisher] Finished sending messages." << std::endl;
    eCAL::Finalize();
    return 0;
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "\nTCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }
}
