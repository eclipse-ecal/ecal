#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> count_42(0);
std::atomic<int> count_43(0);
std::atomic<int> count_44(0);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  if (data_.buffer_size > 0)
  {
    int value = static_cast<int>(static_cast<const unsigned char*>(data_.buffer)[0]);
    if (value == 42) ++count_42; // 42 = Local UDP
    if (value == 43) ++count_43; // 43 = Network UDP
    if (value == 44) ++count_44; // 44 = Local UDP 2 after network crash

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
    TCLAP::ValueArg<int> wait_arg("w", "wait", "Total wait duration (s)", false, 30, "int");

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

    std::cout << "\n=== Message Summary ===\n\n";
    std::cout << "Local UDP PUB1 messages (42):--------------> " << count_42.load() << "/25\n";
    std::cout << "Network UDP PUB2 messages (43):-------------> " << count_43.load() << "/25\n";
    std::cout << "Local UDP PUB3 messages after crash (44):--> " << count_44.load() << "/15\n\n";

    bool enough_42 = count_42 = 25;
    bool enough_43 = count_43 > 2 && count_43 < 10;
    bool enough_44 = count_44 = 15;

    if (enough_42 && enough_43 && enough_44)
    {
      std::cout << "\n[✓] All expected messages received: local before and after crash, and network.\n";
      return 0;
    }
    else
    {
      std::cerr << "\n[✗] Missing expected messages from one or more sources.\n";
      return 1;
    }

  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")\n";
    return 1;
  }
}
