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
#include <iostream>

#include <tclap/CmdLine.h>

#include <ecal/ecal.h>
#include <ecal/ecal_publisher.h>

// main entry
int main(int argc, char **argv)
{
  // parse command line
  TCLAP::CmdLine cmd("datarate_snd");
  TCLAP::ValueArg<std::string> arg_topic_name       ("t", "topic_name",       "Topic name to publish.",                  false, "topic", "string");
  TCLAP::ValueArg<int>         arg_size             ("s", "size",             "Message size in MB.",                     false,       8,    "int");
  TCLAP::ValueArg<int>         arg_sleep            ("d", "delay",            "Delay between publication in ms.",        false,       1,    "int");
  TCLAP::SwitchArg             arg_zero_copy        ("z", "zero_copy",        "Zero copy mode on/off."                                           );
  TCLAP::ValueArg<int>         arg_buffer_count     ("b", "buffer_count",     "Number of memory file buffers.",          false,       1,    "int");
  TCLAP::ValueArg<int>         arg_acknowledge_time ("a", "acknowledge_time", "Handshake acknowledgement timout in ms.", false,       0,    "int");
  cmd.add(arg_topic_name);
  cmd.add(arg_size);
  cmd.add(arg_sleep);
  cmd.add(arg_zero_copy);
  cmd.add(arg_buffer_count);
  cmd.add(arg_acknowledge_time);
  cmd.parse(argc, argv);
  
  // get parameters
  const std::string topic_name(arg_topic_name.getValue());
  size_t            size(arg_size.getValue());
  const int         sleep(arg_sleep.getValue());
  const bool        zero_copy(arg_zero_copy.getValue());
  const int         buffer_count(arg_buffer_count.getValue());
  const int         acknowledge_time(arg_acknowledge_time.getValue());

  // log parameter
  std::cout << "Topic name           = " << topic_name                   << std::endl;
  std::cout << "Message size         = " << size             << " MByte" << std::endl;
  std::cout << "Sleep time           = " << sleep            << " ms"    << std::endl;
  std::cout << "Zero copy mode       = " << zero_copy                    << std::endl;
  std::cout << "Buffer count         = " << buffer_count                 << std::endl;
  std::cout << "Acknowledgement time = " << acknowledge_time << " ms"    << std::endl;

  // initialize eCAL API
  eCAL::Initialize(argc, argv, "datarate_snd");

  // create publisher config
  eCAL::Publisher::Configuration pub_config;
  // set zero copy
  pub_config.layer.shm.zero_copy_mode = zero_copy;
  // set buffering
  pub_config.layer.shm.memfile_buffer_count = buffer_count;
  // set handshake acknowledgement timeout [ms]
  pub_config.layer.shm.acknowledge_timeout_ms = acknowledge_time;

  // new publisher
  eCAL::CPublisher pub(topic_name, pub_config);

  // default send string
  size *= 1024 * 1024;
  std::string send_s = "Hello World ";
  while(send_s.size() < size)
  {
    send_s += send_s;
  }
  send_s.resize(size);

  // send updates
  while(eCAL::Ok())
  {
    // send content
    pub.Send(send_s);
    // sleep
    if(sleep > 0) std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
  }

  // destroy publisher
  pub.Destroy();

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
