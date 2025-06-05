#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("Large Publisher", ' ', "1.0");

    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
    TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_topic", "string");
    TCLAP::ValueArg<std::string> name_arg("n", "name", "eCAL node name", false, "large_publisher", "string");
    TCLAP::ValueArg<int> count_arg("c", "count", "Number of messages", false, 3, "int");
    TCLAP::ValueArg<int> delay_arg("d", "delay", "Delay between sends (ms)", false, 1000, "int");

    cmd.add(mode_arg);
    cmd.add(topic_arg);
    cmd.add(name_arg);
    cmd.add(count_arg);
    cmd.add(delay_arg);
    cmd.parse(argc, argv);

    setup_ecal_configuration(mode_arg.getValue(), true, name_arg.getValue());


    eCAL::Publisher::Configuration pub_config;
    //pub_config.layer.shm.memfile_buffer_count = 3;
    //pub_config.layer.shm.memfile_min_size_bytes = 209715200; // 200 MB
    //pub_config.layer.shm.memfile_reserve_percent = 30;              // optional
    //pub_config.layer.shm.zero_copy_mode = true;

    eCAL::CPublisher pub(topic_arg.getValue(), eCAL::SDataTypeInformation(), pub_config);

    //eCAL::CPublisher pub(topic_arg.getValue());

    // Create 1 GB message
    //std::string buffer(1024L * 1024L * 1024L, 'X');
    //std::cout << "[Publisher] Prepared 1GB message.\n";
    //Crashes to big

    //std::string buffer(500L * 1024L * 1024L, 'X');
    //std::cout << "[Publisher] Prepared 500MB message.\n";
    // Does not get sendet by network UDP must be to big

    std::string buffer(50L * 1024L * 1024L, 'X');
    std::cout << "[Publisher] Prepared 100MB message.\n";

    // This can be used for UDP Local for testing
    //std::string buffer(1L * 1024L, 'X');
    //std::cout << "[Publisher] Small UDP Local message.\n";
    //UDP Local has a message size limit

    wait_for_subscriber(topic_arg.getValue(), 2, 50000);
    std::cout << "[Publisher] Subscriber detected. Starting to send.\n";

    for (int i = 0; i < count_arg.getValue() && eCAL::Ok(); ++i)
    {
      bool sent = pub.Send(buffer.data(), buffer.size());
      std::cout << "[Publisher] Send result: " << (sent ? "✓" : "✗") << ", size: " << buffer.size() << " bytes\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_arg.getValue()));
    }

    std::cout << "[Publisher] Finished sending.\n";
    eCAL::Finalize();
    return 0;
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")\n";
    return 1;
  }
}
