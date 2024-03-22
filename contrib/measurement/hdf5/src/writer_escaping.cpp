#include "writer_escaping.h"
#include <escape.h>

using namespace eCAL::experimental::measurement::base;

eCAL::experimental::measurement::base::EscapingWriter::EscapingWriter(WriterCreator writer_creator_, const FileOptions& options_)
{
  auto escaped_options_ = options_;
  escaped_options_.base_name = GetEscapedFilename(options_.base_name);
  writer = writer_creator_(options_);
}

void eCAL::experimental::measurement::base::EscapingWriter::SetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel, const DataTypeInformation& info)
{
  Channel escaped_channel{ eCAL::GetEscapedTopicname(channel.name), channel.id };
  writer->SetChannelDataTypeInformation(escaped_channel, info);
}

void eCAL::experimental::measurement::base::EscapingWriter::AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const WriterEntry& entry)
{
  base::Channel modified_channel{ channel };
  modified_channel.name = eCAL::GetEscapedTopicname(channel.name);
  writer->AddEntryToFile(modified_channel, entry);
}

WriterCreator eCAL::experimental::measurement::base::EscapingWriter::GetCreator(WriterCreator creator_)
{
  return [creator_](const FileOptions& options_)
  {
    return std::make_unique<EscapingWriter>(creator_, options_);
  };
}
