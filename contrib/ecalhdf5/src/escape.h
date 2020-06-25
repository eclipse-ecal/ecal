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

namespace eCAL
{
  namespace eh5
  {
    namespace
    {
      // Lookup table to tell us which character we must escape
      constexpr bool is_reserved_[]
      {
        true,   //  NUL     (0x00)
        true,   //  SOH     (0x01)
        true,   //  STX     (0x02)
        true,   //  ETX     (0x03)
        true,   //  EOT     (0x04)
        true,   //  ENQ     (0x05)
        true,   //  ACK     (0x06)
        true,   //  BEL     (0x07)
        true,   //  BS      (0x08)
        true,   //  HT      (0x09)
        true,   //  LF      (0x0A)
        true,   //  VT      (0x0B)
        true,   //  FF      (0x0C)
        true,   //  CR      (0x0D)
        true,   //  SO      (0x0E)
        true,   //  SI      (0x0F)
        true,   //  DLE     (0x10)
        true,   //  DC1     (0x11)
        true,   //  DC2     (0x12)
        true,   //  DC3     (0x13)
        true,   //  DC4     (0x14)
        true,   //  NAK     (0x15)
        true,   //  SYN     (0x16)
        true,   //  ETB     (0x17)
        true,   //  CAN     (0x18)
        true,   //  EM      (0x19)
        true,   //  SUB     (0x1A)
        true,   //  ESC     (0x1B)
        true,   //  FS      (0x1C)
        true,   //  GS      (0x1D)
        true,   //  RS      (0x1E)
        true,   //  US      (0x1F)
        false,  //  SP      (0x20)
        false,  //  !       (0x21)
        false,  //  "       (0x22)
        false,  //  #       (0x23)
        false,  //  $       (0x24)
        true,   //  %       (0x25)
        false,  //  &       (0x26)
        false,  //  '       (0x27)
        false,  //  (       (0x28)
        false,  //  )       (0x29)
        false,  //  *       (0x2A)
        false,  //  +       (0x2B)
        true,   //  ,       (0x2C)
        false,  //  -       (0x2D)
        false,  //  .       (0x2E)
        true,   //  /       (0x2F)
        false,  //  0       (0x30)
        false,  //  1       (0x31)
        false,  //  2       (0x32)
        false,  //  3       (0x33)
        false,  //  4       (0x34)
        false,  //  5       (0x35)
        false,  //  6       (0x36)
        false,  //  7       (0x37)
        false,  //  8       (0x38)
        false,  //  9       (0x39)
        false,  //  :       (0x3A)
        false,  //  ;       (0x3B)
        false,  //  <       (0x3C)
        false,  //  =       (0x3D)
        false,  //  >       (0x3E)
        false,  //  ?       (0x3F)
        false,  //  @       (0x40)
        false,  //  A       (0x41)
        false,  //  B       (0x42)
        false,  //  C       (0x43)
        false,  //  D       (0x44)
        false,  //  E       (0x45)
        false,  //  F       (0x46)
        false,  //  G       (0x47)
        false,  //  H       (0x48)
        false,  //  I       (0x49)
        false,  //  J       (0x4A)
        false,  //  K       (0x4B)
        false,  //  L       (0x4C)
        false,  //  M       (0x4D)
        false,  //  N       (0x4E)
        false,  //  O       (0x4F)
        false,  //  P       (0x50)
        false,  //  Q       (0x51)
        false,  //  R       (0x52)
        false,  //  S       (0x53)
        false,  //  T       (0x54)
        false,  //  U       (0x55)
        false,  //  V       (0x56)
        false,  //  W       (0x57)
        false,  //  X       (0x58)
        false,  //  Y       (0x59)
        false,  //  Z       (0x5A)
        false,  //  [       (0x5B)
        false,  //  \       (0x5C)
        false,  //  ]       (0x5D)
        false,  //  ^       (0x5E)
        false,  //  _       (0x5F)
        false,  //  `       (0x60)
        false,  //  a       (0x61)
        false,  //  b       (0x62)
        false,  //  c       (0x63)
        false,  //  d       (0x64)
        false,  //  e       (0x65)
        false,  //  f       (0x66)
        false,  //  g       (0x67)
        false,  //  h       (0x68)
        false,  //  i       (0x69)
        false,  //  j       (0x6A)
        false,  //  k       (0x6B)
        false,  //  l       (0x6C)
        false,  //  m       (0x6D)
        false,  //  n       (0x6E)
        false,  //  o       (0x6F)
        false,  //  p       (0x70)
        false,  //  q       (0x71)
        false,  //  r       (0x72)
        false,  //  s       (0x73)
        false,  //  t       (0x74)
        false,  //  u       (0x75)
        false,  //  v       (0x76)
        false,  //  w       (0x77)
        false,  //  x       (0x78)
        false,  //  y       (0x79)
        false,  //  z       (0x7A)
        false,  //  {       (0x7B)
        false,  //  |       (0x7C)
        false,  //  }       (0x7D)
        false,  //  ~       (0x7E)
        true,   //  DEL     (0x7F)
        true,   // Codepage (0x80)
        true,   // Codepage (0x81)
        true,   // Codepage (0x82)
        true,   // Codepage (0x83)
        true,   // Codepage (0x84)
        true,   // Codepage (0x85)
        true,   // Codepage (0x86)
        true,   // Codepage (0x87)
        true,   // Codepage (0x88)
        true,   // Codepage (0x89)
        true,   // Codepage (0x8A)
        true,   // Codepage (0x8B)
        true,   // Codepage (0x8C)
        true,   // Codepage (0x8D)
        true,   // Codepage (0x8E)
        true,   // Codepage (0x8F)
        true,   // Codepage (0x90)
        true,   // Codepage (0x91)
        true,   // Codepage (0x92)
        true,   // Codepage (0x93)
        true,   // Codepage (0x94)
        true,   // Codepage (0x95)
        true,   // Codepage (0x96)
        true,   // Codepage (0x97)
        true,   // Codepage (0x98)
        true,   // Codepage (0x99)
        true,   // Codepage (0x9A)
        true,   // Codepage (0x9B)
        true,   // Codepage (0x9C)
        true,   // Codepage (0x9D)
        true,   // Codepage (0x9E)
        true,   // Codepage (0x9F)
        true,   // Codepage (0xA0)
        true,   // Codepage (0xA1)
        true,   // Codepage (0xA2)
        true,   // Codepage (0xA3)
        true,   // Codepage (0xA4)
        true,   // Codepage (0xA5)
        true,   // Codepage (0xA6)
        true,   // Codepage (0xA7)
        true,   // Codepage (0xA8)
        true,   // Codepage (0xA9)
        true,   // Codepage (0xAA)
        true,   // Codepage (0xAB)
        true,   // Codepage (0xAC)
        true,   // Codepage (0xAD)
        true,   // Codepage (0xAE)
        true,   // Codepage (0xAF)
        true,   // Codepage (0xB0)
        true,   // Codepage (0xB1)
        true,   // Codepage (0xB2)
        true,   // Codepage (0xB3)
        true,   // Codepage (0xB4)
        true,   // Codepage (0xB5)
        true,   // Codepage (0xB6)
        true,   // Codepage (0xB7)
        true,   // Codepage (0xB8)
        true,   // Codepage (0xB9)
        true,   // Codepage (0xBA)
        true,   // Codepage (0xBB)
        true,   // Codepage (0xBC)
        true,   // Codepage (0xBD)
        true,   // Codepage (0xBE)
        true,   // Codepage (0xBF)
        true,   // Codepage (0xC0)
        true,   // Codepage (0xC1)
        true,   // Codepage (0xC2)
        true,   // Codepage (0xC3)
        true,   // Codepage (0xC4)
        true,   // Codepage (0xC5)
        true,   // Codepage (0xC6)
        true,   // Codepage (0xC7)
        true,   // Codepage (0xC8)
        true,   // Codepage (0xC9)
        true,   // Codepage (0xCA)
        true,   // Codepage (0xCB)
        true,   // Codepage (0xCC)
        true,   // Codepage (0xCD)
        true,   // Codepage (0xCE)
        true,   // Codepage (0xCF)
        true,   // Codepage (0xD0)
        true,   // Codepage (0xD1)
        true,   // Codepage (0xD2)
        true,   // Codepage (0xD3)
        true,   // Codepage (0xD4)
        true,   // Codepage (0xD5)
        true,   // Codepage (0xD6)
        true,   // Codepage (0xD7)
        true,   // Codepage (0xD8)
        true,   // Codepage (0xD9)
        true,   // Codepage (0xDA)
        true,   // Codepage (0xDB)
        true,   // Codepage (0xDC)
        true,   // Codepage (0xDD)
        true,   // Codepage (0xDE)
        true,   // Codepage (0xDF)
        true,   // Codepage (0xE0)
        true,   // Codepage (0xE1)
        true,   // Codepage (0xE2)
        true,   // Codepage (0xE3)
        true,   // Codepage (0xE4)
        true,   // Codepage (0xE5)
        true,   // Codepage (0xE6)
        true,   // Codepage (0xE7)
        true,   // Codepage (0xE8)
        true,   // Codepage (0xE9)
        true,   // Codepage (0xEA)
        true,   // Codepage (0xEB)
        true,   // Codepage (0xEC)
        true,   // Codepage (0xED)
        true,   // Codepage (0xEE)
        true,   // Codepage (0xEF)
        true,   // Codepage (0xF0)
        true,   // Codepage (0xF1)
        true,   // Codepage (0xF2)
        true,   // Codepage (0xF3)
        true,   // Codepage (0xF4)
        true,   // Codepage (0xF5)
        true,   // Codepage (0xF6)
        true,   // Codepage (0xF7)
        true,   // Codepage (0xF8)
        true,   // Codepage (0xF9)
        true,   // Codepage (0xFA)
        true,   // Codepage (0xFB)
        true,   // Codepage (0xFC)
        true,   // Codepage (0xFD)
        true,   // Codepage (0xFE)
        true,   // Codepage (0xFF)
      };
    }

// GCC 7.1 warns about output truncation. Output truncation however is an
// explicit feature of snprintf and the reason why we are using it.
#if defined __GNUC__
#if __GNUC__ > 7 || (__GNUC__ == 7 && (__GNUC_MINOR__ > 0))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
#endif
    static std::string GetEscapedString(const std::string& input)
    {
#if defined __GNUC__
#if __GNUC__ > 7 || (__GNUC__ == 7 && (__GNUC_MINOR__ > 0))
#pragma GCC diagnostic pop
#endif
#endif
      std::string output;
      output.reserve(input.size() * 3); // Reserve enough so we never have to re-allocate

      for  (size_t i = 0; i < input.size(); i++)
      {
        if (is_reserved_[static_cast<unsigned char>(input[i])])
        {
          output += "%xx";
          snprintf(&output[output.size() - 2], 2, "%02X", static_cast<unsigned char>(input[i]));
        }
        else
        {
          output += input[i];
        }
      }

      return output;
    }

    static std::string GetUnescapedString(const std::string& input)
    {
      std::string output;
      output.reserve(input.size());

      for (size_t i = 0; i < input.size(); i++)
      {
        if(input[i] == '%')
        {
          if (i + 2 < input.size())
          {
            std::string hex_string = input.substr(i + 1, 2);
            unsigned int char_num;
            if (sscanf(hex_string.c_str(), "%X", &char_num))
            {
              output += static_cast<char>(char_num);
              i += 2;
              continue;
            }
          }
        }

        output += input[i];
      }

      return output;
    }
  }
}
