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

#pragma once

#include <string>
#include <map>
#include <set>
#include <memory>
#include <chrono>
#include <ecal/ecal.h>

#include "continuity_report.h"
#include "ecal_play_state.h"
#include "ecal_play_scenario.h"
#include "ecal_play_globals.h"

// Forward declarations
class PlayThread;

/**
 * @brief The EcalPlay class contains the main eCAL Player API
 *
 * The eCAL Player can load measuremensts and play and pause the playback. It
 * provides several playback-related options, e.g. to repeat the playback from
 * the beginning when it has finished or to rename channels from the
 * measurement.
 *
 * The EcalPlay API aims to be as easily usable as possible. In order to load a
 * measurement and start a playback, the following code is sufficient:
 *
 *  #include <EcalPlay.h>
 *  #include <thread>        // only required for sleeping
 *
 *  int main()
 *  {
 *    // Create eCAl Player instance
 *    EcalPlay ecal_play;
 *
 *    // Load the measurement
 *    ecal_play.LoadMeasurement("C:\\meas01");
 *
 *    // Start the playback
 *    ecal_play.Play();
 *
 *    // Loop forever, as otherwise the program would exit
 *    for (;;) std::this_thread::sleep_for(std::chrono::milliseconds(100));
 *  }
 *
 * This code will create an instance of the eCAL Player, load a mesaurement
 * from C:\meas01 and start a playback with default options.
 *
 * When instanciated, EcalPlay will publish two eCAL Channels:
 *    __ecalplay_state__: Contains information about the current eCAL Player
 *                        instance, e.g. the path of the loaded measurement or
 *                        the measurement speed
 *    __sim_time__:       Contains information about the current simulation
 *                        time. This topic is most relevant for the
 *                        ecaltime-simtime plugin, that provides an eCAL::Time
 *                        implementation that will be kept synchronously with
 *                        the eCAL Player.
 *
 * Note, that is therefore discouraged to create multiple instances of EcalPlay.
 * Although possible, this class should be treated as Singleton. If multiple
 * instances exist, external applictations will not be able to read a consistent
 * __sim_time__ or __ecalplay_state__ anymore.
 */
class EcalPlay
{
public:

  //////////////////////////////////////////////////////////////////////////////
  //// Constructor / Destructor                                             ////
  //////////////////////////////////////////////////////////////////////////////
  EcalPlay();
  ~EcalPlay();

  EcalPlay(const EcalPlay&)            = delete;
  EcalPlay& operator=(const EcalPlay&) = delete;

  //////////////////////////////////////////////////////////////////////////////
  //// Measurement                                                          ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Loads a new measurement from the given path
   *
   * If there already was a measurement loaded, the old measurement is replaced.
   * After loading the measurement, the __ecalplay_state__ topic is published
   * immediatelly to inform all subscribers about the changes.
   *
   * Loading a measurement will also:
   *   - Pause the playback (if running)
   *   - Reset the limit interval
   *   - Set the current position to the first frame
   *   - Set the simulation time to the first measurement timestamp
   *   - Remove all old eCAL publishers (i.e. the player will have to be initialized again)
   *
   * When a measurement is loaded, the __sim_time__ will be marked as PAUSED
   * (or PLAYING after a playback has started)
   *
   * If loading the given measurement fails, nothing is changed, i.e. the old
   * measurement remains loaded and all settings stay the same.
   *
   * @param path A Path to load the measurement from
   * @return @code{true}, if loading the measurement was successfull
   */
  bool LoadMeasurement(const std::string& path);

  /**
   * @brief Closes the measurement without loading a new one
   *
   * If a measurement was loaded, this measurement will be un-loaded. If no
   * measurement was loaded, nothing will happen.
   *
   * Closing a measurement will also:
   *   - Pause the playback (if running)
   *   - Reset the limit interval to zero
   *   - Set the current position to zero
   *   - Set the simulation time to zero
   *   - Remove all old eCAL publishers
   *
   * When no measurement is loaded, the __sim_time__ will be marked as STOPPED.
   */
  void CloseMeasurement();

  /**
   * @brief Returns whether a measurement has successfully been loaded
   * @see LoadMeasurement
   * @see CloseMeasurement
   *
   * @return @code{true}, if a measurement is currently loaded.
   */
  bool IsMeasurementLoaded() const;

