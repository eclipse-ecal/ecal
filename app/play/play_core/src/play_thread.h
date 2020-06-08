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

#include "ThreadingUtils/InterruptibleThread.h"

#include "measurement_container.h"
#include "ecal_play_state.h"
#include "continuity_report.h"
#include "ecal_play_command.h"
#include "stop_watch.h"

#include <ecalhdf5/eh5_meas.h>
#include <ecal/ecal.h>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <utility>

class StatePublisherThread;

/**
 * @brief The PlayThread is the actual heart of eCAL Play.
 *
 * The Play Thread offers the actual thread safe API that is then made publicly
 * available through a separate wrapper.
 *
 * After instanciating, the thread should be started with start(). It will then
 * be able to execute commands like play(), pause(), etc.
 */
class PlayThread : public InterruptibleThread
{
public:

  PlayThread();
  ~PlayThread();

  void Interrupt() override;

  //////////////////////////////////////////////////////////////////////////////
  //// Measurement                                                          ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Sets the currently used measurement
   *
   * If there already was a measurement loaded, the old measurement is replaced.
   * After setting the measurement, the __ecalplay_state__ topic is published
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
   * (or PLAYING after a playback has started)w
   *
   * @param measurement    The new measurement
   * @param path           The (optional) path from where the measurement was loaded
   */
  void SetMeasurement(std::shared_ptr<eCAL::eh5::HDF5Meas> measurement, const std::string& path = "");

  /**
   * @brief Returns whether a measurement has successfully been loaded
   * @return True if a measurement is loaded
   */
  bool IsMeasurementLoaded();

  /**
   * @brief Returns the path of the currently loaded measurment
   *
   * If no measurement was loaded, an empty string is returned. On the opposite,
   * an empty string does NOT indicate, that no measurement was set.
   *
   * @see SetMeasurement
   *
   * @return The measurement path
   */
  std::string GetMeasurementPath();

  /**
   * @brief Returns the duration between the first and last frame of the loaded measurement
   * @return The length of the measurement (or 0 if no measurement is loaded)
   */
  std::chrono::nanoseconds GetMeasurementLength();

  /**
   * @brief Returns the maximum (@code{.first}) and minimum (@code{.second}) timestamp of the measerement
   * @return The maximum and minimum timestamp (or [0s, 0s], if no measurement is loaded)
   */
  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> GetMeasurementBoundaries();

  /**
   * @brief Returns the total number of messages in the current measurement
   * @return the number of messages in the current measurement
   */
  long long GetFrameCount();

  /**
   * @brief Returns the timestamp of a given index (or a 0s if the index is too small or too great / a negative time point if no measurement is loaded)
   * @param frame_index    The index to get the timestamp of
   * @return the timestamp of the given index
   */
  eCAL::Time::ecal_clock::time_point GetTimestampOf(long long frame_index);

  /**
   * @brief Returns a list of all channels in the measurment
   *
   * The channel set contains source-channel-names as they are stored in the
   * measurement file; i.e the channel mapping does not affect this list.
   * If no measurement is loaded, an empty set is returned.
   *
   * @return A set of all channels from the measurement
   */
  std::set<std::string> GetChannelNames();

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
  std::map<std::string, ContinuityReport> CreateContinuityReport();

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
  std::map<std::string, long long> GetMessageCounters();

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
  bool SetLimitInterval(const std::pair<long long, long long>& limit_interval);

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
  bool SetLimitInterval(const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& limit_interval);

  /**
   * @brief Returns the index-limit-interval that limits the playback to a smaller part of the measurement
   *
   * If the limit interval has never been set, it will be [0, FrameCount - 1].
   * If no measurement is loaded, the limit interval will be [0, 0].
   *
   * @return The current limit interval
   */
  std::pair<long long, long long> GetLimitInterval();

  //////////////////////////////////////////////////////////////////////////////
  //// Settings                                                             ////
  //////////////////////////////////////////////////////////////////////////////
  ///
  /**
   * @brief Enables / disables measurement repeating.
   *
   * If repeating is enabled, the playback will start from the beginning if the
   * end has been reached
   * The default value is @code{false}.
   *
   * @param enabled    Whether repeating shall be enabled
   */
  void SetRepeatEnabled(bool enabled);

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
  void SetPlaySpeed(double speed);

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
  void SetLimitPlaySpeedEnabled(bool enabled);

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
  void SetFrameDroppingAllowed(bool allowed);

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
  void SetEnforceDelayAccuracyEnabled(bool enabled);

