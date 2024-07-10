/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

/**
 * @brief This file provides classes to calculate frequencies.
 *        These classes are NOT threadsafe!
**/

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <numeric>

namespace eCAL
{
  template <typename T>
  double calculateFrequency(const std::chrono::time_point<T>& start, const std::chrono::time_point<T>& end, long long ticks)
  {
    auto time_delta = (end - start).count();
    if (time_delta == 0)
    {
      return std::numeric_limits<double>::max();
    }
    auto denominator = typename T::duration::period().den;
    auto numerator = typename T::duration::period().num;
    return ((double)denominator * ticks / ((double) numerator * time_delta));
  }

  template <class T>
  class FrequencyCalculator
  {
  public:
    using time_point = std::chrono::time_point<T>;

    FrequencyCalculator(const time_point& now_)
      : counted_elements(0)
      , first_tick_time(now_)
      , last_tick_time(now_)
      , previous_frequency(0.0)
    {
    }

    void addTick(const time_point& now)
    {
      counted_elements++;
      last_tick_time = now;
    }

    double getFrequency()
    {
      if (counted_elements == 0)
      {
        return previous_frequency;
      }

      previous_frequency = calculateFrequency<T>(first_tick_time, last_tick_time, counted_elements);
      first_tick_time = last_tick_time;
      counted_elements = 0;
      return previous_frequency;
    }

  private:
    long long counted_elements;
    time_point first_tick_time;
    time_point last_tick_time;
    double previous_frequency;
  };


  template <class T>
  class ResettableFrequencyCalculator
  {
  public:
    using time_point = std::chrono::time_point<T>;

    ResettableFrequencyCalculator(float reset_factor_)
      : reset_factor(reset_factor_)
    {}

    void addTick(const time_point& now)
    {
      if (!calculator)
      {
        calculator = std::make_unique<FrequencyCalculator<T>>(now);
      }
      else
      {
        calculator->addTick(now);
      }
      last_tick_time = now;
      received_tick_since_get_frequency_called = true;
    }

    double getFrequency(const time_point& now)
    {
      double frequency = calculator ? calculator->getFrequency() : 0.0;

      // if the frequency is 0.0, return it right away
      if (frequency == 0.0)
      {
        return 0.0;
      }

      // if we have received ticks, this means we don't want to reset the calculation
      // so we return the frequency that the calculator has calculated
      if (received_tick_since_get_frequency_called)
      {
        received_tick_since_get_frequency_called = false;
        return frequency;
      }

      // if we have not received ticks since the last call of getFrequency
      // then we calculate the  ΔT, e.g. the timespan in seconds between last received and next incoming element
      // Based on the last time we received a tick we could calculate when we expect the next tick
      // We can also calculate when we want to time out, based on the reset factor
      // timeout = last_tick + ΔT * reset_factor
      // If the current time is greater than the timeout time, we reset the calculator and return 0
      auto expected_time_in_seconds_between_last_and_next_tick = std::chrono::duration<double>(1 / frequency);
      time_point timeout_time = last_tick_time + std::chrono::duration_cast<std::chrono::nanoseconds>(expected_time_in_seconds_between_last_and_next_tick * reset_factor);
      if (now > timeout_time)
      {
        calculator.reset();
        return 0.0;
      }

      return frequency;
    }

  private:
    float reset_factor;
    time_point last_tick_time;
    bool received_tick_since_get_frequency_called = false;
    std::unique_ptr<FrequencyCalculator<T>> calculator;
  };
}