  /**
   * @brief Returns the path of the loaded measurement
   *
   * The path may point to a file or a directory; it depends on whether the user
   * loaded a measurement file (.ecalmeas) or a measurement directory. Use
   * @see{GetMeasurementDirectory()} to always obtain the measurement directory.
   *
   * @return the path of the current measurement (or an empty string, if no measurement has been loaded)
   */
  std::string GetMeasurementPath() const;

  /**
   * @brief Returns the directory of the loaded measurement
   *
   * The returned path always point to the loaded measurement directory and does
   * not depend on whether the user loaded the measurement from a file or a
   * directory.
   *
   * @return the directory of the current measurement (or an empty string, if no measurement has been loaded)
   */
  std::string GetMeasurementDirectory() const;

  /**
   * @brief Loads the given file as channel mapping file
   *
   * A channel mapping file contains source- and target channel names separated
   * by a tabulator "\t".
   * If the file cannot be loaded, an empty map is returned.
   *
   * Note that the mapping will not be applied automatically! Use
   * @see{InitializePublishers()} to apply the mapping.
   *
   * @param path    The path to load the channel mapping from
   *
   * @return the channel mapping from the given file
   */
  std::map<std::string, std::string> LoadChannelMappingFile(const std::string& path) const;

  /**
   * @brief Returns the description of the measurement found in the @code{doc/description.txt} file
   *
   * If no description is available (e.g. because the file is missing), or no
   * measurement has been loaded, an empty string is returned
   *
   * @return The description (or an empty string, if no description is available)
   */
  std::string GetDescription() const;

  /**
   * @brief Returns a list of scenarios found in the @code{doc/scenario.txt} file
   *
   * A Scenario is a named time point. The list is sorted by time.
   *
   * @return The list of scenarios
   */
  std::vector<EcalPlayScenario> GetScenarios() const;

  /**
   * @brief Returns a list of all channels in the measurment
   *
   * The channel set contains source-channel-names as they are stored in the
   * measurement file; i.e the channel mapping does not affect this list.
   * If no measurement is loaded, an empty set is returned.
   *
   * @return A set of all channels from the measurement
   */
  std::set<std::string> GetChannelNames() const;

  /**
   * @brief Creates a continuity report for all channels in the measurment
   *
   * For each channel in the measurement, the Continutiy report contains
   * contains information about the expected frame count and the existing frame
   * count.
   * Note, that the expected frame count is a pure heuristic value based on the
   * Send-IDs stored in the measurement. If the expectd frame count cannot be
   * computed, it is set to -1. That happens in cases when multiple publishers
   * send data on the same topic. However, even this case cannot be determined
   * with absolute certanty.
   *
   * The channel names in the continutity map are source-channel-names as stored
   * in the measurement; i.e. the channel mapping does not affect them.
   *
   * If no measurement is loaded, an empty map is returned.
   *
   * @return a map containing information about the expected and actual frame count for each channel
   */
  std::map<std::string, ContinuityReport> CreateContinuityReport() const;

  /**
   * @brief Returns how often each channel has been published
   *
   * Returns a map containing information how often each channel has been
   * published. The counters are resetted, when the measurement is closed, a new
   * measurement is loaded or the publishers are de-initialized.
   *
   * The channel names in the  map are source-channel-names as stored in the
   * measurement; i.e. the channel mapping does not affect them.
   *
   * @return A map with information how often each channel has been published
   */
  std::map<std::string, long long> GetMessageCounters() const;

  /**
   * @brief Returns the duration between the first and last frame of the loaded measurement
   * @return The length of the current measurement (or 0s, if no measurement is loaded)
   */
  std::chrono::nanoseconds GetMeasurementLength() const;

  /**
   * @brief Returns the maximum (@code{.first}) and minimum (@code{.second}) timestamp of the measurement
   * @return The maximum and minimum timestamp (or [0s, 0s], if no measurement is loaded)
   */
  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> GetMeasurementBoundaries() const;