  /**
   * @brief Checks whether the player starts from the beginning, if the measurement end has been reached
   * The default value is @code{false}.
   * @return Whether repeat is enabled
   */
  bool IsRepeatEnabled();

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
  double GetPlaySpeed();

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
  bool IsLimitPlaySpeedEnabled();

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
  bool IsFrameDroppingAllowed();

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
  bool IsEnforceDelayAccuracyEnabled();

  //////////////////////////////////////////////////////////////////////////////
  //// Playback                                                             ////
  //////////////////////////////////////////////////////////////////////////////

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
  bool Play(long long until_index = -1);

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
  bool PlayToNextOccurenceOfChannel(const std::string& source_channel_name);

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
  bool Pause();

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
  bool StepForward();

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
  double GetCurrentPlaySpeed();

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
  bool JumpTo(long long index);

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
  bool JumpTo(eCAL::Time::ecal_clock::time_point timestamp);

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
  EcalPlayState GetCurrentPlayState();

  /**
   * @brief Checks whether the playback is running
   * @return True if the playback is running
   */
  bool IsPlaying();

  /**
   * @brief Checks whether the playback is not running
   * @return True if the playback is not running
   */
  bool IsPaused();

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
  eCAL::Time::ecal_clock::time_point GetCurrentFrameTimestamp();

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
  long long GetCurrentFrameIndex();

  //////////////////////////////////////////////////////////////////////////////
  //// Publishers                                                           ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Initializes all eCAL Publishers with their default name
   *
   * Creates all eCAL Publishers for the currently loaded measurement with their
   * default name. If the publisher sare already initialized, everything will be
   * re-initialized. Thus, the message counters are resetted, as well.
   *
   * Initializing the publishers fails, if no measurement is loaded.
   *
   * @return True if successfull
   */
  bool InitializePublishers();

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
  bool InitializePublishers(const std::map<std::string, std::string>& channel_mapping);

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
  bool DeInitializePublishers();

  /**
   * @brief Checks whether eCAL Publishers have been initialized
   * @return True if initialized
   */
  bool PubishersInitialized();

  //TODO: document
  std::map<std::string, std::string> GetChannelMapping();

protected:
  void Run() override;

private:
  /**
   * @brief Sets the current play rate
   *
   * When calling this method, it will:
   *   - set the play speed
   *   - update the internally stored sim time, as it will run with a different speed from now on
   *   - Trigger the state publisher to publish the new information
   *
   * This function does not lock any mutex. It modifies the command_, so the
   * command_mutex_ has to be locked when calling this function.
   *
   * @param play_rate    The new play rate
   */
  void SetPlayRate_Private(double play_rate);

  /**
   * @brief Sets the curernt simulation time
   *
   * Sets the simulation time that is valid when calling this function.
   *
   * This function does not lock any mutex. It modifies the command_, so the
   * command_mutex_ has to be locked when calling this function.
   *
   * @param current_sim_time    The new simulation time
   */
  void SetSimTime_Private(eCAL::Time::ecal_clock::time_point current_sim_time);

  /**
   * @brief Returns the (interpolated) simulation time that is valid when calling this function
   *
   * This function does not lock any mutex. It reads the command_, so the
   * command_mutex_ has to be locked when calling this function.
   *
   * @return The current simulatoin time
   */
  eCAL::Time::ecal_clock::time_point GetCurrentSimTime_Private() const;

  /**
   * @brief Calculates the system time that maps to the given simulation time considering the current play speed
   *
   * If the current play speed is 0.0, the given simulation time will never be
   * reached. In that case, std::chrono::steady_clock::time_point::max() is
   * returned.
   * 
   * This function does not lock any mutex. It reads the command_, so the
   * command_mutex_ has to be locked when calling this function.
   *
   * @param sim_time    The simulation time to calculate the system time for
   * @return            The calculated system time or std::chrono::steady_clock::time_point::max()
   */
  std::chrono::steady_clock::time_point GetSystemTime_Private(eCAL::Time::ecal_clock::time_point sim_time) const;

  /**
   * @brief Starts or pauses the playback
   *
   * The internally stored simulation time is updated. The stopwatch that
   * measures the actual play speed is paused or resumed.
   *
   * This function does not lock any mutex. It reads the command_, so the
   * command_mutex_ has to be locked when calling this function.
   *
   * @param playing
   */
  void SetPlaying_Private(bool playing);

