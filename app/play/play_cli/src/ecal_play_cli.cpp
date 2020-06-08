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

#include "ecal_play.h"

#include "ecal_play_service.h"

#include <ecal/msg/protobuf/server.h>

#include "tclap/CmdLine.h"
#include <vector>
#include <thread>
#include <cctype>
#include <clocale>

#include <iomanip>
#include <sstream>

#ifdef WIN32
#define NOMINMAX
#endif // WIN32

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 )
#endif //_MSC_VER
#include <termcolor/termcolor.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER


#ifdef ECAL_OS_WINDOWS
#include <conio.h>
#else // ECAL_OS_WINDOWS
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int _kbhit()
{
  static const int STDIN = 0;
  static bool initialized = false;

  if (!initialized)
  {
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = true;
  }

  int bytesWaiting;
  ioctl(STDIN, FIONREAD, &bytesWaiting);
  return bytesWaiting;
}

int _getch(void)
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}
#endif  // ECAL_OS_WINDOWS

std::string UsageString(const TCLAP::Arg& arg)
{
  return arg.flagStartChar() + arg.getFlag() + " / " + arg.nameStartString() + arg.getName();
}

// Keys for Settings
static char KEY_LOAD_MEAS              = 'l';
static char KEY_LIMIT_PLAY_SPEED       = 'o';
static char KEY_SET_PLAY_SPEED         = 'p';
static char KEY_ENFORCE_DELAY_ACCURACY = 'e';
static char KEY_REPEAT                 = 'r';
static char KEY_LIMIT_INTERVAL         = 'n';

// Keys for Playback
static char KEY_INITIALIZE             = 'i';
static char KEY_DE_INITIALIZE          = 'd';
static char KEY_JUMP_TO                = 'j';
static char KEY_PLAY_PAUSE             = ' ';
static char KEY_STEP                   = 's';
static char KEY_STEP_CHANNEL           = 'c';

static char KEY_MEAS_INFO              = 'm';

static char KEY_EXIT                   = 0x1B; // ESC

std::string keyCharToString(char key)
{
  if (key == ' ')
    return "Space";
  else if (key == 0x1B)
    return "Esc";
  else if ((key >= 'a') && (key <= 'z'))
    return std::string(1, (char)std::toupper(key));
  else
    return std::string(1, key);
}

void printInteractiveHelp()
{
  std::cout << std::endl;
  std::cout << termcolor::bold << "Keyboard mapping:" << termcolor::reset << std::endl;
  std::cout << std::endl;
  std::cout << "  Measurement Info:                  [" << keyCharToString(KEY_MEAS_INFO)              << "]" << std::endl;
  std::cout << std::endl;
  std::cout << "  Load measurement:                  [" << keyCharToString(KEY_LOAD_MEAS)              << "]" << std::endl;
  std::cout << "  Enable / disable play speed limit: [" << keyCharToString(KEY_LIMIT_PLAY_SPEED)       << "]" << std::endl;
  std::cout << "  Set play speed:                    [" << keyCharToString(KEY_SET_PLAY_SPEED)         << "]" << std::endl;
  std::cout << "  Enable / disable delay accuracy:   [" << keyCharToString(KEY_ENFORCE_DELAY_ACCURACY) << "]" << std::endl;
  std::cout << "  Enable / disable repeating:        [" << keyCharToString(KEY_REPEAT)                 << "]" << std::endl;
  std::cout << "  Set limit interval:                [" << keyCharToString(KEY_LIMIT_INTERVAL)         << "]" << std::endl;
  std::cout << std::endl;
  std::cout << "  Initialize:                        [" << keyCharToString(KEY_INITIALIZE)             << "]" << std::endl;
  std::cout << "  De-Initialize:                     [" << keyCharToString(KEY_DE_INITIALIZE)          << "]" << std::endl;
  std::cout << "  Jump to Position:                  [" << keyCharToString(KEY_JUMP_TO)                << "]" << std::endl;
  std::cout << "  Play / Pause:                      [" << keyCharToString(KEY_PLAY_PAUSE)             << "]" << std::endl;
  std::cout << "  Step 1 frame:                      [" << keyCharToString(KEY_STEP)                   << "]" << std::endl;
  std::cout << "  Step Channel:                      [" << keyCharToString(KEY_STEP_CHANNEL)           << "]" << std::endl;
  std::cout << "  Exit:                              [" << keyCharToString(KEY_EXIT)                   << "]" << std::endl;
}

