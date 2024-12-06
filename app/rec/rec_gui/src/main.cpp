/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include "ecalrec_gui.h"

#include <thread>

#include <QtWidgets/QApplication>
#include <ecal/ecal.h>

#include "qecalrec.h"

#include <rec_client_core/ecal_rec_defs.h>

#include <ecal_utils/command_line.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505) // disable tclap warning (unreferenced local function has been removed)
#endif
#include "tclap/CmdLine.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <custom_tclap/advanced_tclap_output.h>
#include "help_window.h"


int main(int argc, char *argv[])
{
  // Define the command line object.
  TCLAP::CmdLine cmd("eCAL Rec GUI", ' ', ECAL_REC_VERSION_STRING, false);

  /////////////////////////////////////////////////
  /// Rec Server args
  /////////////////////////////////////////////////

  // Handle parameters
  TCLAP::UnlabeledMultiArg<std::string> unlabled_config_arg            ("config_", "The Configuration file to load.", false, "Path");

  TCLAP::SwitchArg                      help_arg                       ("h", "help",                  "Displays help.",                 false);
  TCLAP::SwitchArg                      version_arg                    ("v", "version",               "Displays version information.",  false);
  TCLAP::ValueArg<std::string>          config_arg                     ("c", "config",                "The configuration file to load.", false, "", "Path");

  TCLAP::SwitchArg                      activate_arg                   ("",  "activate",              "Activate the recorder and all clients.", false);
  TCLAP::SwitchArg                      start_recording_arg            ("r", "record",                "Start a new recording.", false);

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &help_arg,
    &version_arg,
    &config_arg,
    &activate_arg,
    &start_recording_arg,
    &unlabled_config_arg,
  };

  for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); arg_iterator++)
  {
    cmd.add(*arg_iterator);
  }

  std::stringstream tclap_output_stream;
  CustomTclap::AdvancedTclapOutput advanced_tclap_output(std::vector<std::ostream*>{ &tclap_output_stream, &std::cout }, 75);
  advanced_tclap_output.setArgumentHidden(&unlabled_config_arg, true);

  cmd.setOutput(&advanced_tclap_output);

  try
  {
#ifdef WIN32
    auto utf8_args_vector = EcalUtils::CommandLine::GetUtf8Argv();
    cmd.parse(utf8_args_vector);
#else
    cmd.parse(argc, argv);
#endif // WIN32
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
  }

  // Create and configure the QApplication
  QApplication a(argc, argv);
  a.setOrganizationName      ("Continental");
  a.setOrganizationDomain    ("continental-corporation.com");
  a.setApplicationName       ("ecalrecgui");
  a.setApplicationDisplayName("eCAL Recorder");

  //////////////////////////////////////
  // --help
  // --version
  //////////////////////////////////////
  if (help_arg.isSet() || version_arg.isSet())
  {
    tclap_output_stream.clear();

    if (help_arg.isSet())
    {
      advanced_tclap_output.usage(cmd);
    }
    else if (version_arg.isSet())
    {
      advanced_tclap_output.version(cmd);
    }

#ifdef WIN32
    // On Windows we display the command line help as GUI window. That approach
    // is somehow standard on Windows, as Windows will not print the
    // stdout/stderr streams to the console the app was started from.

    HelpWindow w(tclap_output_stream.str());

    w.show();
    return a.exec();
#else // WIN32
    return 0;
#endif // WIN32
  }

  // Just make sure that eCAL is initialized
  eCAL::Initialize(0, nullptr, "eCALRecGUI", eCAL::Init::Default | eCAL::Init::Service | eCAL::Init::Monitoring);
  eCAL::Monitoring::SetFilterState(false);

  EcalRecGui* w = new EcalRecGui();
  w->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

  QEcalRec::instance()->setParent(w);

  //////////////////////////////////////
  // --config
  //////////////////////////////////////
  
  std::string start_config = "";

  if (config_arg.isSet())
  {
    start_config = config_arg.getValue();
  }
  else if (unlabled_config_arg.isSet())
  {
    start_config = unlabled_config_arg.getValue()[0];
  }

  if (!start_config.empty())
  {
    QEcalRec::instance()->loadConfigFromFile(start_config);
  }

  //////////////////////////////////////
  // --activate
  //////////////////////////////////////
  if (activate_arg.isSet())
  {
    bool success = QEcalRec::instance()->connectToEcal(true);
    if (!success)
      std::cerr << "Failed connecting to eCAL" << std::endl;
  }

  //////////////////////////////////////
  // --record
  //////////////////////////////////////
  if (start_recording_arg.isSet())
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    QEcalRec::instance()->startRecording();
  }

  w->show();
  int return_code = a.exec();

  eCAL::Finalize();

  return return_code;
}
