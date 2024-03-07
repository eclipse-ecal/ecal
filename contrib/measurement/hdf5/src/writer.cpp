#include <ecal/measurement/hdf5/writer.h>

#include <writer_escaping.h>
#include <writer_splitting.h>
#include <writer_hdf5_v5.h>
#include <writer_hdf5_v6.h>


using namespace eCAL::experimental::measurement;

std::unique_ptr<base::Writer> hdf5::CreateWriter(const base::WriterOptions& writer_options_)
{
  // at some point we have to do some kind of error checking, regarding folder etc. but this comes later
  auto hdf5_creator = hdf5::V6Writer::GetCreator();
  auto splitting_size_creator = base::SizeSplittingWriter::GetCreator(hdf5_creator, writer_options_.size_splitting);
  auto channel_splitting_creator = base::ChannelSplittingWriter::GetCreator(splitting_size_creator, writer_options_.channel_splitting);
  auto escaping_creator = base::EscapingWriter::GetCreator(channel_splitting_creator);
  return escaping_creator(writer_options_.file_options);
}
