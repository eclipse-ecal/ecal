#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> count_42(0);
std::atomic<int> count_43(0);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size > 0)
  {
    int value = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);
    if (value == 42) ++count_42;
    if (value == 43) ++count_43;

    std::cout << "[Multi Subscriber] Received value: " << value << std::endl;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Multi Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<int> wait_arg("w", "wait", "Wait duration (seconds)", false, 18, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(wait_arg);
    cmd.parse(argc, argv);

    setup_ecal_configuration(mode_arg.getValue(), false, "multi_subscriber");

    eCAL::CSubscriber sub(topic_arg.getValue());
    sub.SetReceiveCallback(OnReceive);

    std::cout << "[Multi Subscriber] Waiting for messages..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(wait_arg.getValue()));

    eCAL::Finalize();

    std::cout << "[Summary] Received " << count_42 << " x 42, " << count_43 << " x 43" << std::endl;

    if (count_42 >= 14 && count_43 >= 14)
    {
      std::cout << "[✓] Received messages from both publishers." << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "[✗] Missing messages from one or both publishers." << std::endl;
      return 1;
    }
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }
}
