#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <chrono>
#include <thread>

std::chrono::steady_clock::time_point g_start;

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - g_start).count();

  std::cout << "[Crash_Sub] Received " << data_.buffer_size << " bytes\n";

  if (elapsed >= 2)
  {
    std::cerr << "[Crash_Sub] Simulating crash after 2 seconds during active reception\n";
    std::abort();
  }
}

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Crash Send Subscriber", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode (e.g., shm, udp)", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<std::string> name_arg("n", "name", "eCAL node name", false, "crash_send_sub", "string");
    TCLAP::ValueArg<int> wait_arg("w", "wait", "Total runtime before finalizing (sec)", false, 20, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(name_arg);
    cmd.add(wait_arg);
    cmd.parse(argc, argv);

    setup_ecal_configuration(mode_arg.getValue(), false, name_arg.getValue());

    eCAL::CSubscriber sub(topic_arg.getValue());
    sub.SetReceiveCallback(OnReceive);

    std::cout << "[Crash_Sub] Listening to topic '" << topic_arg.getValue()
              << "' in mode '" << mode_arg.getValue() << "'\n";

    g_start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(wait_arg.getValue()));

    std::cout << "[Crash_Sub] Finished without crash (unexpected)" << std::endl;
    eCAL::Finalize();
    return 0;
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "[Crash_Sub] TCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }
}
