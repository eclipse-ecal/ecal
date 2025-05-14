#include <EcalConfigHelper/ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

std::atomic<bool> g_received(false);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size > 0)
  {
    int value = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);
    std::ofstream log_file("/app/received.log", std::ios::out | std::ios::trunc);
    log_file << value;
    log_file.close();

    std::cout << "[Subscriber] Received value: " << value << std::endl;
    g_received = true;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Crash Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<int> timeout_arg("w", "wait", "Wait duration (seconds)", false, 20, "int");
    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(timeout_arg);
    cmd.parse(argc, argv);

    std::string node_name = std::string(argv[0]);
    setup_ecal_configuration(mode_arg.getValue(), false, node_name);

    eCAL::CSubscriber sub(topic_arg.getValue());
    std::cout << "[Subscriber] Registering callback and waiting for messages..." << std::endl;
    sub.SetReceiveCallback(OnReceive);

    std::this_thread::sleep_for(std::chrono::seconds(timeout_arg.getValue()));
    std::cout << "[Subscriber] Finalize eCAL..." << std::endl;
    eCAL::Finalize();

    if (g_received)
    {
      std::cout << "[Subscriber] Communication successful!" << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "[Subscriber] Communication failed!" << std::endl;
      return 1;
    }
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }
}
