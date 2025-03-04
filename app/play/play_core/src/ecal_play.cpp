/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <chrono>
#include <clocale>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "ecal_play_logger.h"
#include "play_thread.h"
#include <ecalhdf5/eh5_meas.h>
#include <ecal_utils/string.h>
#include <ecal_utils/filesystem.h>
#include <ecal_utils/str_convert.h>

EcalPlay::EcalPlay()
{
  LogAppNameVersion();

  play_thread_ = std::make_unique<PlayThread>();
  play_thread_->Start();

  eCAL::Process::SetState(eCAL::Process::eSeverity::warning, eCAL::Process::eSeverityLevel::level1, "No measurement loaded");
}

EcalPlay::~EcalPlay()
{
  play_thread_->Interrupt();
  play_thread_->Join();
}

////////////////////////////////////////////////////////////////////////////////
//// Measurement loading                                                    ////
////////////////////////////////////////////////////////////////////////////////
bool EcalPlay::LoadMeasurement(const std::string& path)
{
  EcalPlayLogger::Instance()->info("Loading measurement...");

  std::shared_ptr<eCAL::eh5::v2::HDF5Meas> measurement(std::make_shared<eCAL::eh5::v2::HDF5Meas>());

  std::string meas_dir;               // The directory of the measurement
  std::string path_to_load;           // The actual path we load the measurement from. May be a directory or a .hdf5 file
  bool        is_hdf5_file = false;   // Whether the given path pointed to an hdf5 file
  
  // Check if the user opened a file or a directory
  auto file_status = EcalUtils::Filesystem::FileStatus(path, EcalUtils::Filesystem::OsStyle::Current);
  if (!file_status.IsOk())
  {
    EcalPlayLogger::Instance()->error("Failed loading measurement from \"" + path + "\": Resource unavailable.");
    return false;
  }
  else
  {
    if (file_status.GetType() == EcalUtils::Filesystem::Type::RegularFile)
    {
      // The user opened a file! Let's check if it is an HDF5 file or an .ecalmeas file!
      std::string filename = EcalUtils::Filesystem::FileName(path, EcalUtils::Filesystem::OsStyle::Current);
      size_t dot_pos = filename.find_last_of('.');
      if (dot_pos != std::string::npos)
      {
        std::string file_extension = filename.substr(dot_pos);
        std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(),
                        [](char c) -> char
                        {
                          return static_cast<char>(std::tolower(static_cast<int>(c)));
                        });
        if (file_extension == ".hdf5")
        {
          is_hdf5_file = true;
        }
      }

      // Although the path points to a file, our filesystem API will strip out the last componentent (i.e. the filename) when appending a "/.."
      meas_dir = EcalUtils::Filesystem::CleanPath(path + "/..", EcalUtils::Filesystem::OsStyle::Current);
    }
    else
    {
      meas_dir = path;
    }
  }

  path_to_load = (is_hdf5_file ? path : meas_dir);

  // Load the measurement
  if (measurement->Open(path_to_load) && measurement->IsOk())
  {
    EcalPlayLogger::Instance()->info("Measurement dir:  " + meas_dir);
    play_thread_->SetMeasurement(measurement, meas_dir);
    measurement_path_ = path;

    LoadDescription(meas_dir + "/doc/description.txt");
    LoadScenarios(meas_dir + "/doc", meas_dir + "/doc/scenario.txt");

    LogMeasurementSummary();
    eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "Measurement loaded");

    return true;
  }
  else
  {
    EcalPlayLogger::Instance()->error("Failed loading measurement from: " + meas_dir);
    eCAL::Process::SetState(eCAL::Process::eSeverity::warning, eCAL::Process::eSeverityLevel::level1, "Failed loading measurement");
    return false;
  }
}

void EcalPlay::CloseMeasurement()
{
  description_ = "";
  play_thread_->SetMeasurement(std::shared_ptr<eCAL::eh5::v2::HDF5Meas>(nullptr));
  measurement_path_ = "";
  clearScenariosPath();
  channel_mapping_path_ = "";
}

