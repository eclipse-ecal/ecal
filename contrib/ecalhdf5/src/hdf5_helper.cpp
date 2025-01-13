/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2024 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include "hdf5_helper.h"

bool CreateStringEntryInRoot(hid_t root, const std::string& url, const std::string& dataset_content)
{
  //  create scalar dataset
  hid_t scalar_dataspace = H5Screate(H5S_SCALAR);
  //  create new string data type
  hid_t string_data_type = H5Tcopy(H5T_C_S1);
  
  //  if attribute's value length exists, allocate space for it
  if (dataset_content.length() > 0)
    H5Tset_size(string_data_type, dataset_content.length());
  //  Create creation property for data_space
  auto ds_property = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_obj_track_times(ds_property, false);
  //H5Pset_create_intermediate_group(ds_property, 1);

  //  create attribute
  hid_t data_set = H5Dcreate(root, url.c_str(), string_data_type, scalar_dataspace, H5P_DEFAULT, ds_property, H5P_DEFAULT);
  if (data_set < 0) return false;

  auto write_status = H5Dwrite(data_set, string_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, dataset_content.c_str());
  if (write_status < 0) return false;

  //  close all created stuff
  H5Dclose(data_set);
  H5Pclose(ds_property);
  H5Tclose(string_data_type);
  H5Sclose(scalar_dataspace);

  return true;
}


bool ReadStringEntryAsString(hid_t root, const std::string& url, std::string& data)
{
  //  empty attribute value
  data.clear();
  if (root < 0) return false;

  auto dataset_id = H5Dopen(root, url.c_str(), H5P_DEFAULT);
  if (dataset_id < 0) return false;

  auto size = H5Dget_storage_size(dataset_id);
  data.resize(size);

  herr_t read_status = -1;
  if (size >= 0)
  {
    hid_t string_data_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string_data_type, size);
    read_status = H5Dread(dataset_id, string_data_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<void*>(const_cast<char*>(data.data())));
  }

  H5Dclose(dataset_id);
  return (read_status >= 0);
}

//    status = H5Dread(dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
bool CreateBinaryEntryInRoot(hid_t root, const std::string& url, const std::string& dataset_content)
{

  hsize_t hs_size = static_cast<hsize_t>(dataset_content.size());
  //  Create DataSpace with rank 1 and size dimension
  auto data_space = H5Screate_simple(1, &hs_size, nullptr);
  //  Create creation property for data_space
  auto ds_property = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_obj_track_times(ds_property, false);
  //H5Pset_create_intermediate_group(ds_property, 1);
  //  Create dataset in data_space
  auto data_set = H5Dcreate(root, url.c_str(), H5T_NATIVE_UCHAR, data_space, H5P_DEFAULT, ds_property, H5P_DEFAULT);

  //  Write buffer to dataset
  herr_t write_status = H5Dwrite(data_set, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, dataset_content.c_str());

  //  Close dataset, data space, and data set property
  H5Dclose(data_set);
  H5Pclose(ds_property);
  H5Sclose(data_space);

  return (write_status >= 0);
}

bool ReadBinaryEntryAsString(hid_t root, const std::string& url, std::string& data)
{
  data.clear();

  auto dataset_id = H5Dopen(root, url.c_str(), H5P_DEFAULT);
  if (dataset_id < 0) return false;

  auto size = H5Dget_storage_size(dataset_id);
  data.resize(size);

  herr_t read_status = -1;
  if (size >= 0)
  {
    read_status = H5Dread(dataset_id, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<void*>(const_cast<char*>(data.data())));
  }

  H5Dclose(dataset_id);
  return (read_status >= 0);
}


bool CreateInformationEntryInRoot(hid_t root, const std::string& url, const eCAL::eh5::EntryInfoVect& entries)
{
  const size_t dataSetsSize = entries.size();
  hsize_t dims[2] = { dataSetsSize, 5 };
  //  Create DataSpace with rank 2 and size dimension
  auto dataSpace = H5Screate_simple(2, dims, nullptr);
  //  Create creation property for data_space
  auto dsProperty = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_obj_track_times(dsProperty, false);
  auto dataSet = H5Dcreate(root, url.c_str(), H5T_NATIVE_LLONG, dataSpace, H5P_DEFAULT, dsProperty, H5P_DEFAULT);
  if (dataSet < 0) return false;

  //  Write buffer to dataset
  herr_t writeStatus = H5Dwrite(dataSet, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, entries.data());
  if (writeStatus < 0) return false;

  //  Close dataset, data space, and data set property
  H5Dclose(dataSet);
  H5Pclose(dsProperty);
  H5Sclose(dataSpace);

  return true;
}