void printMeasurementInformation(std::shared_ptr<EcalPlay> ecal_player)
{
  if (!ecal_player->IsMeasurementLoaded())
  {
    std::cout << "No measurement loaded" << std::endl;
    return;
  }

  // General Measurement information
  std::cout << termcolor::bold << "Measurement information:" << termcolor::reset << std::endl;
  std::cout << "  Directory:       " << ecal_player->GetMeasurementDirectory() << std::endl;
  std::cout << "  Channel count:   " << ecal_player->GetChannelNames().size() << std::endl;
  std::cout << "  Frame count:     " << ecal_player->GetFrameCount() << std::endl;
  auto measurement_boundaries = ecal_player->GetMeasurementBoundaries();
  std::cout << "  Timestamp range: ["
            << std::fixed
            << std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries.first.time_since_epoch()).count()
            << " s, "
            << std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries.second.time_since_epoch()).count()
            << " s]"
            << std::endl;
  std::cout << "  Length:          " << std::chrono::duration_cast<std::chrono::duration<double>>(ecal_player->GetMeasurementLength()).count() << " s" << std::endl;

  std::cout << std::endl;

  // Channel Information
  std::cout << termcolor::bold << "Channels:" << termcolor::reset << std::endl;

  auto channels        = ecal_player->GetChannelNames();
  auto channel_mapping = ecal_player->GetChannelMapping();

  // Determine the size of the largest item
  size_t left_size = 0;
  for (const auto& channel : channels)
  {
    left_size = std::max(left_size, channel.size());
  }
  left_size = std::min(left_size, size_t(50));

  // Print all channels
  for (const auto& channel : channels)
  {
    if (channel_mapping.find(channel) != channel_mapping.end())
    {
      std::cout << termcolor::green;
      std::cout << "  * " << std::left << std::setw(left_size) << channel << " -> " << channel_mapping[channel] << std::endl;
      std::cout << termcolor::reset;
    }
    else
    {
      std::cout << "    " << channel << std::endl;
    }
  }
}

