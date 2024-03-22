#include <writer_splitting.h>

using namespace  eCAL::experimental::measurement::base;

eCAL::experimental::measurement::base::SizeSplittingWriter::SizeSplittingWriter(uint64_t splitting_size_, WriterCreator writer_creator_, const FileOptions& options_)
  : splitting_size(splitting_size_)
  , bytes_written(0)
  , file_number(0)
  , writer_creator(writer_creator_)
  , writer_options(options_)
{
  current_writer = std::move(writer_creator(GetNewWriterOptions()));
}

void eCAL::experimental::measurement::base::SizeSplittingWriter::SetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel_name, const DataTypeInformation& info)
{
  data_type_information[channel_name] = info;
  current_writer->SetChannelDataTypeInformation(channel_name, info);
}

void eCAL::experimental::measurement::base::SizeSplittingWriter::AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const WriterEntry& entry)
{
  if (bytes_written + entry.size > splitting_size)
  {
    ++file_number;
    current_writer = std::move(writer_creator(GetNewWriterOptions()));
    bytes_written = 0;

    for (const auto& info : data_type_information)
    {
      current_writer->SetChannelDataTypeInformation(info.first, info.second);
    }
  }

  current_writer->AddEntryToFile(channel, entry);
  bytes_written += entry.size;
}

WriterCreator eCAL::experimental::measurement::base::SizeSplittingWriter::GetCreator(WriterCreator creator_, const SizeSplittingOptions& size_splitting_)
{
  if (size_splitting_.has_value())
  {
    return [creator_, size_splitting_](const FileOptions& options) {
      return std::make_unique<SizeSplittingWriter>(size_splitting_.value(), creator_, options);
    };
  }
  else
  {
    return creator_;
  }
}

FileOptions eCAL::experimental::measurement::base::SizeSplittingWriter::GetNewWriterOptions() const
{
  FileOptions new_options = writer_options;
  new_options.base_name = new_options.base_name + std::to_string(file_number);
  return new_options;
}



eCAL::experimental::measurement::base::ChannelSplittingWriter::ChannelSplittingWriter(WriterCreator writer_creator_, const FileOptions& options_)
  : writer_creator(writer_creator_)
  , writer_options(options_)
{}

void eCAL::experimental::measurement::base::ChannelSplittingWriter::SetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel, const DataTypeInformation& info)
{
  auto& writer = FindOrCreateWriter(channel.name);
  writer->SetChannelDataTypeInformation(channel, info);
}

void eCAL::experimental::measurement::base::ChannelSplittingWriter::AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const WriterEntry& entry)
{
  auto& writer = FindOrCreateWriter(channel.name);
  writer->AddEntryToFile(channel, entry);
}

WriterCreator eCAL::experimental::measurement::base::ChannelSplittingWriter::GetCreator(WriterCreator creator_, const ChannelSplittingOptions& channel_splitting_)
{
  if (channel_splitting_)
  {
    return [creator_](const FileOptions& options) {
      return std::make_unique<ChannelSplittingWriter>(creator_, options);
    };
  }
  else
  {
    return creator_;
  }
}

FileOptions eCAL::experimental::measurement::base::ChannelSplittingWriter::GetNewWriterOptions(const std::string& channel_name) const
{
  FileOptions new_options = writer_options;
  new_options.base_name = new_options.base_name + channel_name;
  return  new_options;
}

std::unique_ptr<Writer>& eCAL::experimental::measurement::base::ChannelSplittingWriter::FindOrCreateWriter(const std::string& channel_name)
{
  auto writer_ = writers.find(channel_name);
  if (writer_ == writers.end())
  {
    writers.emplace(channel_name, writer_creator(GetNewWriterOptions(channel_name)));
  }
  return writers[channel_name];
}
