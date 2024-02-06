#include <ecal/measurement/hdf5/reader.h>
#include <ecalhdf5/eh5_meas.h>


using namespace eCAL::experimental::measurement::hdf5;
using namespace eCAL::experimental::measurement;

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

std::set<eCAL::experimental::measurement::base::Channel> eCAL::experimental::measurement::hdf5::Reader::GetChannels() const
{
  return measurement->GetChannels();
}

std::set<eCAL::experimental::measurement::base::Channel> eCAL::experimental::measurement::hdf5::Reader::GetChannels(const std::string& channel_name) const
{
  return measurement->GetChannels(channel_name);
}

bool Reader::HasChannel(const eCAL::experimental::measurement::base::Channel& channel) const
{
  return measurement->HasChannel(channel.name);
}

base::DataTypeInformation Reader::GetChannelDataTypeInformation(const base::Channel& channel) const
{
  return measurement->GetChannelDataTypeInformation(channel);
}

long long Reader::GetMinTimestamp(const base::Channel& channel) const
{
  return measurement->GetMinTimestamp(channel);
}

long long Reader::GetMaxTimestamp(const base::Channel& channel) const
{
  return measurement->GetMaxTimestamp(channel);
}

bool Reader::GetEntriesInfo(const base::Channel& channel, base::EntryInfoSet& entries) const
{
  return measurement->GetEntriesInfo(channel, entries);
}

bool Reader::GetEntriesInfoRange(const base::Channel& channel, long long begin, long long end, base::EntryInfoSet& entries) const
{
  return measurement->GetEntriesInfoRange(channel, begin, end, entries);
}

bool Reader::GetEntryDataSize(long long entry_id, size_t& size) const
{
  return measurement->GetEntryDataSize(entry_id, size);
}

bool Reader::GetEntryData(long long entry_id, void* data) const
{
  return measurement->GetEntryData(entry_id, data);
}