  /**
   * @brief Sets the interval that limits the playback to a smaller part of the measurement
   *
   * This function uses index values to determine the limit interval. If the
   * current position is outside the selected interval, the position and
   * simulation time is changed accordingly.
   *
   * If no measurement is loaded, the limit interval will not be set.
   *
   * @param limit_interval    The index-interval to limit the playback to
   * @return @code{true}, if setting the limit interval was successfull (i.e. if a measurement is loaded)
   */
  bool SetLimitInterval(const std::pair<long long, long long>& limit_interval) const;

  /**
   * @brief Sets the interval that limits the playback to a smaller part of the measurement
   *
   * This function uses time pointsto determine the limit interval. The time
   * points are converted to indices. Thus, if the time point does not exactly
   * describe a frame, the nearest frame is selected. Note, that this can cause
   * the resulting limit interval to have different temporal boundaries than set
   * with this function. Use @see{GetLimitInterval} to get the resulting
   * interval.
   *
   * If the current position is outside the selected interval, the position and
   * simulation time is changed accordingly.
   *
   * If no measurement is loaded, the limit interval will not be set.
   *
   * @param limit_interval    The timepoint-interval to limit the playback to
   * @return @code{true}, if setting the limit interval was successfull (i.e. if a measurement is loaded)
   */
  bool SetLimitInterval(const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& limit_interval) const;

  /**
   * @brief Returns the index-limit-interval that limits the playback to a smaller part of the measurement
   *
   * If the limit interval has never been set, it will be [0, FrameCount - 1].
   * If no measurement is loaded, the limit interval will be [0, 0].
   *
   * @return The current limit interval
   */
  std::pair<long long, long long> GetLimitInterval() const;

  /**
   * @brief Returns the total number of messages in the current measurement
   * @return the number of messages in the current measurement
   */
  long long GetFrameCount() const;

  /**
   * @brief Returns the timestamp of a given index (or a 0s if the index is too small or too great / a negative time point if no measurement is loaded)
   * @param frame_index    The index to get the timestamp of
   * @return the timestamp of the given index
   */
  eCAL::Time::ecal_clock::time_point GetTimestampOf(long long frame_index) const;

  //////////////////////////////////////////////////////////////////////////////
  //// Settings                                                             ////
  //////////////////////////////////////////////////////////////////////////////

  // TODO: document
  void SetScenarios(const std::vector<EcalPlayScenario>& scenarios);
  bool SaveScenariosToDisk() const;

  /**
   * @brief Enables / disables measurement repeating.
   *
   * If repeating is enabled, the playback will start from the beginning if the
   * end has been reached
   * The default value is @code{false}.
   *
   * @param enabled    Whether repeating shall be enabled
   */
  void SetRepeatEnabled(bool enabled) const;

  /**
   * @brief Sets the relative play speed
   *
   * The play speed is relative to the original speed, i.e. a play speed of 1.0
   * indicates original speed. A factor smaller than 0.0 is equivalent to 0.0.
   *
   * This property is ignored, if the LimitPlaySpeed property is disabled.
   *
   * The default value is @code{1.0}.
   *
   * @param speed    The relative play speed
   */
  void SetPlaySpeed(double speed) const;

  /**
   * @brief Limit the play speed to the rate set by @see{SetPlaySpeed}
   *
   * If enabled, the playback will be limited to the given speed. If disabled,
   * the player will not wait between frames and therefore publish all messages
   * as fast as possible. In that case, the __sime_time__ rate will be the
   * computed acutal play rate to describe the simulation time as accurate as
   * possible.
   *
   * If disabled, the player will ignore the following settings:
   *  - PlaySpeed
   *  - FrameDroppingAllowed
   *  - EnforceDelayAccuracy
   *
   * The default value is @code{true}.
   *
   * @param enabled   Whether the play speed should be limited
   */
  void SetLimitPlaySpeedEnabled(bool enabled) const;

  /**
   * @brief Allow the player to drop frames in order to achieve temporal accuracy
   *
   * If enabled, the player will drop any frame that cannot be published at the
   * time it should be published. Thus, the overall time will stay accurate,
   * but not all messages may be published.
   *
   * If enabled, the player will ignore the following settings:
   *  - EnforceDelayAccuracy
   *
   * This property is ignored, if the LimitPlaySpeed property is disabled.
   *
   * The default value is @code{false}.
   *
   * @param allowed    Whether the player may drop frames
   */
  void SetFrameDroppingAllowed(bool allowed) const;

