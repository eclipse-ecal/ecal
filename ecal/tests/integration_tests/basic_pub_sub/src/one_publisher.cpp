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
    TCLAP::CmdLine cmd("eCAL Publisher", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<std::string> name_arg("n", "name", "eCAL node name", false, "pub_test", "string");
    TCLAP::ValueArg<int> count_arg("c", "count", "Number of messages", false, 3, "int");
    TCLAP::ValueArg<int> delay_arg("d", "delay", "Delay between sends (ms)", false, 1000, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(name_arg);
    cmd.add(count_arg);
    cmd.add(delay_arg);
    cmd.parse(argc, argv);

    setup_ecal_configuration(mode_arg.getValue(), true, name_arg.getValue());

    eCAL::CPublisher pub(topic_arg.getValue());
    std::vector<unsigned char> bin_buffer(10, 42);

    wait_for_subscriber(topic_arg.getValue(), 1, 5000);

    for (int i = 0; i < count_arg.getValue() && eCAL::Ok(); ++i)
    {
      pub.Send(bin_buffer.data(), bin_buffer.size());
      std::cout << "[Publisher] Sent binary buffer with content: 42" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_arg.getValue()));
    }

    eCAL::Finalize();
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }

  return 0;
}
