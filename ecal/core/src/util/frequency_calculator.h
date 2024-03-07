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
      , first_tick(now_)
      , last_tick(now_)
      , previous_frequency(0.0)
    {
    }

    void addTick(const time_point& now)
    {
      counted_elements++;
      last_tick = now;
    }

    double getFrequency()
    {
      if (counted_elements == 0)
      {
        return previous_frequency;
      }

      previous_frequency = calculateFrequency<T>(first_tick, last_tick, counted_elements);
      first_tick = last_tick;
      counted_elements = 0;
      return previous_frequency;
    }

  private:
    long long counted_elements;
    time_point first_tick;
    time_point last_tick;
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
      last_tick = now;
    }

    double getFrequency(const time_point& now)
    {
      double frequency = calculator ? calculator->getFrequency() : 0.0;

      if (frequency == 0.0)
      {
        return 0.0;
      }

      // calculate theoretical frequency to detect timeouts;
      double theoretical_frequency = calculateFrequency<T>(last_tick, now, 1);
      // If the frequency is higher than reset_factor * theoretical_frequency, we reset. 
      if (frequency >= theoretical_frequency * reset_factor)
      {
        calculator.reset();
        return 0.0;
      }

      return frequency;
    }

  private:
    float reset_factor;
    time_point last_tick;
    std::unique_ptr<FrequencyCalculator<T>> calculator;
  };
}