  /**
   * @brief Force the player to always keep the accurate time between two frames
   *
   * If enabled, the player will always try to keep the accurate time between
   * two frames. Thus, if one frame got delayed, this will delay all following
   * frames, too. This will slow down the entire measurement. In that case, the
   * __sim_time__ may jump backwards.

   * In contrast, if this property is disabled, A delayed frame may not slow
   * down the measurment, if a following delay can be used to catch up with the
   * original play speed. The time between two frames however may not be
   * accurate.
   *
   * This property is ignored, if the LimitPlaySpeed poperty is disabled or the
   * FrameDropping property is enabled.
   *
   * The default value is @code{false}.
   *
   * @param enabled   Whether the player should be forced to keep the delay between two frames accurate
   */
  void SetEnforceDelayAccuracyEnabled(bool enabled) const;

  /**
   * @brief Checks whether the player starts from the beginning, if the measurement end has been reached
   * The default value is @code{false}.
   * @return Whether repeat is enabled
   */
  bool IsRepeatEnabled() const;

  /**
   * @brief Gets the current configured play speed
   *
   * The play speed is relative to the original speed, i.e. a play speed of 1.0
   * indicates original speed. A factor smaller than 0.0 is equivalent to 0.0.
   *
   * This property is ignored, if the LimitPlaySpeed property is disabled.
   *
   * The default value is @code{1.0}.
   *
   * @return The configured play speed
   */
  double GetPlaySpeed() const;

  /**
   * @brief Checks whether the player should limit the play speed to the configured rate
   *
   * If enabled, the playback will be limited to the given speed. If disabled,
   * the player will not wait between frames and therefore publish all messages
   * as fast as possible. In that case, the __sime_time__ rate will be the
   * computed acutal play rate to describe the simulation time as accurate as
   * possible.
   *
   * If disabled, the player will ignore the following settings:
   *  - PlaySpeed
   *  - FrameDroppingAllowed
   *  - EnforceDelayAccuracy
   *
   * @return Whether the player should limit the play speed to the configured rate
   */
  bool IsLimitPlaySpeedEnabled() const;

  /**
   * @brief Checks whether the player may drop frames in order to achieve temporal accuracy
   *
   * If enabled, the player will drop any frame that cannot be published at the
   * time it should be published. Thus, the overall time will stay accurate,
   * but not all messages may be published.
   *
   * If enabled, the player will ignore the following settings:
   *  - EnforceDelayAccuracy
   *
   * This property is ignored, if the LimitPlaySpeed property is disabled.
   *
   * The default value is @code{false}.
   *
   * @return Whether the player may drop frames in order to achieve temporal accuracy
   */
  bool IsFrameDroppingAllowed() const;

  /**
   * @brief Checks whether the player is forced to always keep the accurate time between two frames
   *
   * If enabled, the player will always try to keep the accurate time between
   * two frames. Thus, if one frame got delayed, this will delay all following
   * frames, too. This will slow down the entire measurement. In that case, the
   * __sim_time__ may jump backwards.
   * In contrast, if this property is disabled, A delayed frame may not slow
   * down the measurment, if a following delay can be used to catch up with the
   * original play speed. The time between two frames however may not be
   * accurate.

   *
   * This property is ignored, if the LimitPlaySpeed poperty is disabled or the
   * FrameDropping property is enabled.
   *
   * The default value is @code{false}.
   *
   * @return Whether the player is forced to always keep the accurate time between two frames
   */
  bool IsEnforceDelayAccuracyEnabled() const;

  //////////////////////////////////////////////////////////////////////////////
  //// Playback                                                             ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Initializes all eCAL Publishers with their default name
   *
   * Creates all eCAL Publishers for the currently loaded measurement with their
   * default name. If the publishers are already initialized, everything will be
   * re-initialized. Thus, the message counters are resetted, as well.
   *
   * Initializing the publishers fails, if no measurement is loaded.
   *
   * @return True if successfull
   */
  bool InitializePublishers() const;

