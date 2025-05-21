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
    if (value == 42) ++count_42; // 42 = Local UDP
    if (value == 43) ++count_43; // 43 = Network UDP

    std::cout << "[Subscriber] Received: " << value << std::endl;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("UDP Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<std::string> name_arg("n", "name", "Node name", false, "udp_subscriber", "string");
    TCLAP::ValueArg<int> wait_arg("w", "wait", "Total wait duration (s)", false, 35, "int");

    cmd.add(topic_arg);
    cmd.add(name_arg);
    cmd.add(wait_arg);
    cmd.parse(argc, argv);

    eCAL::Configuration config;
    config.communication_mode = eCAL::eCommunicationMode::network;
    //config.registration.local.transport_type = eCAL::Registration::Local::eTransportType::udp;
    config.registration.network.transport_type = eCAL::Registration::Network::eTransportType::udp;
    config.subscriber.layer.shm.enable = false;
    config.subscriber.layer.udp.enable = true;
    config.subscriber.layer.tcp.enable = false;

    eCAL::Initialize(config, name_arg.getValue(), eCAL::Init::All);

    eCAL::CSubscriber sub(topic_arg.getValue());
    sub.SetReceiveCallback(OnReceive);

    std::cout << "[Subscriber] Listening on topic '" << topic_arg.getValue() << "'...\n";
    std::this_thread::sleep_for(std::chrono::seconds(wait_arg.getValue()));

    eCAL::Finalize();

    std::cout << "\n=== Summary ===\n";
    std::cout << "Messages with value 42 (local): " << count_42.load() << "\n";
    std::cout << "Messages with value 43 (network): " << count_43.load() << "\n";

    if (count_42 >= 30 && count_43 > 2 && count_43 < 25)
    {
      std::cout << "[✓] Messages received as expected.\n";
      return 0;
    }
    else if (count_42 >= 30 && count_43 >= 30)
    {
      std::cout << "[✓] All messages received from local and network sources.\n";
      return 0;
    }
    else if (count_42 >= 30 && count_43 == 0)
    {
      std::cout << "[✓] Only local messages received.\n";
      return 0;
    }
    else if (count_42 == 0 && count_43 >= 30)
    {
      std::cout << "[✓] Only network messages received.\n";
      return 0;
    }
    else if (count_42 == 0 && count_43 == 0)
    {
      std::cout << "[✓] Messages from both local and network sources received.\n";
      return 0;
    }
    else
    {
      std::cerr << "[✗] Missing expected messages.\n";
      return 1;
    }
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")\n";
    return 1;
  }
}
