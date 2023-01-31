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
#include <ecal/msg/protobuf/client.h>
#include <ecal/msg/protobuf/subscriber.h>
#include "tclap/CmdLine.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/app/pb/play/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex m;
std::condition_variable cv;
std::string lastCallbackTopic;
std::string pauseName;   // receiving on this channel should pause playback
std::string triggerName; // receiving on this channel should start playback
bool received = false;

void callback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* /*data_*/)
{
    std::string topicName = topic_name_;
    if (topicName == triggerName || topicName == pauseName)
    {
        std::lock_guard<std::mutex> lk(m);
        lastCallbackTopic = topic_name_;
        received = true;
        cv.notify_all();
    }
}

// main entry
int main(int argc, char** argv)
{
    bool resimPauseMode = false;
    try
    {
        TCLAP::CmdLine cmd("Ecal stepper", ' ', "0.1");
        TCLAP::SwitchArg pauseModeSwitch(
            "r",
            "resim-pause",
            "If this switch is on ecal stepper subscribes to the given pause channel, and pauses "
            "the player when a message is received. If this switch is off, ecal stepper uses the "
            "given pause channel as a step-channel is ecal player.",
            cmd,
            false);
        TCLAP::ValueArg<std::string> trigArg(
            "t",
            "trigger",
            "When a message is received on the given channel, playback is started. ",
            true,
            "",
            "ecal channel name");
        TCLAP::ValueArg<std::string> pauseArg(
            "p", "pause", "Channel used to pause playback. ", true, "", "ecal channel name");
        cmd.add(trigArg);
        cmd.add(pauseArg);
        cmd.parse(argc, argv);
        pauseName = pauseArg.getValue();
        triggerName = trigArg.getValue();
        resimPauseMode = pauseModeSwitch.getValue();
    }
    catch (TCLAP::ArgException& e) // catch exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    if (pauseName == "" || triggerName == "")
    {
        std::cerr << "error: You must set both 'trigger' and 'pause' arguments.\nExample usage: "
                     "ecal_stepper -p LSM_VEDODO -t emfEventEMFfinished_resim"
                  << std::endl;
    }

    // initialize eCAL API
    eCAL::Initialize(argc, argv, "ecal stepper", eCAL::Init::Default);

    // create player service client
    eCAL::protobuf::CServiceClient<eCAL::pb::play::EcalPlayService> player_service;

    // sleep for service matching
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    eCAL::CSubscriber triggerSubscriber(triggerName);
    triggerSubscriber.AddReceiveCallback(
        std::bind(callback, std::placeholders::_1, std::placeholders::_2));
    eCAL::CSubscriber pauseSubscriber(pauseName);

    eCAL::pb::play::CommandRequest command_request;

    if(resimPauseMode)
    {
        pauseSubscriber.AddReceiveCallback(
            std::bind(callback, std::placeholders::_1, std::placeholders::_2));
    }

    while (eCAL::Ok())
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [] { return received; });
        received = false;
        command_request.Clear();
        if (lastCallbackTopic == triggerName)
        {
            if(resimPauseMode)
            {
                command_request.set_command(eCAL::pb::play::CommandRequest::play);
            }
            else
            {
                command_request.set_command(eCAL::pb::play::CommandRequest::step_channel);
                command_request.set_step_reference_channel(pauseName);
            }
            player_service.Call("SetCommand", command_request);
        }
        else if (lastCallbackTopic == pauseName)
        {
            command_request.set_command(eCAL::pb::play::CommandRequest::pause);
            player_service.Call("SetCommand", command_request);
        }
    }
    // finalize eCAL API
    eCAL::Finalize();

    return (0);
}
