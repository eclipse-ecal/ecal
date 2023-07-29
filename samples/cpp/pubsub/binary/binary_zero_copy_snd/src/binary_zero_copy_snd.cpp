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

#include <ecal/ecal.h>

#include <cstdint>
#include <iostream>

#pragma pack(push, 1)
struct SSimpleStruct
{
  uint32_t version      = 1;
  uint16_t rows         = 5;
  uint16_t cols         = 3;
  uint32_t clock        = 0;
  uint8_t  bytes[5 * 3] = { 0 };
};
#pragma pack(pop)

std::ostream& operator<<(std::ostream& os, const SSimpleStruct& s)
{
  os << "Version : " << s.version << std::endl;
  os << "Rows    : " << s.rows    << std::endl;
  os << "Cols    : " << s.cols    << std::endl;
  os << "Clock   : " << s.clock   << std::endl;

  os << "Bytes   : " << std::endl;
  for (int i = 0; i < s.rows; ++i) {
    for (int j = 0; j < s.cols; ++j) {
      os << static_cast<int>(s.bytes[i * s.cols + j]) << " ";
    }
    os << std::endl;
  }
  return os;
}

// a binary payload object for sending a SSimpleStruct
class CStructPayload : public eCAL::CPayloadWriter
{
public:
  // write a newly initialized and updated SSimpleStruct to the shared memory
  bool Write(void* buf_, size_t len_) override
  {
    // recheck available size
    if (len_ < GetSize()) return false;

    // (re)create the complete struct
    SSimpleStruct simple_struct;
    UpdateStruct(&simple_struct);

    // copy complete struct into the memory
    memcpy(buf_, &simple_struct, GetSize());

    // log it
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Write SSimpleStruct :"                << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << simple_struct << std::endl;

    return true;
  };

  // just update the SSimpleStruct in the shared memory
  bool Update(void* buf_, size_t len_) override
  {
    // recheck available size
    if (len_ < GetSize()) return false;

    // update the struct in memory
    UpdateStruct(static_cast<SSimpleStruct*>(buf_));

    // log it
    std::cout << "------------------------------------" << std::endl;
    std::cout << "Update SSimpleStruct :"               << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << *static_cast<SSimpleStruct*>(buf_) << std::endl;

    return true;
  };

  size_t GetSize() override { return sizeof(SSimpleStruct); };

private:
  void UpdateStruct(SSimpleStruct* simple_struct)
  {
    // modify the simple_struct
    simple_struct->clock = clock;
    for (auto i = 0; i < (simple_struct->rows * simple_struct->cols); ++i)
    {
      simple_struct->bytes[i] = static_cast<char>(simple_struct->clock);
    }

    // increase internal state clock
    clock++;
  };

  uint32_t clock = 0;
};

int main(int argc, char** argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "binary_zero_copy_snd");

  // publisher for topic "number"
  eCAL::CPublisher pub("simple_struct", { "custom", "SSimpleStruct", "" });

  // turn zero copy mode on
  pub.ShmEnableZeroCopy(true);

  // create the simple struct payload
  CStructPayload struct_payload;

  // send updates every 100 ms
  while (eCAL::Ok())
  {
    pub.Send(struct_payload);
    eCAL::Process::SleepMS(100);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
