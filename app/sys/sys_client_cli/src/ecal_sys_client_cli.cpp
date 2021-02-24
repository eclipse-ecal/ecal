/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include <memory>
#include <thread>
#include <cctype>

#include <tclap/CmdLine.h>
#include <custom_tclap/advanced_tclap_output.h>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/server.h>

#include <ecal_utils/string.h>

#include "ecal_sys_client_service.h"

#include <sys_client_core/ecal_sys_client_defs.h>
#include <sys_client_core/ecal_sys_client_logger.h>

int main(int argc, char** argv)
{
  TCLAP::CmdLine cmd("eCAL Sys Client", ' ', ECAL_SYS_CLIENT_VERSION_STRING);

  TCLAP::UnlabeledMultiArg<std::string> unlabled_arg  (     "other_args_",                    "Other arguments", false, "arguments");

  // Command args
  TCLAP::SwitchArg              accept_security_risk_arg ("", "auto-accept-security-risk", "Automatically accept the security risk warning.", false);

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &accept_security_risk_arg,
    &unlabled_arg,
  };

  for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); arg_iterator++)
  {
    cmd.add(*arg_iterator);
  }

  std::stringstream tclap_output_stream;
  CustomTclap::AdvancedTclapOutput advanced_tclap_output(std::vector<std::ostream*>{ &tclap_output_stream, &std::cout }, 75);
  advanced_tclap_output.setArgumentHidden(&unlabled_arg, true);

  cmd.setOutput(&advanced_tclap_output);

  try
  {
    cmd.parse(argc, argv);
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
  }

  if (!accept_security_risk_arg.isSet())
  {
    std::string centered_string("eCAL Sys Security Warning");
    EcalUtils::String::CenterString(centered_string, ' ', 75);

    auto logger = eCAL::sys_client::EcalSysClientLogger::Instance();
    
    logger->warn(std::string(79, '='));
    logger->warn("==" + centered_string + "==");
    logger->warn(std::string(79, '='));
    logger->warn("");
    logger->warn("The eCAL Sys client will allow any eCAL Sys to execute arbitrary commands on this machine. "\
      "This means, that any PC in your local network can fully control this machine. "\
      "eCAL Sys does not implement any security measures to protect you from malicious attackers. "\
      "You MUST NOT USE eCAL Sys Client, if you do not full trust your network and every machine in it." );
    logger->warn("");
    logger->warn("Please enter [i accept] if you fully understand and accept the security risk:" );

    std::string user_input;
    std::getline (std::cin, user_input);

    std::transform(user_input.begin(), user_input.end(), user_input.begin(), [](unsigned char c) -> char { return static_cast<char>(std::tolower(static_cast<int>(c))); });
    if (user_input == std::string("i accept"))
    {
      logger->warn("");
      logger->warn("Starting eCAL Sys Client. To automatically accept the security risk, you can start eCAL Sys Client with the following parameter:" );
      logger->warn("");
      logger->warn("  " + accept_security_risk_arg.longID() );
      logger->warn("");
    }
    else
    {
      logger->error("");
      logger->error("User did not accept. Shutting down.");

      return 0;
    }
  }
  else
  {
    auto logger = eCAL::sys_client::EcalSysClientLogger::Instance();
    logger->info("eCAL Sys Client was started with " + accept_security_risk_arg.longID() + ". Not displaying security warning.");
  }

  eCAL::Initialize(argc, argv, "eCALSysClient", eCAL::Init::Default | eCAL::Init::Monitoring);

  // Print System information
  std::string config_string;
  eCAL::Process::DumpConfig(config_string);
  eCAL::sys_client::EcalSysClientLogger::Instance()->info(config_string);

  // Create sys client service
  std::shared_ptr<eCAL::pb::sys_client::EcalSysClientService> sys_client_service(new EcalSysClientService());
  eCAL::protobuf::CServiceServer<eCAL::pb::sys_client::EcalSysClientService> sys_client_service_server(sys_client_service);

  // Loop forever and accept commands
  while (eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  eCAL::Finalize();
}