bool EcalPlay::IsMeasurementLoaded() const
{
  return play_thread_->IsMeasurementLoaded();
}

std::string EcalPlay::GetMeasurementPath() const
{
  return measurement_path_;
}

std::string EcalPlay::GetMeasurementDirectory() const
{
  return play_thread_->GetMeasurementPath();
}

std::string EcalPlay::GetScenariosDirectory() const
{
  return scenarios_path_.first;
}

std::string EcalPlay::GetScenariosPath() const
{
  return scenarios_path_.second;
}

std::string EcalPlay::GetChannelMappingPath() const
{
  return channel_mapping_path_;
}

std::map<std::string, std::string> EcalPlay::LoadChannelMappingFile(const std::string& path)
{
  std::map<std::string, std::string> channel_mapping;

#ifdef WIN32
  std::wstring w_path = EcalUtils::StrConvert::Utf8ToWide(path);
  std::ifstream mapping_file(w_path);
#else
  std::ifstream mapping_file(path);
#endif // WIN32

  if (mapping_file.is_open() == true)
  {
    std::string line;
    while (std::getline(mapping_file, line))
    {
      std::vector<std::string> mapping;
      EcalUtils::String::Split(line, "\t", mapping);

      if (mapping.size() == 1)
      {
        channel_mapping[EcalUtils::String::Trim(mapping[0])] = EcalUtils::String::Trim(mapping[0]);
      }
      else if (mapping.size() == 2)
      {
        channel_mapping[EcalUtils::String::Trim(mapping[0])] = EcalUtils::String::Trim(mapping[1]);
      }
    }

    channel_mapping_path_ = path;
  }
  else
  {
    EcalPlayLogger::Instance()->warn("Unable to load channel mapping from file: " + path);
  }

  return channel_mapping;
}

std::set<std::string> EcalPlay::GetChannelNames() const
{
  return play_thread_->GetChannelNames();
}

double EcalPlay::GetMinTimestampOfChannel(const std::string& channel_name) const
{
  return play_thread_->GetMinTimestampOfChannel(channel_name);
}

double EcalPlay::GetMaxTimestampOfChannel(const std::string& channel_name) const
{
  return play_thread_->GetMaxTimestampOfChannel(channel_name);
}

std::string EcalPlay::GetChannelType(const std::string& channel_name) const
{
  return play_thread_->GetChannelType(channel_name);
}

std::string EcalPlay::GetChannelEncoding(const std::string& channel_name) const
{
  return play_thread_->GetChannelEncoding(channel_name);
}

void EcalPlay::CalculateEstimatedSizeForChannels() const
{
  play_thread_->CalculateEstimatedSizeForChannels();
}

size_t EcalPlay::GetChannelCumulativeEstimatedSize(const std::string& channel_name) const
{
  return play_thread_->GetChannelCumulativeEstimatedSize(channel_name);
}

std::map<std::string, ContinuityReport> EcalPlay::CreateContinuityReport() const
{
  return play_thread_->CreateContinuityReport();
}

std::map<std::string, long long> EcalPlay::GetMessageCounters() const
{
  return play_thread_->GetMessageCounters();
}

bool EcalPlay::LoadDescription(const std::string& path)
{
  std::stringstream ss;
  std::string line;

#ifdef WIN32
  std::wstring w_path = EcalUtils::StrConvert::Utf8ToWide(path);
  std::ifstream description_file(w_path);
#else
  std::ifstream description_file(path);
#endif // WIN32

  if (description_file.is_open())
  {
    while (getline(description_file, line))
    {
      ss << line << std::endl;
    }
    description_file.close();
    EcalPlayLogger::Instance()->info("Description file: " + path);
  }
  else
  {
    description_ = "";
    EcalPlayLogger::Instance()->warn("Unable to load description file: " + path);
    return false;
  }

  description_ = ss.str();
  return true;
}

void EcalPlay::clearScenariosPath()
{
  scenarios_path_.first.clear();
  scenarios_path_.second.clear();
}

