#include <ecal/measurement/hdf5/writer.h>
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
        struct WriterImpl
        {
          eCAL::eh5::HDF5Meas measurement;

          WriterImpl() = default;
          WriterImpl(const std::string& path)
            : measurement(path, eCAL::eh5::eAccessType::CREATE)
          {}
        };
      }
    }
  }
}

Writer::Writer() 
  : impl(std::make_unique<WriterImpl>())
{}

Writer::Writer(const std::string& path) 
  : impl(std::make_unique<WriterImpl>(path))
{}

Writer::~Writer() = default;

Writer::Writer(Writer&&) noexcept = default;

Writer& Writer::operator=(Writer&&) noexcept = default;

bool Writer::Open(const std::string& path)
{
  return impl->measurement.Open(path, eCAL::eh5::eAccessType::CREATE);
}

bool Writer::Close()
{
  return impl->measurement.Close();
}

bool Writer::IsOk() const
{
  return impl->measurement.IsOk();
}

size_t Writer::GetMaxSizePerFile() const
{
  return impl->measurement.GetMaxSizePerFile();
}

void Writer::SetMaxSizePerFile(size_t size)
{
  return impl->measurement.SetMaxSizePerFile(size);
}

bool Writer::IsOneFilePerChannelEnabled() const
{
  return impl->measurement.IsOneFilePerChannelEnabled();
}

void Writer::SetOneFilePerChannelEnabled(bool enabled)
{
  return impl->measurement.SetOneFilePerChannelEnabled(enabled);
}

void Writer::SetChannelDataTypeInformation(const base::Channel& channel_name, const base::DataTypeInformation& info)
{
  impl->measurement.SetChannelDataTypeInformation(channel_name, info);
}

void Writer::SetFileBaseName(const std::string& base_name)
{
  return impl->measurement.SetFileBaseName(base_name);
}

bool Writer::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const eCAL::experimental::measurement::base::Channel& channel, long long id, long long clock)
{
  return impl->measurement.AddEntryToFile(data, size, snd_timestamp, rcv_timestamp, channel, id, clock);
}
