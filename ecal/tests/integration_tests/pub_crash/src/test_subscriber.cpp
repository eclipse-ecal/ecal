#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

std::atomic<int> count_42(0);
std::atomic<int> count_43(0);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size > 0)
  {
    int value = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);
    std::cout << "[Subscriber] Received value: " << value << std::endl;

    if (value == 42) ++count_42;
    if (value == 43) ++count_43;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Test Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<int> timeout_arg("w", "wait", "Wait duration (seconds)", false, 35, "int");
    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(timeout_arg);
    cmd.parse(argc, argv);

    std::string node_name = std::string(argv[0]);
    setup_ecal_configuration(mode_arg.getValue(), false, node_name);

    eCAL::CSubscriber sub(topic_arg.getValue());
    std::cout << "\n[Subscriber] Registering callback and waiting for messages...\n" << std::endl;
    sub.SetReceiveCallback(OnReceive);

    std::this_thread::sleep_for(std::chrono::seconds(timeout_arg.getValue()));
    eCAL::Finalize();

    std::cout << "[Summary] Received " << count_42 << " x 42 (crash pub), "
          << count_43 << " x 43 (test pub)" << std::endl;

    if (count_43 >= 25)
    {
    std::cout << "\n[✓] Communication continued after crash!" << std::endl;
    if (count_42 < 11 || count_42 > 4)
     {
      std::cout << "\n[✓] One Publisher crashed and no more messages received from the crashed publisher!" << std::endl;
      return 0;
     }
     else
     {
      std::cout << "\n[✗] One Publisher crashed and still messages received from the crashed publisher!" << std::endl;
      return 1;
     }
    }
    else
    {
    std::cerr << "\n[✗] Communication did not continue!" << std::endl;
    return 1;
    }

  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "\nTCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }
}