std::string EcalPlay::GetDescription() const
{
  return description_;
}

bool EcalPlay::LoadScenarios(const std::string& selected_dir, const std::string& selected_file)
{
  // Remove old scenarios
  scenarios_.clear();
  clearScenariosPath();

  std::string scenario_line;

#ifdef WIN32
  std::wstring w_path = EcalUtils::StrConvert::Utf8ToWide(selected_file);
  std::ifstream scenario_file(w_path);
#else
  std::ifstream scenario_file(selected_file);
#endif // WIN32

  if (scenario_file.is_open())
  {
    // Load the Scenario file
    while (getline(scenario_file, scenario_line))
    {
      scenario_line = EcalUtils::String::Trim(scenario_line);

      // Split the string representation of one scenario (they are separated by a ;)
      std::vector<std::string> parts;
      EcalUtils::String::Split(scenario_line, ";", parts);

      if (parts.size() == 2)
      {
        std::string time_string = EcalUtils::String::Trim(parts[0]);
        double time_double;

        // Safely convert the string into a double
        try
        {
          char decimal_point = std::localeconv()->decimal_point[0];
          std::replace(time_string.begin(), time_string.end(), '.', decimal_point);
          time_double = std::stod(time_string);
        }
        catch (const std::exception&)
        {
          continue;
        }

        // Create one scenario and fill it with data
        EcalPlayScenario scenario;
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(time_double));
        scenario.time_ = GetMeasurementBoundaries().first + time;
        scenario.name_ = EcalUtils::String::Trim(parts[1]);

        // Add the scenario to the scenario list
        scenarios_.push_back(scenario);
      }
    }
    scenario_file.close();

    EcalPlayLogger::Instance()->info("Scenario file:    " + selected_file);
    scenarios_path_ = std::make_pair(selected_dir, selected_file);
    // Sort the scenario list by timestamp
    std::sort(scenarios_.begin(), scenarios_.end(), [](const EcalPlayScenario& s1, const EcalPlayScenario& s2) {return s1.time_ < s2.time_; });
    return true;
  }
  else
  {
    EcalPlayLogger::Instance()->warn("Unable to load scenario file: " + selected_file);
    return false;
  }
}

std::vector<EcalPlayScenario> EcalPlay::GetScenarios() const
{
  return scenarios_;
}



////////////////////////////////////////////////////////////////////////////////
//// Settings                                                               ////
////////////////////////////////////////////////////////////////////////////////

void EcalPlay::SetScenarios(const std::vector<EcalPlayScenario>& scenarios)
{
  scenarios_ = scenarios;
}