  /**
   * @brief Calculates the current play speed
   *
   * The current play speed is calculated by checking how many measurement time
   * has passed in the last realtime second. Each time the measurement is
   * paused, the realtime stopwatch is paused, so the pause does not affect the
   * current play speed. If the play speed cannot be determined (because nothing
   * has been published since the last time the player jumped etc.), the
   * configured target play speed is returned.
   *
   * Note that if the player is currently paused, the last play speed before the
   * pause will be returned, i.e. the play speed will most likely not be zero.
   *
   * This function does not lock any mutex. It reads the command_ and
   * frame_stopwatch_, so the command_mutex_ has to be locked when calling this
   * function.
   *
   * @return The current play speed
   */
  double GetCurrentPlaySpeed_Private() const;


  /**
   * @brief Initializes the eCAL Publishers in the measurement_container_
   *
   * A channel_mapping can be used to rename the publishers or to only
   * initialize some of them. If the use_channel_mapping is true, the given
   * mapping will be used. Otherwise, all publishers will be initialized with
   * their default name from the measurement.
   *
   * If the publishers are already initialized, everything will be
   * re-initialized. Thus, the message counters are resetted as well.
   * Initializing the publishers fails, if no measurement is loaded.
   *
   * This function will lock the measurement_mutex_ and command_mutex_. Thus,
   * none of these mutexes must be locked.
   *
   * @param use_channel_mapping    Whether to use the channel_mapping to rename / filter publishers
   * @param channel_mapping        A map containing the source and target name of all publishers that should be initialized
   *
   * @return True if successfull
   */
  bool InitializePublishers_Private(bool use_channel_mapping, const std::map<std::string, std::string>& channel_mapping = std::map<std::string, std::string>());

  /**
   * @brief Sets the current limit interval to either the given indixes (if they are not a nullptr) or the given timestamps.
   *
   * The Limit interval is determined either by the given indexes, or the given
   * timestamps. If the indexes are nullptr, the timestamps must not be nullptr,
   * as they are then used to calculate the required indexes.
   *
   * The input is bound to the available frames / timestamps. The timestamps are
   * converted into indices, if applicable. Thus, the limit interval timestamps
   * may differ from the input, if no index with that exact timestamp exists.
   *
   * Setting the limit interval failes, if no measurement is loaded.
   *
   * This function will lock the measurement_mutex_ and command_mutex_. Thus,
   * none of these mutexes must be locked.
   *
   * @param indexes       The indexes of the limit interval. If this is a nullptr, the timestamps will be used to calculate the indexes.
   * @param timestamps    The timestamps of the limit interval. If hte indexes are a nullptr, this must not also be nullptr.
   * @return
   */
  bool SetLimitInterval_Private(const std::pair<long long, long long>* indexes, const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>* timestamps);

  /**
   * @brief Prints the given channel mapping to the log output
   * @param channel_mapping    The mapping to print
   */
  static void LogChannelMapping(const std::map<std::string, std::string>& channel_mapping);


////////////////////////////////////////////////////////////////////////////////
//// Member variables                                                       ////
////////////////////////////////////////////////////////////////////////////////
private:
  // Measurement
  std::shared_timed_mutex               measurement_mutex_;                     /**< A mutex that protects the measurement_container_. When the measurement_container_ is modified internally or replaced with another one, this mutex must be locked unique. */
  std::unique_ptr<MeasurementContainer> measurement_container_;                 /**< The wrapped measurement */

  // State
  std::mutex               command_mutex_;                                      /**< A mutex protecting the command_, time_log_ and time_log_complete_time_span_ variables. It is also the mutex for the pause_cv_ condition variable used for pausing the playback and waiting between frames. */
  EcalPlayCommand          command_;                                            /**< A struct containing various information (e.g. wether the thread is suppsed to play or pause and up to which frame it should play).*/
  std::condition_variable  pause_cv_;                                           /**< The condition variable used for pausing and waiting for the next frame. This variable has to be notified every time something playback related changed, so the player can react to the change. */
  std::deque <std::pair<eCAL::Time::ecal_clock::time_point, std::chrono::nanoseconds>> time_log_; /** This list stores how long it took to publish a frame with a specific timestamp. This information is used to compute the current play rate. Only the last second is stored. */
  std::chrono::nanoseconds time_log_complete_time_span_;                        /**< The accumulated real time duration of the time_log_. Used for efficiently removing elements to limit it to 1s. */
  Stopwatch                frame_stopwatch_;                                    /**< A Stopwatch measuring the time between frames */
  
  // State publisher
  std::unique_ptr<StatePublisherThread> state_publisher_thread_;                /**< Periodically publishes the __sim_time__ and __ecalplay_state__ topics */
};