  /**
   * @brief Initializes all eCAL Publishers that are present in the given channel mapping
   *
   * Creates all eCAL Publishers from the channel mapping. The channel_mapping
   * has to have pairs in the form:
   *    [SourceChannelName] = TargetChannelName
   *
   * If the publishers are already initialized, everything will be
   * re-initialized. Thus, the message counters are resetted, as well.
   *
   * Initializing the publishers fails, if no measurement is loaded.
   *
   * @param channel_mapping    A [source]->target channel name mapping
   *
   * @return True if successfull
   */
  bool InitializePublishers(const std::map<std::string, std::string>& channel_mapping) const;

  /**
   * @brief De-initializes all eCAL Publishers
   *
   * De-initializes all eCAL Publishers that have been initialized by
   * @see{InitializePublishers}. The message counters are resetted to 0.
   *
   * De-initializing the publishers fails, if no measurement is loaded.
   *
   * If no publishers are initialized but a measurement is loaded, this function
   * does nothing, but will still report success.
   *
   * @return True if successfull
   */
  bool DeInitializePublishers() const;

  /**
   * @brief Checks whether eCAL Publishers have been initialized
   * @return True if initialized
   */
  bool IsInitialized() const;
 
  //TODO: document
  std::map<std::string, std::string> GetChannelMapping() const;

  /**
   * @brief Starts the playback
   *
   * Starts playing the currently loaded measurement.
   * The playback will start from the last position, the beginning of the
   * measurement or the beginning of the limit interval, depending on the
   * situation.
   *
   * If no measurement is loaded, the function will do nothing and report a
   * failure.
   * If the publishers have not been initialized, yet, all publishers are
   * initialized with their default channel names.
   * When playing, the __sim_time__ will be marked as PLAYING.
   *
   * An optional play_until_index parameter can be provided. If set, the
   * playback will stop at the given index. If the index is outside the valid
   * range [0, frame_count], is does not have any affect.
   *
   * @param play_until_index    The index until the measurement should be played
   *
   * @return True if successfull
   */
  bool Play(long long play_until_index = -1) const;

  /**
   * @brief Pauses the playback
   *
   * Pauses the playback, if the player is currently playing a measurement. If
   * the player is already paused, this functio does nothing, but still reports
   * success.
   * If no measurement is loaded, this function reports a failure.
   *
   * When paused, the __sim_time__ will be marked as PAUSED
   *
   * @return True if successfull
   */
  bool Pause() const;

  /**
   * @brief Steps the player by one frame
   *
   * Publishes one frame and advances the current poisiton. This will also step
   * the __sim_time__. The player will *not* change into the playing state.
   *
   * If no measurement is loaded, this function reports a failure
   *
   * @return True if successfull
   */
  bool StepForward() const;

  /**
   * @brief Starts the playback and plays until a message of a given channel is reached
   *
   * Starts the playback and plays until a message of a given channel is
   * reached. If there is no further occurence of the given channel, the
   * playback will stop at the end (if repeat is disabled) or search for the
   * next occurence from the beginning.
   * The given channel name is the source channel as stored in the measurement,
   * i.e. the channel mapping does not have any effect.
   *
   * If the eCAL Publishers are not initialized, all publishers from the
   * measurement are initialized with their default name.
   *
   * If no measurement is loaded, this function reports a failure.
   *
   * @param source_channel_name    The channel name until which to play
   *
   * @return True if successfull
   */
  bool PlayToNextOccurenceOfChannel(const std::string& source_channel_name) const;

  /**
   * @brief Computes and returns the current play speed
   *
   * The current play speed is computed by taking the last (realtime) second
   * and checking, how many messages with which timestamps have been published.
   *
   * When the playback is paused, this pause will not affect the play speed
   * computation, i.e. the last realtime second is taken only from when the
   * player was running.
   *
   * When the current position is set (by @see{JumpTo} or by repeating from the
   * beginning), the current play speed is resetted. In that case, the set
   * target play speed is returned.
   *
   * Note that when the player is paused, the play speed from before the pause
   * is returned. Thus, the play speed will most likely not be zero, even though
   * nothing is currently published.
   *
   * @return The current play speed
   */
  double GetCurrentPlaySpeed() const;