bool EcalPlay::SaveScenariosToDisk(const std::string& selected_dir, const std::string& selected_file)
{
  if (!IsMeasurementLoaded()) 
  {
    EcalPlayLogger::Instance()->error("Unable to save scenario.txt: No measurement is loaded");
    return false;
  }
  else
  {
    std::string dir_path = selected_dir;
    std::string scenario_path = selected_file;
    if (dir_path.empty() || scenario_path.empty())
    {
      if (scenarios_path_.first.empty() || scenarios_path_.second.empty())
      {
        dir_path      = GetMeasurementDirectory() + "/doc/";
        scenario_path = dir_path + "scenario.txt";
      }
      else
      {
        dir_path = scenarios_path_.first;
        scenario_path = scenarios_path_.second;
      }
    }

    auto doc_dir_status = EcalUtils::Filesystem::FileStatus(dir_path, EcalUtils::Filesystem::Current);
    if (doc_dir_status.IsOk())
    {
      if (doc_dir_status.GetType() != EcalUtils::Filesystem::Type::Dir)
      {
        EcalPlayLogger::Instance()->error("Unable to save scenario.txt: \"" + dir_path + "\" is not a directory");
        return false;
      }
    }
    else
    {
      if (!EcalUtils::Filesystem::MkPath(dir_path, EcalUtils::Filesystem::Current))
      {
        EcalPlayLogger::Instance()->error("Unable to save scenario.txt: Failed creating directory \"" + dir_path + "\"");
        return false;
      }
    }

    std::ofstream scenario_file_stream;

#ifdef WIN32
    std::wstring w_scenario_path = EcalUtils::StrConvert::Utf8ToWide(scenario_path);
    scenario_file_stream.open (w_scenario_path);
#else
    scenario_file_stream.open (scenario_path);
#endif // WIN32

    if (!scenario_file_stream.is_open())
    {
      EcalPlayLogger::Instance()->error("Unable to save scenario.txt: Failed opening file \"" + scenario_path + "\"");
      return false;
    }

    auto sorted_scenarios = scenarios_;
    std::sort(sorted_scenarios.begin(), sorted_scenarios.end(), [](const EcalPlayScenario& a, const EcalPlayScenario& b) { return a.time_ < b.time_; });

    for (const EcalPlayScenario& scenario : sorted_scenarios)
    {
      auto time_since_start = scenario.time_ - GetMeasurementBoundaries().first;
      scenario_file_stream << std::chrono::duration_cast<std::chrono::duration<double>>(time_since_start).count();
      scenario_file_stream << "; ";

      scenario_file_stream << EcalUtils::String::Replace(scenario.name_, std::string(";"), std::string("_")); // Replace ';' by '_', as the semicolon is used as separator
      scenario_file_stream << std::endl;
    }
    scenario_file_stream.close();
    scenarios_path_ = std::make_pair(dir_path, scenario_path);

    return true;
  }
}

void EcalPlay::SetChannelMappingPath(const std::string& channel_mapping_path)
{
  channel_mapping_path_ = channel_mapping_path;
}

void EcalPlay::SetRepeatEnabled(bool enabled) const
{
  play_thread_->SetRepeatEnabled(enabled);
}

void EcalPlay::SetPlaySpeed(double speed) const
{
  play_thread_->SetPlaySpeed(speed);
}

void EcalPlay::SetLimitPlaySpeedEnabled(bool enabled) const
{
  play_thread_->SetLimitPlaySpeedEnabled(enabled);
}

void EcalPlay::SetFrameDroppingAllowed(bool allowed) const
{
  play_thread_->SetFrameDroppingAllowed(allowed);
}

void EcalPlay::SetEnforceDelayAccuracyEnabled(bool enabled) const
{
  play_thread_->SetEnforceDelayAccuracyEnabled(enabled);
}

bool EcalPlay::SetLimitInterval(const std::pair<long long, long long>& limit_interval) const
{
  return play_thread_->SetLimitInterval(limit_interval);
}

bool EcalPlay::SetLimitInterval(const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& limit_interval) const
{
  return play_thread_->SetLimitInterval(limit_interval);
}

bool EcalPlay::IsRepeatEnabled() const
{
  return play_thread_->IsRepeatEnabled();
}

double EcalPlay::GetPlaySpeed() const
{
  return play_thread_->GetPlaySpeed();
}

bool EcalPlay::IsLimitPlaySpeedEnabled() const
{
  return play_thread_->IsLimitPlaySpeedEnabled();
}

bool EcalPlay::IsFrameDroppingAllowed() const
{
  return play_thread_->IsFrameDroppingAllowed();
}

bool EcalPlay::IsEnforceDelayAccuracyEnabled() const
{
  return play_thread_->IsEnforceDelayAccuracyEnabled();
}

std::pair<long long, long long> EcalPlay::GetLimitInterval() const
{
  return play_thread_->GetLimitInterval();
}

////////////////////////////////////////////////////////////////////////////////
//// Playback                                                               ////
////////////////////////////////////////////////////////////////////////////////

bool EcalPlay::InitializePublishers() const
{
  return play_thread_->InitializePublishers();
}

bool EcalPlay::InitializePublishers(const std::map<std::string, std::string>& channel_mapping) const
{
  return play_thread_->InitializePublishers(channel_mapping);
}

bool EcalPlay::DeInitializePublishers() const
{
  return play_thread_->DeInitializePublishers();
}