int main(int argc, char *argv[])
{
  TCLAP::CmdLine cmd("eCAL Player", ' ', EcalPlayGlobals::VERSION_STRING);
  
  assert(cmd.getArgList().size() == 3);
  auto arg_it = cmd.getArgList().begin();

  //TCLAP::Arg& ignore_rest_arg = **arg_it;
  ++arg_it;
  //TCLAP::Arg& version_arg     = **arg_it;
  ++arg_it;
  TCLAP::Arg& help_arg        = **arg_it;



  TCLAP::ValueArg<std::string> measurement_path_arg      ("m", "measurement",            "Load and play the measurement from the path. If interactive mode is used, the measurement is just loaded. The playback has to be started by the user.",  false, "", "string");
  TCLAP::ValueArg<std::string> channel_rename_file_path  ("c", "channel-mapping",        "Loads the channel mapping from the given file and uses it to filter and intialize eCAL publishers. Otherwise, all publishers from the measurement will keep their default name.", false, "", "string");
  TCLAP::SwitchArg             no_play_speed_limit_arg   ("u", "unlimited-speed",        "Publish frames as fast as possible",                                                                                                                     false);
  TCLAP::ValueArg<double>      play_speed_arg            ("s", "speed",                  "Relative rate at which the player shall publish the messages. Ignored, when using \"unlimited_play_speed\"",                                             false, 1.0, "double");
  TCLAP::SwitchArg             allow_framedropping_arg   ("f", "framedropping",          "Drop frames when the messages cannot be sent at the required speed",                                                                                     false);
  TCLAP::SwitchArg             enforce_delay_accuracy_arg("d", "enforce-delay-accuracy", "Always wait the correct amount of time between two messages, even if this will slow down the playback",                                                  false);
  TCLAP::SwitchArg             repeat_arg                ("r", "repeat",                 "Repeat playback from the beginning if the end has been reached",                                                                                         false);
  TCLAP::ValueArg<double>      limit_interval_start_arg  ("l", "limit-interval-start",   "Start the playback from this time (relative value in seconds, 0.0 indicates the begin of the measurement)",                                              false, -1.0, "double");
  TCLAP::ValueArg<double>      limit_interval_end_arg    ("e", "limit-interval-end",     "End the playback at this time (relative value in seconds)",                                                                                              false, -1.0, "double");

  TCLAP::SwitchArg             interactive_arg           ("i", "interactive",            "Just start the Player and dont exit. The user can interactively use the player or control it with the eCAL Service API.",                                false);

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &measurement_path_arg,
    &channel_rename_file_path,
    &no_play_speed_limit_arg,
    &play_speed_arg,
    &allow_framedropping_arg,
    &enforce_delay_accuracy_arg,
    &repeat_arg,
    &limit_interval_start_arg,
    &limit_interval_end_arg,
    &interactive_arg,
  };
  
  for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); ++arg_iterator)
  {
    cmd.add(*arg_iterator);
  }
 
  try
  {
    cmd.parse(argc, argv);
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  //// Argument constraints                                                 ////
  //////////////////////////////////////////////////////////////////////////////
  if (!measurement_path_arg.isSet())
  {
    if (!interactive_arg.isSet())
    {
      std::cerr << "Nothing to do, exiting. Load a measurement with " << UsageString(measurement_path_arg)
        << " or use the interactive mode with " << UsageString(interactive_arg) << "." << std::endl
        << "Use " << UsageString(help_arg) << " to display a list of all available parameters."
        << std::endl << std::endl;
      return EXIT_FAILURE;
    }

    if (channel_rename_file_path.isSet())
    {
      std::cerr << UsageString(channel_rename_file_path) << " can only be used when a measurement is loaded. Use " << UsageString(measurement_path_arg) << " to load a measurement" << std::endl;
    }

    if (limit_interval_start_arg.isSet() || limit_interval_end_arg.isSet())
    {
      std::cerr << limit_interval_start_arg.getName() << " and " << limit_interval_end_arg.getName()
        << " can only be set when a measurement is loaded. Use " << UsageString(measurement_path_arg) << " to load a measurement" << std::endl;
      return EXIT_FAILURE;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  //// EcalPlay instance                                                    ////
  //////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<EcalPlay> ecal_player(new EcalPlay());

  //////////////////////////////////////////////////////////////////////////////
  //// Apply command line                                                   ////
  //////////////////////////////////////////////////////////////////////////////

  if (measurement_path_arg.isSet())
  {
    bool success = ecal_player->LoadMeasurement(measurement_path_arg.getValue());
    if (!success)
    {
      return EXIT_FAILURE;
    }
  }

  if (channel_rename_file_path.isSet())
  {
    auto channel_mapping = ecal_player->LoadChannelMappingFile(channel_rename_file_path.getValue());
    ecal_player->InitializePublishers(channel_mapping);
  }

  if (no_play_speed_limit_arg.isSet())
  {
    ecal_player->SetLimitPlaySpeedEnabled(!no_play_speed_limit_arg.getValue());
  }

  if (play_speed_arg.isSet())
  {
    ecal_player->SetPlaySpeed(play_speed_arg.getValue());
  }

  if (allow_framedropping_arg.isSet())
  {
    ecal_player->SetFrameDroppingAllowed(allow_framedropping_arg.getValue());
  }

  if (enforce_delay_accuracy_arg.isSet())
  {
    ecal_player->SetEnforceDelayAccuracyEnabled(enforce_delay_accuracy_arg.getValue());
  }

  if (repeat_arg.isSet())
  {
    ecal_player->SetRepeatEnabled(repeat_arg.getValue());
  }

  if (limit_interval_start_arg.isSet() || limit_interval_end_arg.isSet())
  {
    auto limit_interval = ecal_player->GetMeasurementBoundaries();
    auto measurement_start = limit_interval.first;

    if (limit_interval_start_arg.isSet())
    {
      limit_interval.first = measurement_start + std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(limit_interval_start_arg.getValue()));
    }

    if (limit_interval_end_arg.isSet())
    {
      limit_interval.second = measurement_start + std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(limit_interval_end_arg.getValue()));
    }

    bool success = ecal_player->SetLimitInterval(limit_interval);
    if (!success)
    {
      return EXIT_FAILURE;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Start Playback or interactive mode                                   ////
  //////////////////////////////////////////////////////////////////////////////
  if (interactive_arg.isSet())
  {
    std::cout << "Running interactive mode. Press " << keyCharToString(KEY_EXIT) << " to exit." << std::endl;
    printInteractiveHelp();

    std::shared_ptr<eCAL::pb::play::EcalPlayService> play_service_ (new EcalPlayService(ecal_player));
    eCAL::protobuf::CServiceServer<eCAL::pb::play::EcalPlayService> play_service_server_(play_service_);

    std::string step_reference_channel;

    for(;;)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
      if ((static_cast<EcalPlayService*>(play_service_.get()))->IsExitRequested())
      {
        break;
      }
      // Keyboard Control
      else if (_kbhit() > 0)
      {
        auto pressed_char = _getch();
        
        // The Ubuntu Bash echoes the input, so we lazy overwrite it.
        std::cout << "\r";

        // Load measurement
        if (pressed_char == KEY_LOAD_MEAS)
        {
          std::string meas_path;
          std::cout << "Measurement path: ";
          std::getline(std::cin, meas_path);    // use std::getline to allow the imput being empty
          if (!meas_path.empty())
          {
            ecal_player->LoadMeasurement(meas_path);
          }
        }
        // Limit play speed
        else if (pressed_char == KEY_LIMIT_PLAY_SPEED)
        {
          ecal_player->SetLimitPlaySpeedEnabled(!ecal_player->IsLimitPlaySpeedEnabled());
        }
        // Set play speed
        else if (pressed_char == KEY_SET_PLAY_SPEED)
        {

          double play_speed = 1.0;
          bool success = false;

          while (!success)
          {
            std::cout << "Play speed: ";
            std::string play_speed_string;
            std::getline(std::cin, play_speed_string);

            char decimal_point = std::localeconv()->decimal_point[0];
            std::replace(play_speed_string.begin(), play_speed_string.end(), '.', decimal_point);

            // Parse the string
            try
            {
              play_speed = std::stod(play_speed_string);
              success = true;
            }
            catch (const std::exception& e)
            {
              std::cout << "Invalid input (" << e.what() << "). Try again." << std::endl;
            }
          }

          ecal_player->SetPlaySpeed(play_speed);
        }
        // Enforce delay accuracy
        else if (pressed_char == KEY_ENFORCE_DELAY_ACCURACY)
        {
          ecal_player->SetEnforceDelayAccuracyEnabled(!ecal_player->IsEnforceDelayAccuracyEnabled());
        }
        // Repeat
        else if (pressed_char == KEY_REPEAT)
        {
          ecal_player->SetRepeatEnabled(!ecal_player->IsRepeatEnabled());
        }
        // Set limit interval
        else if (pressed_char == KEY_LIMIT_INTERVAL)
        {
          if (!ecal_player->IsMeasurementLoaded())
          {
            std::cout << "No measurement loaded. Press [" << keyCharToString(KEY_LOAD_MEAS) << "] to load a measurement." << std::endl;
            continue;
          }

          char decimal_point = std::localeconv()->decimal_point[0];

          bool success = false;

          auto measurement_boundaries = ecal_player->GetMeasurementBoundaries();
          auto measurement_length     = std::chrono::duration_cast<std::chrono::duration<double>>(ecal_player->GetMeasurementLength());
          auto limit_interval         = measurement_boundaries;

          // Limit interval start
          while (!success)
          {
            std::cout << "Limit interval start: [0.0 secs] ";
            std::string limit_interval_start_rel_secs_string;
            std::getline(std::cin, limit_interval_start_rel_secs_string);             // use std::getline to allow the imput being empty
            std::replace(limit_interval_start_rel_secs_string.begin(), limit_interval_start_rel_secs_string.end(), '.', decimal_point);

            if (limit_interval_start_rel_secs_string.empty())
            {
              // Don't change the limit inteval, leave it at the outer measurement boundaries
              success = true;
            }
            else
            {
              // Parse the string
              try
              {
                double limit_interval_start_rel_secs = std::stod(limit_interval_start_rel_secs_string);
                limit_interval.first = measurement_boundaries.first + std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(limit_interval_start_rel_secs));
                success = true;
              }
              catch (const std::exception& e)
              {
                std::cout << "Invalid input: " << e.what() << " Try again." << std::endl;
              }
            }
          }

          success = false;

          // Limit interval end
          while (!success)
          {
            std::cout << "Limit interval end: [" << std::fixed << std::setprecision(3) << measurement_length.count() << " secs] ";
            std::string limit_interval_end_rel_secs_string;
            std::getline(std::cin, limit_interval_end_rel_secs_string);             // use std::getline to allow the imput being empty
            std::replace(limit_interval_end_rel_secs_string.begin(), limit_interval_end_rel_secs_string.end(), '.', decimal_point);

            if (limit_interval_end_rel_secs_string.empty())
            {
              // Don't change the limit inteval, leave it at the outer measurement boundaries
              success = true;
            }
            else
            {
              // Parse the string
              try
              {
                double limit_interval_end_rel_secs = std::stod(limit_interval_end_rel_secs_string);
                limit_interval.second = measurement_boundaries.first + std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(limit_interval_end_rel_secs));
                success = true;
              }
              catch (const std::exception& e)
              {
                std::cout << "Invalid input: " << e.what() << " Try again." << std::endl;
              }
            }
          }
          
          ecal_player->SetLimitInterval(limit_interval);
        }
        // Measurement Info
        else if (pressed_char == KEY_MEAS_INFO)
        {
          printMeasurementInformation(ecal_player);
        }
        // Initialize
        else if (pressed_char == KEY_INITIALIZE)
        {
          if (!ecal_player->IsMeasurementLoaded())
          {
            std::cout << "No measurement loaded. Press [" << keyCharToString(KEY_LOAD_MEAS) << "] to load a measurement." << std::endl;
            continue;
          }

          std::cout << "Channel mapping file (empty for initializing everything): ";
          std::string channel_mapping_file_path;
          std::getline(std::cin, channel_mapping_file_path);    // use std::getline to allow the imput being empty
          if (channel_mapping_file_path.empty())
          {
            ecal_player->InitializePublishers();
          }
          else
          {
            auto channel_map = ecal_player->LoadChannelMappingFile(channel_mapping_file_path);
            if (channel_map.size() != 0)
            {
              ecal_player->InitializePublishers(channel_map);
            }
            else
            {
              std::cout << "Channel mapping file invalid" << std::endl;
            }
          }
        }
        // De-Initialize
        else if (pressed_char == KEY_DE_INITIALIZE)
        {
          if (!ecal_player->IsMeasurementLoaded())
          {
            std::cout << "No measurement loaded. Press [" << keyCharToString(KEY_LOAD_MEAS) << "] to load a measurement." << std::endl;
            continue;
          }

          ecal_player->DeInitializePublishers();
        }
        // Jump to
        else if (pressed_char == KEY_JUMP_TO)
        {
          if (!ecal_player->IsMeasurementLoaded())
          {
            std::cout << "No measurement loaded. Press [" << keyCharToString(KEY_LOAD_MEAS) << "] to load a measurement." << std::endl;
            continue;
          }

          double rel_secs;
          bool success = false;
          while (!success)
          {
            std::cout << "Jump to (rel. seconds since start): ";

            std::string time_string;
            std::getline(std::cin, time_string);

            char decimal_point = std::localeconv()->decimal_point[0];
            std::replace(time_string.begin(), time_string.end(), '.', decimal_point);

            // Parse the string
            try
            {
              rel_secs = std::stod(time_string);
              success = true;
            }
            catch (const std::exception& e)
            {
              std::cout << "Invalid input (" << e.what() << "). Try again." << std::endl;
            }
          }

          auto rel_time = std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(rel_secs));
          auto measurement_boundaries = ecal_player->GetMeasurementBoundaries();
          ecal_player->JumpTo(measurement_boundaries.first + rel_time);
        }
        // Play / Pause
        else if (pressed_char == KEY_PLAY_PAUSE)
        {
          if (!ecal_player->IsMeasurementLoaded())
          {
            std::cout << "No measurement loaded. Press [" << keyCharToString(KEY_LOAD_MEAS) << "] to load a measurement." << std::endl;
            continue;
          }

          if (ecal_player->IsPlaying())
          {
            ecal_player->Pause();
          }
          else
          {
            ecal_player->Play();
          }
        }
        // Step
        if (pressed_char == KEY_STEP)
        {
          if (!ecal_player->IsMeasurementLoaded())
          {
            std::cout << "No measurement loaded. Press [" << keyCharToString(KEY_LOAD_MEAS) << "] to load a measurement." << std::endl;
            continue;
          }

          ecal_player->StepForward();
        }
        // Step channel
        if (pressed_char == KEY_STEP_CHANNEL)
        {
          if (!ecal_player->IsMeasurementLoaded())
          {
            std::cout << "No measurement loaded. Press [" << keyCharToString(KEY_LOAD_MEAS) << "] to load a measurement." << std::endl;
            continue;
          }

          std::cout << "Step reference channel: ";
          if (!step_reference_channel.empty())
          {
            std::cout << "[" << step_reference_channel << "] ";
          }
          std::string step_reference_channel_temp;
          std::getline(std::cin, step_reference_channel_temp);    // use std::getline to allow the imput being empty
          if (!step_reference_channel_temp.empty())
          {
            step_reference_channel = step_reference_channel_temp;
          }
          ecal_player->PlayToNextOccurenceOfChannel(step_reference_channel);
        }
        // Exit
        else if (pressed_char == KEY_EXIT)
        {
          break;
        }
      }
    }
  }
  else if(ecal_player->IsMeasurementLoaded())
  {
    std::cout << "Starting measurement playback. Press " << keyCharToString(KEY_EXIT) << " to exit." << std::endl;
    ecal_player->Play();
    while (ecal_player->IsPlaying())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (_kbhit() > 0 && _getch() == KEY_EXIT)
      {
        break;
      }
    }
  }

  return 0;
}
