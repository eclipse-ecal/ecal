#include <ecal/measurement/hdf5/writer.h>
#include <ecalhdf5/eh5_meas.h>

using namespace eCAL::measurement::hdf5;

Writer::Writer() 
  : measurement(std::make_unique<eh5::HDF5Meas>()) 
{}

Writer::Writer(const std::string& path) 
  : measurement(std::make_unique<eh5::HDF5Meas>(path, eh5::eAccessType::CREATE))
{}

Writer::~Writer() = default;

Writer::Writer(Writer&&) noexcept = default;

Writer& Writer::operator=(Writer&&) noexcept = default;

bool Writer::Open(const std::string& path)
{
  return measurement->Open(path, eh5::eAccessType::CREATE);
}

bool Writer::Close()
{
  return measurement->Close();
}

bool Writer::IsOk() const
{
  return measurement->IsOk();
}

size_t Writer::GetMaxSizePerFile() const
{
  return measurement->GetMaxSizePerFile();
}

void Writer::SetMaxSizePerFile(size_t size)
{
  return measurement->SetMaxSizePerFile(size);
}

bool Writer::IsOneFilePerChannelEnabled() const
{
  return measurement->IsOneFilePerChannelEnabled();
}

void Writer::SetOneFilePerChannelEnabled(bool enabled)
{
  return measurement->SetOneFilePerChannelEnabled(enabled);
}

void Writer::SetChannelDescription(const std::string& channel_name, const std::string& description)
{
  return measurement->SetChannelDescription(channel_name, description);
}

void Writer::SetChannelType(const std::string& channel_name, const std::string& type)
{
  return measurement->SetChannelType(channel_name, type);
}

void Writer::SetFileBaseName(const std::string& base_name)
{
  return measurement->SetFileBaseName(base_name);
}

bool Writer::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock)
{
  return measurement->AddEntryToFile(data, size, snd_timestamp, rcv_timestamp, channel_name, id, clock);
}
