#include <ecal/measurement/hdf5/reader.h>

namespace eCAL {
  class ReaderImplementation {
  };
}

using namespace eCAL::experimental::measurement::hdf5;
using namespace eCAL::experimental::measurement;

Reader::Reader() 
{}

Reader::Reader(const std::string& path) 
{}

Reader::~Reader() = default;

Reader::Reader(Reader&&) noexcept = default;

Reader& Reader::operator=(Reader&&) noexcept = default;

bool Reader::Open(const std::string& path) 
{
  return true;
}

bool Reader::Close() 
{
  return {};
}

bool Reader::IsOk() const
{
  return {};
}

std::string Reader::GetFileVersion() const
{
  return {};
}

std::set<std::string> Reader::GetChannelNames() const
{
  return {};
}

std::set<eCAL::experimental::measurement::base::Channel> eCAL::experimental::measurement::hdf5::Reader::GetChannels() const
{
  return {};
}

std::set<eCAL::experimental::measurement::base::Channel> eCAL::experimental::measurement::hdf5::Reader::GetChannels(const std::string& channel_name) const
{
  return {};
}

bool Reader::HasChannel(const eCAL::experimental::measurement::base::Channel& channel) const
{
  return {};
}

base::DataTypeInformation Reader::GetChannelDataTypeInformation(const base::Channel& channel) const
{
  return {};
}

long long Reader::GetMinTimestamp(const base::Channel& channel) const
{
  return {};
}

long long Reader::GetMaxTimestamp(const base::Channel& channel) const
{
  return {};
}

bool Reader::GetEntriesInfo(const base::Channel& channel, base::EntryInfoSet& entries) const
{
  return {};
}

bool Reader::GetEntriesInfoRange(const base::Channel& channel, long long begin, long long end, base::EntryInfoSet& entries) const
{
  return {};
}

bool Reader::GetEntryDataSize(long long entry_id, size_t& size) const
{
  return {};
}

bool Reader::GetEntryData(long long entry_id, void* data) const
{
  return {};
}