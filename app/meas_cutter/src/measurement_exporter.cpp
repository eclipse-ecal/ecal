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

#include "measurement_exporter.h"
#include <ecal/measurement/writer_factory.h>

MeasurementExporter::MeasurementExporter():
  _writer(eCAL::measurement::CreateWriter())
{
}

void MeasurementExporter::setPath(const std::string& path, const std::string& base_name, const size_t& max_size_per_file)
{
  _output_path = EcalUtils::Filesystem::CleanPath(path + EcalUtils::Filesystem::NativeSeparator(EcalUtils::Filesystem::OsStyle::Current) + eCALMeasCutterUtils::kDefaultFolderOutput, EcalUtils::Filesystem::OsStyle::Current);
  if (!_writer->Open(_output_path))
  {
    throw ExporterException("Unable to create HDF5 protobuf output path " + path + ".");
  }

  try
  {
    _writer->SetMaxSizePerFile(max_size_per_file);
    _writer->SetFileBaseName(base_name);
    _writer->SetOneFilePerChannelEnabled(eCALMeasCutterUtils::enable_one_file_per_topic);
  }
  catch (const std::invalid_argument&)
  {
    throw ExporterException("Key \"splitsize\" or  \"basename\" not valid!");
  }
}

MeasurementExporter::~MeasurementExporter()
{
  _writer->Close();
}

void MeasurementExporter::createChannel(const std::string& channel_name, const eCALMeasCutterUtils::ChannelInfo& channel_info)
{
  _current_channel_name = channel_name;
  std::string channel_type = channel_info.format == eCALMeasCutterUtils::SerializationFormat::PROTOBUF ? "proto:" + channel_info.type : channel_info.type;
  _writer->SetChannelMetaInformation(channel_name, channel_type, channel_info.description);
}

void MeasurementExporter::setData(eCALMeasCutterUtils::Timestamp timestamp, const eCALMeasCutterUtils::MetaData& meta_data, const std::string& payload)
{
  eCALMeasCutterUtils::MetaData::const_iterator iter;

  iter = meta_data.find(eCALMeasCutterUtils::MetaDatumKey::SENDER_TIMESTAMP);
  const auto sender_timestamp = (iter != meta_data.end()) ? iter->second.sender_timestamp : static_cast<eCALMeasCutterUtils::Timestamp>(0);

  iter = meta_data.find(eCALMeasCutterUtils::MetaDatumKey::SENDER_ID);
  const auto sender_id = (iter != meta_data.end()) ? iter->second.sender_id : static_cast<uint64_t>(0);

  iter = meta_data.find(eCALMeasCutterUtils::MetaDatumKey::SENDER_CLOCK);
  const auto sender_clock = (iter != meta_data.end()) ? iter->second.sender_clock : static_cast<uint64_t>(0);

  if (!_writer->AddEntryToFile(payload.data(), payload.size(), sender_timestamp, timestamp, _current_channel_name, sender_id, sender_clock))
  {
    throw ExporterException("Unable to export protobuf message.");
  }
}

std::string MeasurementExporter::getOutputPath()
{
  return _output_path;
}
