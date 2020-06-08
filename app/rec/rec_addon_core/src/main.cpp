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

#include "recorder.h"
#include "request_handler.h"
#include "io_stream_server.h"
#include "function_descriptors.h"

using namespace eCAL::rec::addon;

int main()
{
  eCAL::rec::addon::Recorder ecal_rec_addon(*recorder_impl);

  RequestHandler request_handler;

  request_handler.AddFunctionCallback(
    function_descriptor::initialize,
    [&ecal_rec_addon](const Request&) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.Initialize() ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::deinitialize,
    [&ecal_rec_addon](const Request&) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.Deinitialize() ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::start_recording,
    [&ecal_rec_addon](const Request& request) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.StartRecording(request.parameters.at("id").GetIntegerValue(), request.parameters.at("path").GetStringValue()) ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::stop_recording,
    [&ecal_rec_addon](const Request& request) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.StopRecording(request.parameters.at("id").GetIntegerValue()) ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::enable_prebuffering,
    [&ecal_rec_addon](const Request& ) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.EnablePrebuffering() ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::disable_prebuffering,
    [&ecal_rec_addon](const Request& ) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.DisablePrebuffering() ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::set_prebuffer_length,
    [&ecal_rec_addon](const Request& request) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.SetPrebufferLength(std::chrono::milliseconds(request.parameters.at("duration").GetIntegerValue())) ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::save_prebuffer,
    [&ecal_rec_addon](const Request& request) -> Response
  {
    Response response;

    response.status = ecal_rec_addon.SavePrebuffer(request.parameters.at("id").GetIntegerValue(), request.parameters.at("path").GetStringValue()) ? Response::Status::Ok : Response::Status::Failed;
    response.status_message = ecal_rec_addon.GetLastStatus();

    return response;
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::info,
    [&ecal_rec_addon](const Request&) -> Response {

    auto info = ecal_rec_addon.GetInfo();
    return Response{
      Response::Status::Ok,
      "",
      {
      {
        { "id",  info.id },
        { "name", info.name },
        { "description", info.description }
      }
      }
    };
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::prebuffer_count,
    [&ecal_rec_addon](const Request&) -> Response {

    auto count = ecal_rec_addon.GetPrebufferFrameCount();
    return Response{
      Response::Status::Ok,
      "",
      {
        {
          { "frame_count",  static_cast<std::int64_t>(count) }
        }
      }
    };
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::api_version,
    [](const Request&) -> Response {

    return Response{
      Response::Status::Ok,
      "",
      {
        {
          { "version",  Variant(static_cast<std::int64_t>(1)) }
        }
      }
    };
  }
  );

  request_handler.AddFunctionCallback(
    function_descriptor::job_statuses,
    [&ecal_rec_addon](const Request&) -> Response {

    auto job_statuses = ecal_rec_addon.GetJobStatuses();
    Response response;

    static const std::unordered_map<int, std::string> job_states_map
    {
      { static_cast<int>(JobStatus::State::NotStarted), "not started" },
      { static_cast<int>(JobStatus::State::Recording), "recording" },
      { static_cast<int>(JobStatus::State::Flushing), "flushing" },
      { static_cast<int>(JobStatus::State::Finished), "finished" }
    };

    for (const auto& job_status: job_statuses)
    {
      std::unordered_map<std::string, Variant> result;

      result.insert(std::make_pair("id", Variant(std::int64_t(job_status.first))));
      result.insert(std::make_pair("state", Variant(job_states_map.at(static_cast<int>(job_status.second.state)))));
      result.insert(std::make_pair("healthy", Variant(job_status.second.healthy)));
      result.insert(std::make_pair("status_description", Variant(job_status.second.description)));
      result.insert(std::make_pair("frame_count", Variant(static_cast<std::int64_t>(job_status.second.frame_count))));
      result.insert(std::make_pair("queue_count", Variant(static_cast<std::int64_t>(job_status.second.queue_count))));

      response.results.push_back(result);
    }
    response.status = Response::Status::Ok;

    return response;
  }
  );

  IOStreamServer server(std::cin, std::cout);
  server.SetCallback([&request_handler](const std::string& request_line) -> std::vector<std::string> {
    std::vector<std::string> response_lines;
    if (!request_handler.Query(request_line, response_lines))
    {
      response_lines.clear();
      request_handler.EncodeResponse(response_lines, Response{ Response::Status::Syntax_Error, request_handler.GetErrorString(), {} });
    }

    return response_lines;
  });

  server.Listen();

  return 0;
}
