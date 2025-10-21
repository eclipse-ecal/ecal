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

#include <benchmark/benchmark.h>
#include "ecal_serialize_service.h"
#include "ecal_serialize_sample_registration.h"
#include "service_generate.h"
#include "registration_generate.h"


using namespace eCAL;

namespace
{
  eCAL::Registration::SampleList GenerateSampleList()
  {
    eCAL::Registration::SampleList list;
    list.push_back(eCAL::Registration::GenerateProcessSample());
    list.push_back(eCAL::Registration::GenerateTopicSample());
    list.push_back(eCAL::Registration::GenerateTopicSample());
    list.push_back(eCAL::Registration::GenerateTopicSample());
    return list;
  }

  struct NanopbSerialization  {
    template<typename T>
    static bool SerializeToBuffer(const T& sample_, std::string& buffer_)
    {
      return eCAL::nanopb::SerializeToBuffer(sample_, buffer_);
    }

    template<typename T>
    static bool DeserializeFromBuffer(const char* data_, size_t size_, T& target_sample_)
    {
      return eCAL::nanopb::DeserializeFromBuffer(data_, size_, target_sample_);
    }
  };

  struct ProtozeroSerialization  {
    template<typename T>
    static bool SerializeToBuffer(const T& sample_, std::string& buffer_)
    {
      return eCAL::protozero::SerializeToBuffer(sample_, buffer_);
    }

    template<typename T>
    static bool DeserializeFromBuffer(const char* data_, size_t size_, T& target_sample_)
    {
      return eCAL::protozero::DeserializeFromBuffer(data_, size_, target_sample_);
    }
  };

  // Gen is a function pointer: SerializationType Gen()
  template<class SerializationProtocol, typename SerializationType, SerializationType (*Gen)()>
  void BM_Serialize(benchmark::State& state)
  {
    std::string buffer;
    SerializationType service_request = Gen();        // generate once

    // warm up once so buffer has capacity
    SerializationProtocol::SerializeToBuffer(service_request, buffer);

    for (auto _ : state)
    {
      SerializationProtocol::SerializeToBuffer(service_request, buffer);
      benchmark::DoNotOptimize(buffer);
    }
  }

  template<class SerializationProtocol, typename SerializationType, SerializationType (*Gen)()>
  void BM_Deserialize(benchmark::State& state)
  {
    std::string buffer;
    SerializationType service_request = Gen();
    SerializationProtocol::SerializeToBuffer(service_request, buffer);

    SerializationType deserialized_request{};
    // one initial run to ensure everything is set up
    SerializationProtocol::DeserializeFromBuffer(buffer.data(), buffer.size(), deserialized_request);

    for (auto _ : state)
    {
      SerializationProtocol::DeserializeFromBuffer(buffer.data(), buffer.size(), deserialized_request);
      benchmark::DoNotOptimize(deserialized_request);
    }
  }

  template<class SerializationProtocol>
  void RegisterFamily(const char* tag)
  {
    benchmark::RegisterBenchmark(
      std::string("Serialize/ServiceRequest/") + tag,
      &BM_Serialize<SerializationProtocol, eCAL::Service::Request, &eCAL::Service::GenerateRequest>);

    benchmark::RegisterBenchmark(
      std::string("Deserialize/ServiceRequest/") + tag,
      &BM_Deserialize<SerializationProtocol, eCAL::Service::Request, &eCAL::Service::GenerateRequest>);

    benchmark::RegisterBenchmark(
      std::string("Serialize/ServiceResponse/") + tag,
      &BM_Serialize<SerializationProtocol, eCAL::Service::Response, &eCAL::Service::GenerateResponse>);

    benchmark::RegisterBenchmark(
      std::string("Deserialize/ServiceResponse/") + tag,
      &BM_Deserialize<SerializationProtocol, eCAL::Service::Response, &eCAL::Service::GenerateResponse>);

    benchmark::RegisterBenchmark(
      std::string("Serialize/RegistrationSampleList/") + tag,
      &BM_Serialize<SerializationProtocol, eCAL::Registration::SampleList, GenerateSampleList>);

    benchmark::RegisterBenchmark(
      std::string("Deserialize/RegistrationSampleList/") + tag,
      &BM_Deserialize<SerializationProtocol, eCAL::Registration::SampleList, GenerateSampleList>);
  }
}

int main(int argc, char** argv)
{
  ::benchmark::Initialize(&argc, argv);

  RegisterFamily<NanopbSerialization>("Nanopb");
  RegisterFamily<ProtozeroSerialization>("Protozero");

  ::benchmark::RunSpecifiedBenchmarks();
}