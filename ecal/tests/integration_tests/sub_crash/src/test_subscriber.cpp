#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> message_count(0);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size > 0)
  {
    int val = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);
    std::cout << "[Test_Subscriber] Received value: " << val << std::endl;
    ++message_count;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Test Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<int> timeout_arg("w", "wait", "Timeout in seconds", false, 34, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(timeout_arg);
    cmd.parse(argc, argv);

    std::string node_name = std::string(argv[0]);
    setup_ecal_configuration(mode_arg.getValue(), false, node_name);

    eCAL::CSubscriber sub(topic_arg.getValue());
    sub.SetReceiveCallback(OnReceive);
    std::cout << "\n[Test_Subscriber] Waiting for messages..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(timeout_arg.getValue()));
    eCAL::Finalize();

    if (message_count >= 30)
    {
      std::cout << "\n[✓] Communication successful, received " << message_count << " messages." << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "\n[✗] Communication failed, only received " << message_count << " messages." << std::endl;
      return 1;
    }
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "\nTCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }
}