bool EcalPlay::IsInitialized() const
{
  return play_thread_->PubishersInitialized();
}

std::map<std::string, std::string> EcalPlay::GetChannelMapping() const
{
  return play_thread_->GetChannelMapping();
}


bool EcalPlay::Play(long long play_until_index) const
{
  return play_thread_->Play(play_until_index);
}

bool EcalPlay::Pause() const
{
  return play_thread_->Pause();
}

bool EcalPlay::StepForward() const
{
  return play_thread_->StepForward();
}

bool EcalPlay::PlayToNextOccurenceOfChannel(const std::string& source_channel_name) const
{
  return play_thread_->PlayToNextOccurenceOfChannel(source_channel_name);
}

double EcalPlay::GetCurrentPlaySpeed() const
{
  return play_thread_->GetCurrentPlaySpeed();
}

bool EcalPlay::JumpTo(long long index) const
{
  return play_thread_->JumpTo(index);
}

bool EcalPlay::JumpTo(eCAL::Time::ecal_clock::time_point timestamp) const
{
  return play_thread_->JumpTo(timestamp);
}


////////////////////////////////////////////////////////////////////////////////
//// State                                                                  ////
////////////////////////////////////////////////////////////////////////////////

EcalPlayState EcalPlay::GetCurrentPlayState() const
{
  return play_thread_->GetCurrentPlayState();

}

bool EcalPlay::IsPlaying() const
{
  return play_thread_->IsPlaying();
}

bool EcalPlay::IsPaused() const
{
  return play_thread_->IsPaused();
}

std::chrono::nanoseconds EcalPlay::GetMeasurementLength() const
{
  return play_thread_->GetMeasurementLength();
}

std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> EcalPlay::GetMeasurementBoundaries() const
{
  return play_thread_->GetMeasurementBoundaries();
}

long long EcalPlay::GetFrameCount() const
{
  return play_thread_->GetFrameCount();
}

eCAL::Time::ecal_clock::time_point EcalPlay::GetTimestampOf(long long frame_index)  const
{
  return play_thread_->GetTimestampOf(frame_index);
}

eCAL::Time::ecal_clock::time_point EcalPlay::GetCurrentFrameTimestamp() const
{
  return play_thread_->GetCurrentFrameTimestamp();
}

long long EcalPlay::GetCurrentFrameIndex() const
{
  return play_thread_->GetCurrentFrameIndex();
}

void EcalPlay::LogAppNameVersion() const
{
  std::string app_version_header = " " + std::string(EcalPlayGlobals::ECAL_PLAY_NAME) + " " + std::string(EcalPlayGlobals::VERSION_STRING) + " ";
  std::string ecal_version_header = " (eCAL Lib " + std::string(ECAL_VERSION) + ") ";

  EcalUtils::String::CenterString(app_version_header, '-', 79);
  EcalUtils::String::CenterString(ecal_version_header, ' ', 79);

  EcalPlayLogger::Instance()->info("");
  EcalPlayLogger::Instance()->info(app_version_header);
  EcalPlayLogger::Instance()->info(ecal_version_header);
  EcalPlayLogger::Instance()->info("");
}

void EcalPlay::LogMeasurementSummary() const
{
  std::stringstream ss;

  ss << "Measurement information:" << std::endl;
  ss << "  Directory:       " << GetMeasurementDirectory() << std::endl;
  ss << "  Channel count:   " << GetChannelNames().size() << std::endl;
  ss << "  Frame count:     " << GetFrameCount() << std::endl;
  auto measurement_boundaries = GetMeasurementBoundaries();
  ss << "  Timestamp range: ["
    << std::fixed
    << std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries.first.time_since_epoch()).count()
    << " s, "
    << std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries.second.time_since_epoch()).count()
    << " s]"
    << std::endl;
  ss << "  Length:          " << std::chrono::duration_cast<std::chrono::duration<double>>(GetMeasurementLength()).count() << " s" << std::endl;
  
  EcalPlayLogger::Instance()->info(ss.str());
}
