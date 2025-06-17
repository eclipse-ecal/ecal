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

#include "custom_types/byte_size.h"

QString ByteSize::toString(int precision) const
{
  // Binary (IEC) units: B, KiB, MiB, GiB, …
  static constexpr const char* units[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB" };

  double value = double(m_bytes);
  int    idx   = 0;
  // divide by 1024 until we're under 1024 or out of units
  while (value >= 1024.0 && idx < (int)(sizeof(units)/sizeof(*units)) - 1) {
    value /= 1024.0;
    ++idx;
  }
  // e.g. QString::number(1.23456, 'f', 2) -> "1.23"
  return QString::number(value, 'f', precision) + ' ' + units[idx];
}

bool ByteSize::operator<(const ByteSize& other) const {
  return m_bytes < other.m_bytes;
}