#include <ecal/measurement/hdf5/reader.h>
#include <ecalhdf5/eh5_meas.h>

using namespace eCAL::measurement::hdf5;

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

std::string Reader::GetChannelDescription(const std::string& channel_name) const
{
  return measurement->GetChannelDescription(channel_name);
}

std::string Reader::GetChannelType(const std::string& channel_name) const
{
  return measurement->GetChannelType(channel_name);
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