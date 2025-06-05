#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include "test_message.pb.h"
#include <tclap/CmdLine.h>
#include <iostream>
#include <fstream>
#include <atomic>

std::atomic<bool> g_received(false);
std::atomic<bool> g_valid_message(false);

void OnReceive(const eCAL::STopicId&, const eCAL::SDataTypeInformation&, const eCAL::SReceiveCallbackData& data_)
{
    std::cout << "[Subscriber] Received data of size: " << data_.buffer_size << std::endl;
    if (data_.buffer_size > 0)
    {
        test::TestMessage msg;
        if (msg.ParseFromArray(data_.buffer, static_cast<int>(data_.buffer_size)))
        {
            g_received = true;
            g_valid_message = (msg.id() == 1 && msg.payload() == "Hello");

            std::cout << "[Subscriber] Received valid Protobuf message: ID=" 
                      << msg.id() << ", Payload=" << msg.payload() << std::endl;
        }
        else
        {
            g_received = true;
            g_valid_message = false;
            std::cerr << "[Subscriber] Invalid Protobuf message. Data: " 
                      << std::string(static_cast<const char*>(data_.buffer), data_.buffer_size) 
                      << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    try
    {
        TCLAP::CmdLine cmd("eCAL Message Subscriber", ' ', "1.0");

        TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
        TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_message", "string");
        TCLAP::ValueArg<std::string> name_arg("n", "name", "eCAL node name", false, "msg_subscriber", "string");
        cmd.add(mode_arg);
        cmd.add(topic_arg);
        cmd.add(name_arg);
        cmd.parse(argc, argv);

        setup_ecal_configuration(mode_arg.getValue(), false, name_arg.getValue());

        eCAL::CSubscriber sub(topic_arg.getValue());
        sub.SetReceiveCallback(OnReceive);

        std::cout << "[Subscriber] Waiting for Protobuf message..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Listening for messages

        eCAL::Finalize();
        if (g_received && g_valid_message)
        {
            std::cout << "[Subscriber] Message received and validated successfully." << std::endl;
            return 0;
        }
        else if (g_received)
        {
            std::cerr << "[Subscriber] Invalid or malformed message received." << std::endl;
            return 1;
        }
        else
        {
            std::cerr << "[Subscriber] No message received." << std::endl;
            return 1;
        }
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
        return 1;
    }
}
