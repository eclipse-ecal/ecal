#include <ecal/measurement/hdf5/reader.h>
#include <ecalhdf5/eh5_meas.h>


using namespace eCAL::experimental::measurement::hdf5;
using namespace eCAL::experimental::measurement;

namespace
{
  // To be removed soon-ish!
  std::pair<std::string, std::string> SplitCombinedTopicType(const std::string& combined_topic_type_)
  {
    auto pos = combined_topic_type_.find(':');
    if (pos == std::string::npos)
    {
      std::string encoding;
      std::string type{ combined_topic_type_ };
      return std::make_pair(encoding, type);
    }
    else
    {
      std::string encoding = combined_topic_type_.substr(0, pos);
      std::string type = combined_topic_type_.substr(pos + 1);
      return std::make_pair(encoding, type);
    }
  }
}

Reader::Reader() 
  : measurement(std::make_unique<eh5::HDF5Meas>()) 
{}

Reader::Reader(const std::string& path) 
  : measurement(std::make_unique<eh5::HDF5Meas>(path, eh5::eAccessType::RDONLY))
{}

Reader::~Reader() = default;

Reader::Reader(Reader&&) noexcept = default;

Reader& Reader::operator=(Reader&&) noexcept = default;

bool Reader::Open(const std::string& path) 
{
  return measurement->Open(path, eh5::eAccessType::RDONLY);
}

bool Reader::Close() 
{
  return measurement->Close(); 
}

bool Reader::IsOk() const
{
  return measurement->IsOk();
}

std::string Reader::GetFileVersion() const
{
  return measurement->GetFileVersion();
}

std::set<std::string> Reader::GetChannelNames() const
{
  return measurement->GetChannelNames();
}

bool Reader::HasChannel(const std::string& channel_name) const
{
  return measurement->HasChannel(channel_name);
}

base::DataTypeInformation Reader::GetChannelDataTypeInformation(const std::string& channel_name) const
{
  base::DataTypeInformation info;
  auto type = measurement->GetChannelType(channel_name);
  auto split_types = SplitCombinedTopicType(type);
  info.encoding = split_types.first;
  info.name = split_types.second;
  info.descriptor = measurement->GetChannelDescription(channel_name);
  return info;
}

long long Reader::GetMinTimestamp(const std::string& channel_name) const
{
  return measurement->GetMinTimestamp(channel_name);
}

long long Reader::GetMaxTimestamp(const std::string& channel_name) const
{
  return measurement->GetMaxTimestamp(channel_name);
}

bool Reader::GetEntriesInfo(const std::string& channel_name, base::EntryInfoSet& entries) const
{
  return measurement->GetEntriesInfo(channel_name, entries);
}

bool Reader::GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, base::EntryInfoSet& entries) const
{
  return measurement->GetEntriesInfoRange(channel_name, begin, end, entries);
}

bool Reader::GetEntryDataSize(long long entry_id, size_t& size) const
{
  return measurement->GetEntryDataSize(entry_id, size);
}

bool Reader::GetEntryData(long long entry_id, void* data) const
{
  return measurement->GetEntryData(entry_id, data);
}