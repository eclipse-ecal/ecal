#pragma once

#include <string>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

void wait_for_subscriber(const std::string& topic_name, int min_subscribers, int timeout_ms);
void setup_ecal_configuration(const std::string& mode, bool is_publisher, const std::string& node_name);