  /**
   * @brief Sets the current position of the player to a specific frame index
   *
   * Sets the current position to the given index. The index is bound to the
   * range of valid indices consisting of the measurement length and the set
   * limitInterval.
   * The index will be the next frame that is published.
   * The __sim_time__ will be set to the timestamp of the given frame index.
   *
   * If no measurement is loaded, this function reports a failure.
   *
   * @param index    The index of the frame to jump to
   *
   * @return True if successfull
   */
  bool JumpTo(long long index) const;

  /**
   * @brief Sets the current position of the player to the frame identified by the given timestamp
   *
   * Sets the current position to the frame that is nearest to the given
   * timestamp. Although the parameter is an absolute time point, the set
   * position will be determined by the frame that is closest to the given time.
   *
   * The range of valid timestamps can be determined by
   * @see{GetMeasurementBoundaries()}
   *
   * The __sim_time__ will be set to the timestamp of the determined frame.
   *
   * If no measurement is loaded, this function reports a failure.
   *
   *
   * @param timestamp    The absolute time to jump to
   *
   * @return True if successfull
   */
  bool JumpTo(eCAL::Time::ecal_clock::time_point timestamp) const;

  //////////////////////////////////////////////////////////////////////////////
  //// State                                                                ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Returns a summary of the most important play state information
   *
   * An EcalPlayState consists of:
   *   - Whether the player is playing
   *   - The current play rate
   *   - Index and timestamp of the current frame
   *   - Information about the simulation time
   *
   * @return The current player state
   */
  EcalPlayState GetCurrentPlayState() const;

  /**
   * @brief Checks whether the playback is running
   * @return True if the playback is running
   */
  bool IsPlaying() const;

  /**
   * @brief Checks whether the playback is not running
   * @return True if the playback is not running
   */
  bool IsPaused() const;

  /**
   * @brief Gets the absolute eCAL timestamp of the current frame
   *
   * The "current" frame is:
   *    - The selected frame after the player jumped to a new position (e.g.
   *      when a measurement has been loaded, the position was set manually by
   *      the user or the playback has finished)
   *    - The last published frame in all other cases (e.g. when the player is
   *      currently playing, has been paused or stepped)
   *
   * @return The timestamp of the current frame
   */
  eCAL::Time::ecal_clock::time_point GetCurrentFrameTimestamp() const;

  /**
   * @brief Gets the index of the current frame
   *
   * The "current" frame is:
   *    - The selected frame after the player jumped to a new position (e.g.
   *      when a measurement has been loaded, the position was set manually by
   *      the user or the playback has finished)
   *    - The last published frame in all other cases (e.g. when the player is
   *      currently playing, has been paused or stepped)
   *
   * @return The current index
   */
  long long GetCurrentFrameIndex() const;

private:
  std::string description_;                                                     /**< The description loaded from the current measurement. Empty, if no measurement is loaded or no description file has been found*/
  std::vector<EcalPlayScenario> scenarios_;                                     /**< The scenarios loaded from the current measurement. Empty, if no measurement is loaded or no description file has been found*/

  std::unique_ptr<PlayThread> play_thread_;                                     /**< The "actual" eCAL Player that runs in it's own thread and executes commands from outside */
  std::string                 measurement_path_;                                 /**< The path that was loaded. It may point to a file or a directory (depending on if the user loaded a measurement file or a mesaurement directory */

  /**
   * @brief Loads a text file from the given file as measurement description
   *
   * If the file does not exist, the description will stay empty.
   *
   * @param path    The path to the description file
   * @return True if successfull
   */
  bool LoadDescription(const std::string& path);

  /**
   * @brief Loads and parses scenarios from the given path
   *
   * A scenario file is a text file containing semicolon-separated name and time
   * inforamtion with one timestamp per line. The timestamps are relative
   * timestamps in seconds starting with the first frame of the measurment
   * as 0.0s.
   *
   * Example:
   *
   *    1.474 ; extraordinary event
   *    3.995 ; construction site
   *    7.421 ; curve left
   *
   * If the file does not exist, the scenario list will stay empty.
   *
   * @param path    The path to the senario file
   * @return True if successfull
   */
  bool LoadScenarios(const std::string& path);

  /**
   * @brief Prints version information to the logging ouptut
   */
  void LogAppNameVersion() const;

  /**
   * @brief Prints information about the loaded measurement to the logging output (e.g. Length, Frame count, path, etc.)
   */
  void LogMeasurementSummary() const;
};
