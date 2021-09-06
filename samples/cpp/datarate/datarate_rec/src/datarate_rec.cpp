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

#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>

#include <tclap/CmdLine.h>

#include <ecal/ecal.h>
#include <ecal/msg/string/subscriber.h>

std::vector<char> rec_buffer;

// subscriber callback function
void OnReceive(const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_)
{
  // make a memcpy to emulate user action
  rec_buffer.reserve(data_->size);
  std::memcpy(rec_buffer.data(), data_->buf, data_->size);
}

// main entry
int main(int argc, char** argv)
{
  // parse command line
  TCLAP::CmdLine cmd("datarate_rec");
  TCLAP::ValueArg<std::string> arg_topic_name("t", "topic_name", "Topic name to subscribe.", false, "topic", "string");
  cmd.add(arg_topic_name);
  cmd.parse(argc, argv);

  // get parameters
  std::string topic_name(arg_topic_name.getValue());

  // log parameter
  std::cout << "Topic name = " << topic_name << std::endl;

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "datarate_rec");

  // new subscriber
  eCAL::CSubscriber sub(topic_name);

  // add callback
  sub.AddReceiveCallback(std::bind(OnReceive, std::placeholders::_1, std::placeholders::_2));

  // idle main thread
  while (eCAL::Ok())
  {
    // sleep 100 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // destroy publisher
  sub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
