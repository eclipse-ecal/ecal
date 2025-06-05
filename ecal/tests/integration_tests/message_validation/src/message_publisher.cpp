#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include "test_message.pb.h"
#include <tclap/CmdLine.h>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
    try
    {
        TCLAP::CmdLine cmd("eCAL Message Publisher", ' ', "1.0");

        TCLAP::ValueArg<std::string> mode_arg("m", "mode", "Transport mode", true, "", "string");
        TCLAP::ValueArg<std::string> topic_arg("t", "topic", "Topic name", false, "test_message", "string");
        TCLAP::ValueArg<std::string> name_arg("n", "name", "eCAL node name", false, "msg_publisher", "string");
        TCLAP::SwitchArg malformed_arg("", "malformed", "Send a malformed Protobuf message", false);
        cmd.add(mode_arg);
        cmd.add(topic_arg);
        cmd.add(name_arg);
        cmd.add(malformed_arg);
        cmd.parse(argc, argv);

        setup_ecal_configuration(mode_arg.getValue(), true, name_arg.getValue());
        std::cout << "[Publisher] Configuration set. Mode: " << mode_arg.getValue() << std::endl;

        eCAL::CPublisher pub(topic_arg.getValue());
        wait_for_subscriber(topic_arg.getValue(), 1, 5000);

        if (malformed_arg.getValue())
        {
            std::string malformed_data = "This is not a Protobuf message";
            std::cout << "[Publisher] Sending malformed message: " << malformed_data << std::endl;
            pub.Send(malformed_data.c_str(), malformed_data.size());
        }
        else
        {
            test::TestMessage msg;
            msg.set_id(1);
            msg.set_payload("Hello");

            std::string serialized_msg;
            if (msg.SerializeToString(&serialized_msg))
            {
                std::cout << "[Publisher] Sending serialized Protobuf message with payload: Hello" << std::endl;
                pub.Send(serialized_msg.c_str(), serialized_msg.size());
            }
            else
            {
                std::cerr << "[Publisher] Failed to serialize Protobuf message." << std::endl;
                return 1;
            }
        }

        eCAL::Finalize();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "TCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
        return 1;
    }

    return 0;
}
