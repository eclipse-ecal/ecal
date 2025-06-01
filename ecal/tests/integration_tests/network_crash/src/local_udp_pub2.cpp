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
    TCLAP::CmdLine cmd("Local UDP Publisher 2", ' ', "1.0");

    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<std::string> name_arg("n", "name", "Node name", false, "local_udp_pub_2", "string");
    TCLAP::ValueArg<int> count_arg("c", "count", "Number of messages", false, 15, "int");
    TCLAP::ValueArg<int> delay_arg("d", "delay", "Delay between sends (ms)", false, 1000, "int");

    cmd.add(topic_arg);
    cmd.add(name_arg);
    cmd.add(count_arg);
    cmd.add(delay_arg);
    cmd.parse(argc, argv);

    setup_ecal_configuration("local_udp", true, name_arg.getValue());

    eCAL::CPublisher pub(topic_arg.getValue());
    std::vector<unsigned char> buffer(10, 44);  // 44 = new value

    wait_for_subscriber(topic_arg.getValue(), 1, 5000);

    for (int i = 0; i < count_arg.getValue() && eCAL::Ok(); ++i)
    {
      pub.Send(buffer.data(), buffer.size());
      //std::cout << "[Local UDP Publisher 2] Sent message 44 (" << i + 1 << ")\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_arg.getValue()));
    }

    eCAL::Finalize();
    return 0;
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")\n";
    return 1;
  }
}
