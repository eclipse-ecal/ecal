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

#include "ecal_play_gui.h"
#include <QtWidgets/QApplication>
#include <ecal/ecal.h>

#include "q_ecal_play.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505)
#endif
#include <tclap/CmdLine.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <custom_tclap/advanced_tclap_output.h>
#include <custom_tclap/fuzzy_value_switch_arg.h>

#include "help_window.h"
#include <ecal_play_logger.h>


int main(int argc, char *argv[])
{
  //////////////////////////////////////////////////////////////////////////////
  // Parse the command line
  //////////////////////////////////////////////////////////////////////////////

  TCLAP::CmdLine cmd("eCAL Player", ' ', EcalPlayGlobals::VERSION_STRING, false);

  TCLAP::UnlabeledMultiArg<std::string> unlabled_meas_path_arg("measurement_",                "Loads the measurement from the path.",                                                                       false, "Path");
  TCLAP::SwitchArg                 help_arg                       ("h", "help",                   "Displays help.",                                                                                             false);
  TCLAP::SwitchArg                 version_arg                    ("v", "version",                "Displays version information.",                                                                              false);
  TCLAP::ValueArg<std::string>     measurement_path_arg           ("m", "measurement",            "Loads the measurement from the path.",                                                                       false, "", "Path");
  TCLAP::SwitchArg                 play_arg                       ("p", "play",                   "Automatically starts the playback. Requires a measurement being loaded.",                                    false);
  TCLAP::ValueArg<std::string>     channel_rename_file_path_arg   ("c", "channel-mapping",        "Loads the channel mapping from the given file and uses it to filter and intialize eCAL publishers. Otherwise, all publishers from the measurement will keep their default name.", false, "", "Path");
  CustomTclap::FuzzyValueSwitchArg no_play_speed_limit_arg        ("u", "unlimited-speed",        "Publish frames as fast as possible",                                                                         false, false, "true|false");
  TCLAP::ValueArg<double>          play_speed_arg                 ("s", "speed",                  "Relative rate at which the player shall publish the messages. Ignored, when using \"unlimited_play_speed\"", false, 1.0, "Factor");
  CustomTclap::FuzzyValueSwitchArg allow_framedropping_arg        ("f", "framedropping",          "Drop frames when the messages cannot be sent at the required speed",                                         false, false, "true|false");
  CustomTclap::FuzzyValueSwitchArg enforce_delay_accuracy_arg     ("d", "enforce-delay-accuracy", "Always wait the correct amount of time between two messages, even if this will slow down the playback",      false, false, "true|false");
  CustomTclap::FuzzyValueSwitchArg repeat_arg                     ("r", "repeat",                 "Repeat playback from the beginning if the end has been reached",                                             false, false, "true|false");
  TCLAP::ValueArg<double>          limit_interval_start_arg       ("l", "limit-interval-start",   "Start the playback from this time (relative value in seconds, 0.0 indicates the begin of the measurement)",  false, -1.0, "Seconds");
  TCLAP::ValueArg<double>          limit_interval_end_arg         ("e", "limit-interval-end",     "End the playback at this time (relative value in seconds)",                                                  false, -1.0, "Seconds");

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &help_arg,
    &version_arg,
    &play_arg,
    &measurement_path_arg,
    &channel_rename_file_path_arg,
    &no_play_speed_limit_arg,
    &play_speed_arg,
    &allow_framedropping_arg,
    &enforce_delay_accuracy_arg,
    &repeat_arg,
    &limit_interval_start_arg,
    &limit_interval_end_arg,
    &unlabled_meas_path_arg,
  };

  for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); ++arg_iterator)
  {
    cmd.add(*arg_iterator);
  }

  std::stringstream tclap_output_stream;
  CustomTclap::AdvancedTclapOutput advanced_tclap_output(std::vector<std::ostream*>{ &tclap_output_stream, &std::cout }, 75);
  advanced_tclap_output.setArgumentHidden(&unlabled_meas_path_arg, true);

  cmd.setOutput(&advanced_tclap_output);

  try
  {
    cmd.parse(argc, argv);
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
  }

  QApplication a(argc, argv);

  a.setOrganizationName("Continental");
  a.setOrganizationDomain("continental-corporation.com");
  a.setApplicationName("ecalplaygui");
  a.setApplicationDisplayName("eCAL Player");

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


  //////////////////////////////////////////////////////////////////////////////
  // Start eCAL Play
  //////////////////////////////////////////////////////////////////////////////

  // Just make sure that eCAL is initialized
  eCAL::Initialize(0, nullptr, "eCALPlayGUI", eCAL::Init::Default | eCAL::Init::ProcessReg | eCAL::Init::Publisher | eCAL::Init::Service | eCAL::Init::Monitoring);

  EcalplayGui* w = new EcalplayGui();
  w->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

  QEcalPlay::instance()->setParent(w);
  w->show();

  //////////////////////////////////////
  // --measurement
  //////////////////////////////////////
  std::string meas_path = "";

  if (measurement_path_arg.isSet())
  {
    meas_path = measurement_path_arg.getValue();
  }
  else if (unlabled_meas_path_arg.isSet())
  {
    meas_path = unlabled_meas_path_arg.getValue()[0];
  }

  if (!meas_path.empty())
  {
    QEcalPlay::instance()->loadMeasurement(QString::fromStdString(meas_path));
  }

  //////////////////////////////////////
  // --channel-mapping
  //////////////////////////////////////
  if (channel_rename_file_path_arg.isSet())
  {
    QEcalPlay::instance()->loadChannelMappingFile(QString::fromStdString(channel_rename_file_path_arg.getValue()));
  }

  //////////////////////////////////////
  // --unlimited-speed
  // --speed
  //////////////////////////////////////
  if (no_play_speed_limit_arg.isSet())
  {
    QEcalPlay::instance()->setLimitPlaySpeedEnabled(!no_play_speed_limit_arg.getValue());
  }

  if (play_speed_arg.isSet())
  {
    QEcalPlay::instance()->setPlaySpeed(play_speed_arg.getValue());
  }

  //////////////////////////////////////
  // --framedropping
  // --enforce-delay-accuracy
  // --repeat
  // --limit-interval-start
  // --limit-interval-end
  //////////////////////////////////////
  if (allow_framedropping_arg.isSet())
  {
    QEcalPlay::instance()->setFrameDroppingAllowed(allow_framedropping_arg.getValue());
  }

  if (enforce_delay_accuracy_arg.isSet())
  {
    QEcalPlay::instance()->setEnforceDelayAccuracyEnabled(enforce_delay_accuracy_arg.getValue());
  }

  if (repeat_arg.isSet())
  {
    QEcalPlay::instance()->setRepeatEnabled(repeat_arg.getValue());
  }

  if (limit_interval_start_arg.isSet() || limit_interval_end_arg.isSet())
  {
    if (QEcalPlay::instance()->isMeasurementLoaded())
    {
      auto limit_interval = QEcalPlay::instance()->measurementBoundaries();
      auto measurement_start = limit_interval.first;

      if (limit_interval_start_arg.isSet())
      {
        limit_interval.first = measurement_start + std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(limit_interval_start_arg.getValue()));
      }

      if (limit_interval_end_arg.isSet())
      {
        limit_interval.second = measurement_start + std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(limit_interval_end_arg.getValue()));
      }

      QEcalPlay::instance()->setLimitInterval(limit_interval);
    }
    else
    {
      EcalPlayLogger::Instance()->error("Unable to set limit interval: No measurement is loaded.");
    }
  }

  //////////////////////////////////////
  // --play
  //////////////////////////////////////
  if (play_arg.isSet())
  {
    if (QEcalPlay::instance()->isMeasurementLoaded())
    {
      QEcalPlay::instance()->play();
    }
    else
    {
      EcalPlayLogger::Instance()->error("Unable to start playback: No measurement is loaded.");
    }
  }

  int return_code = a.exec();

  eCAL::Finalize();

  return return_code;
}