bool GetEntryInfoVector(hid_t root, const std::string& url, eCAL::eh5::EntryInfoSet& entries)
{
  entries.clear();
  
  auto dataset_id = H5Dopen(root, url.c_str(), H5P_DEFAULT);

  if (dataset_id < 0) return false;

  const size_t sizeof_ll = sizeof(long long);
  hsize_t data_size = H5Dget_storage_size(dataset_id) / sizeof_ll;

  if (data_size <= 0) return false;

  std::vector<long long> data(data_size);
  herr_t status = H5Dread(dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0]);
  H5Dclose(dataset_id);

  for (unsigned int index = 0; index < data_size; index += 5)
  {
    //                        rec timestamp,  channel id,       send clock,       send time stamp,  send ID
    entries.emplace(eCAL::eh5::SEntryInfo(data[index], data[index + 1], data[index + 2], data[index + 3], data[index + 4]));
  }

  return (status >= 0);
}

bool SetAttribute(hid_t id, const std::string& name, const std::string& value)
{
  if (id < 0) return false;

  if (H5Aexists(id, name.c_str()) > 0)
    H5Adelete(id, name.c_str());
  //  create scalar dataset
  hid_t scalarDataset = H5Screate(H5S_SCALAR);

  //  create new string data type
  hid_t stringDataType = H5Tcopy(H5T_C_S1);

  //  if attribute's value length exists, allocate space for it
  if (value.length() > 0)
    H5Tset_size(stringDataType, value.length());

  //  create attribute
  hid_t attribute = H5Acreate(id, name.c_str(), stringDataType, scalarDataset, H5P_DEFAULT, H5P_DEFAULT);

  if (attribute < 0) return false;

  //  write attribute value to attribute
  herr_t writeStatus = H5Awrite(attribute, stringDataType, value.c_str());
  if (writeStatus < 0) return false;

  //  close attribute
  H5Aclose(attribute);
  //  close scalar dataset
  H5Sclose(scalarDataset);
  //  close string data type
  H5Tclose(stringDataType);

  return true;
}

bool GetAttribute(hid_t id, const std::string& name, std::string& value)
{
  bool ret_val = false;
  //  empty attribute value
  value.clear();
  if (id < 0) return false;

  //  check if attribute exists
  if (H5Aexists(id, name.c_str()) != 0)
  {
    //  open attribute by name, getting the attribute index
    hid_t attr_id = H5Aopen_name(id, name.c_str());
    //  fail - attribute can not be opened
    if (attr_id <= 0) return false;

    //  get attribute type
    hid_t attr_type = H5Aget_type(attr_id);
    //  get type class based on attribute type
    H5T_class_t type_class = H5Tget_class(attr_type);
    //  get attribute content dataSize
    const size_t attr_size = H5Tget_size(attr_type);

    //  if attribute class is string
    if (type_class == H5T_STRING)
    {
      hid_t attr_type_mem = H5Tget_native_type(attr_type, H5T_DIR_ASCEND);
      //  create buffer to store the value of the attribute
      std::vector<char> content_buffer(attr_size);
      //  get attribute value
      ret_val = (H5Aread(attr_id, attr_type_mem, &content_buffer[0]) >= 0);

      //  convert value to std string
      value = std::string(&content_buffer[0], attr_size);
    }
    else
    {
      //  fail - attribute is not string type
      ret_val = false;
    }
    //  close attribute
    H5Aclose(attr_id);
  }
  else
  {
    //  fail - attribute name does not exist
    ret_val = false;
  }
  //  return read status
  return ret_val;
}

bool HasGroup(hid_t root, const std::string& path)
{
  hid_t exists = H5Lexists(root, path.c_str(), H5P_DEFAULT);
  return (exists > 0);
}

hid_t OpenOrCreateGroup(hid_t root, const std::string& name)
{
  hid_t group;
  if (HasGroup(root, name))
  {
    group = H5Gopen(root, name.c_str(), H5P_DEFAULT);
  }
  else 
  {
    group = H5Gcreate(root, name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }

  return group;
}

std::vector<std::string> ListSubgroups(hid_t id)
{
  std::vector<std::string> group_vector;
  auto iterate_lambda = [](hid_t /*group*/, const char* name, const H5L_info_t* /*info*/, void* op_data)->herr_t
  {
    auto vec = static_cast<std::vector<std::string>*>(op_data);
    vec->push_back(name);
    return 0;
  };

  H5Literate(id, H5_INDEX_NAME, H5_ITER_INC, nullptr, iterate_lambda, (void*)&group_vector);
  return group_vector;
}