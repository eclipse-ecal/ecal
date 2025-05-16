#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> message_count(0);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size > 0)
  {
    int val = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);
    std::cout << "[Crash_Subscriber] Received value: " << val << std::endl;
    ++message_count;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Crashing Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<int> wait_arg("w", "wait", "Seconds before crash", false, 7, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(wait_arg);
    cmd.parse(argc, argv);

    std::string node_name = std::string(argv[0]);
    setup_ecal_configuration(mode_arg.getValue(), false, node_name);

    eCAL::CSubscriber sub(topic_arg.getValue());
    sub.SetReceiveCallback(OnReceive);

    std::cout << "\n[Crash Subscriber] Registered and sleeping before crash...\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(wait_arg.getValue()));
    std::cerr << "\n[Crash Subscriber] Simulating crash now.\n" << std::endl;
    std::cout << "\n[Crash Subscriber] Received " << message_count << " messages before crash." << std::endl;
    std::abort();  // simulate crash
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "\nTCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }
}
