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

/**
 * @brief Container carrying information about how many frames a channel is expected to have and how many frames are actually contained in the measurement
 */
struct ContinuityReport
{
  long long expected_frame_count_;                                              /**< How many frames the channel is expected to have */
  long long existing_frame_count_;                                              /**< How many frames are actually in the measurement */
  
  ContinuityReport(long long expected_frame_count, long long existing_frame_count)
    : expected_frame_count_(expected_frame_count)
    , existing_frame_count_(existing_frame_count)
  {}
};
