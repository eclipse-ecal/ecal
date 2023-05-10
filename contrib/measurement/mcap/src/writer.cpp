#include <ecal/measurement/mcap/writer.h>
#include <optional>
#include <limits>
#include <unordered_map>

#define MCAP_IMPLEMENTATION
#include <mcap/writer.hpp>

namespace {
  const std::string writer_id{ "ecal_mcap" };
}

// This class is not thread safe!
class BufferedWriter
{
public:
  BufferedWriter(std::shared_ptr<::mcap::McapWriter> writer_) :
    writer(writer_)
  {}

  bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock)
  {
    auto it = channel_id_mapping.find(channel_name);
    if (it != channel_id_mapping.end())
    {
      ::mcap::Message msg;
      msg.logTime = rcv_timestamp * 1000;
      msg.publishTime = snd_timestamp * 1000;
      msg.channelId = it->second;
      msg.data = reinterpret_cast<const std::byte*>(data);
      msg.dataSize = size;
      writer->write(msg);
      return true;
    }
    else
    {
      return false;
    }
  }

  void SetChannelMetaInformation(const std::string& channel_name, const std::string& channel_type, const std::string& channel_descriptor)
  {
    // insert info only once!
    if (channel_id_mapping.find(channel_name) == channel_id_mapping.end())
    {
      // split type at ":" -> this is quite a bit incorrect, we need to review
      auto topic_type = channel_type.substr(channel_type.find_first_of(':') + 1, channel_type.size());
      auto schema = mcap::Schema(topic_type, "protobuf", channel_descriptor);
      writer->addSchema(schema);
      auto channel = mcap::Channel(channel_name, "protobuf", schema.id);
      writer->addChannel(channel);
      channel_id_mapping[channel_name] = channel.id;
    }
  }

private:
    std::shared_ptr<::mcap::McapWriter> writer;
    std::unordered_map<std::string, ::mcap::ChannelId> channel_id_mapping;
};


using SizeSplittingStrategy = std::optional<uint32_t>;

  enum class ChannelSplittingStrategy
  {
    NoSplitting = 0,
    OneChannelPerFile = 1
  };

  struct WriterConfigurationOptions
  {
    SizeSplittingStrategy size_splitting_strategy = 512;
    ChannelSplittingStrategy channel_splitting_stategy = ChannelSplittingStrategy::NoSplitting;
  };


  class WriterImplementation
  {
  public:
    std::unique_ptr<BufferedWriter> writer;
    std::shared_ptr<::mcap::McapWriter> mcap_writer;
    WriterConfigurationOptions options;
    std::string base_filename;
  };



eCAL::measurement::mcap::Writer::Writer() :
  implementation(std::make_unique<WriterImplementation>())
{}

  /**
   * @brief Constructor
  **/
eCAL::measurement::mcap::Writer::Writer(const std::string& path)
  : implementation(std::make_unique<WriterImplementation>())
{
  Open(path);
}

bool eCAL::measurement::mcap::Writer::Open(const std::string& path) { 
  ::mcap::McapWriterOptions options(writer_id);
  options.enableDataCRC = false;
  options.compression = ::mcap::Compression::Lz4;
  options.compressionLevel = ::mcap::CompressionLevel::Default;
  implementation->mcap_writer = std::make_shared<::mcap::McapWriter>();
  implementation->mcap_writer->open(path + implementation->base_filename + "meas.mcap", options);
  implementation->writer = std::make_unique<BufferedWriter>(implementation->mcap_writer);
  return true;
}

bool eCAL::measurement::mcap::Writer::Close(){ 
  implementation->writer.reset();
  implementation->mcap_writer->close();
  implementation->mcap_writer.reset();
  return true; 
}

bool eCAL::measurement::mcap::Writer::IsOk() const { return true; }

size_t eCAL::measurement::mcap::Writer::GetMaxSizePerFile() const {
  return implementation->options.size_splitting_strategy.value_or(std::numeric_limits<size_t>::max());
}

void eCAL::measurement::mcap::Writer::SetMaxSizePerFile(size_t size) {
  implementation->options.size_splitting_strategy = size;
}

bool eCAL::measurement::mcap::Writer::IsOneFilePerChannelEnabled() const {
  return implementation->options.channel_splitting_stategy == ChannelSplittingStrategy::OneChannelPerFile;
}

void eCAL::measurement::mcap::Writer::SetOneFilePerChannelEnabled(bool enabled) {
  if (enabled)
    implementation->options.channel_splitting_stategy = ChannelSplittingStrategy::OneChannelPerFile;
  else
    implementation->options.channel_splitting_stategy = ChannelSplittingStrategy::NoSplitting;
}

// we need this info atomically. E.g. in Mcap this is a schma and it has both information
void eCAL::measurement::mcap::Writer::SetChannelMetaInformation(const std::string& channel_name, const std::string& channel_type, const std::string& channel_descriptor) {
  if (implementation->writer != nullptr)
  {
    implementation->writer->SetChannelMetaInformation(channel_name, channel_type, channel_descriptor);
  }
}

// This is something which a generic class should handle for both mcap and hdft
void eCAL::measurement::mcap::Writer::SetFileBaseName(const std::string& base_name) {
  implementation->base_filename = base_name;
}

bool eCAL::measurement::mcap::Writer::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock) { 
  if (implementation->writer != nullptr)
  {
    implementation->writer->AddEntryToFile(data, size, snd_timestamp, rcv_timestamp, channel_name, id, clock);
  }
  else
  {
    return false;
  }
}

