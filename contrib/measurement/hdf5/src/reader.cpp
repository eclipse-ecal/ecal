#include <ecal/measurement/hdf5/reader.h>
#include <ecalhdf5/eh5_meas.h>


using namespace eCAL::experimental::measurement::hdf5;
using namespace eCAL::experimental::measurement;

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace hdf5
      {
        struct ReaderImpl
        {
          eCAL::eh5::HDF5Meas measurement;

          ReaderImpl() = default;
          ReaderImpl(const std::string& path)
            : measurement(path, eCAL::eh5::eAccessType::RDONLY)
          {}
        };
      }
    }
  }
}

Reader::Reader() 
  : impl(std::make_unique<ReaderImpl>())
{}

Reader::Reader(const std::string& path) 
  : impl(std::make_unique<ReaderImpl>(path))
{}

Reader::~Reader() = default;

Reader::Reader(Reader&&) noexcept = default;

Reader& Reader::operator=(Reader&&) noexcept = default;

bool Reader::Open(const std::string& path) 
{
  return impl->measurement.Open(path, eCAL::eh5::eAccessType::RDONLY);
}

bool Reader::Close() 
{
  return impl->measurement.Close();
}

bool Reader::IsOk() const
{
  return impl->measurement.IsOk();
}

std::string Reader::GetFileVersion() const
{
  return impl->measurement.GetFileVersion();
}

/*
std::set<std::string> Reader::GetChannelNames() const
{
  return impl->measurement.GetChannelNames();
}*/

std::set<eCAL::experimental::measurement::base::Channel> eCAL::experimental::measurement::hdf5::Reader::GetChannels() const
{
  return impl->measurement.GetChannels();
}

/*
std::set<eCAL::experimental::measurement::base::Channel> eCAL::experimental::measurement::hdf5::Reader::GetChannels(const std::string& channel_name) const
{
  return impl->measurement.GetChannels(channel_name);
}*/

bool Reader::HasChannel(const eCAL::experimental::measurement::base::Channel& channel) const
{
  return impl->measurement.HasChannel(channel);
}

base::DataTypeInformation Reader::GetChannelDataTypeInformation(const base::Channel& channel) const
{
  return impl->measurement.GetChannelDataTypeInformation(channel);
}

long long Reader::GetMinTimestamp(const base::Channel& channel) const
{
  return impl->measurement.GetMinTimestamp(channel);
}

long long Reader::GetMaxTimestamp(const base::Channel& channel) const
{
  return impl->measurement.GetMaxTimestamp(channel);
}

bool Reader::GetEntriesInfo(const base::Channel& channel, base::EntryInfoSet& entries) const
{
  return impl->measurement.GetEntriesInfo(channel, entries);
}

bool Reader::GetEntriesInfoRange(const base::Channel& channel, long long begin, long long end, base::EntryInfoSet& entries) const
{
  return impl->measurement.GetEntriesInfoRange(channel, begin, end, entries);
}

bool Reader::GetEntryDataSize(long long entry_id, size_t& size) const
{
  return impl->measurement.GetEntryDataSize(entry_id, size);
}

bool Reader::GetEntryData(long long entry_id, void* data) const
{
  return impl->measurement.GetEntryData(entry_id, data);
}