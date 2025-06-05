#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> message_count(0);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  ++message_count;
  std::cout << "[Test_Sub] Received " << data_.buffer_size << " bytes (message " << message_count.load() << ")\n";
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Test Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<int> timeout_arg("w", "wait", "Wait in seconds", false, 30, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(timeout_arg);
    cmd.parse(argc, argv);

    setup_ecal_configuration(mode_arg.getValue(), false, "test_sub");

    eCAL::CSubscriber sub(topic_arg.getValue());
    sub.SetReceiveCallback(OnReceive);
    std::cout << "[Test_Sub] Ready and waiting...\n";

    std::this_thread::sleep_for(std::chrono::seconds(timeout_arg.getValue()));
    eCAL::Finalize();

    std::cout << "[✓] Received " << message_count << " messages.\n";
    return message_count > 2 ? 0 : 1;
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")\n";
    return 1;
  }
}
