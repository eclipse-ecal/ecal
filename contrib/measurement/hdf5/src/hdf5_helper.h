#pragma once

#include <string>
#include <iomanip>
#include <sstream>

#include <hdf5.h>

#include <ecal/measurement/base/types.h>

//!< @cond
const std::string kChnNameAttribTitle("Channel Name");
const std::string kChnDescAttrTitle("Channel Description");
const std::string kChnTypeAttrTitle("Channel Type");
const std::string kChnIdTypename("TypeName");
const std::string kChnIdEncoding("TypeEncoding");
const std::string kChnIdDescriptor("TypeDescriptor");
const std::string kChnIdData("DataTable");
const std::string kFileVerAttrTitle("Version");
const std::string kTimestampAttrTitle("Timestamps");
const std::string kChnAttrTitle("Channels");

hid_t CreateFile(const std::string& filename, const std::string& file_version);
void CloseFile(hid_t root);

struct BinaryEntry {
  void* address;
  uint64_t size;
};

struct ChannelSummary
{
  eCAL::experimental::measurement::base::DataTypeInformation datatype_info;
  eCAL::experimental::measurement::base::EntryInfoVect       entry_info;
};

void WriteBinaryDataset(hid_t root, uint64_t counter, const BinaryEntry& entry);

bool CreateStringEntryInRoot(hid_t root, const std::string& url, const std::string& dataset_content);
bool ReadStringEntryAsString(hid_t root, const std::string& url, std::string& data);

bool CreateBinaryEntryInRoot(hid_t root, const std::string& url, const std::string& dataset_content);
bool ReadBinaryEntryAsString(hid_t root, const std::string& url, std::string& data);

bool CreateInformationEntryInRoot(hid_t root, const std::string& url, const eCAL::experimental::measurement::base::EntryInfoVect& entries);
bool GetEntryInfoVector(hid_t root, const std::string& url, eCAL::experimental::measurement::base::EntryInfoSet& entries);

bool SetAttribute(hid_t id, const std::string& name, const std::string& value);
bool GetAttribute(hid_t id, const std::string& name, std::string& value);

bool HasGroup(hid_t root, const std::string& path);
std::vector<std::string> ListSubgroups(hid_t root);
hid_t OpenOrCreateGroup(hid_t root, const std::string& name);


inline std::string printHex(eCAL::experimental::measurement::base::Channel::id_t id)
{
  std::stringstream ss;
  ss << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << id;
  return ss.str();
}

inline eCAL::experimental::measurement::base::Channel::id_t parseHexID(std::string string_id)
{
  auto unsigned_value = std::stoull(string_id, 0, 16);
  return static_cast<eCAL::experimental::measurement::base::Channel::id_t>(unsigned_value);
}

void CreateChannelNamesAttribute(hid_t root, const std::set<std::string>& channel_names);

namespace v6
{
  inline std::string GetUrl(const std::string& channel_name_, const std::string& channel_id, const std::string& attribute)
  {
    return "/" + channel_name_ + "/" + channel_id + "/" + attribute;
  }

  void WriteChannelSummary(hid_t root, const eCAL::experimental::measurement::base::Channel& channel, const ChannelSummary& summary);
}

namespace v5
{

}