# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================
"""Module that contains protobuf helper functionality.
"""
import google.protobuf.reflection
import google.protobuf.descriptor
import google.protobuf.descriptor_pb2

import google.protobuf

print("Protobuf version:", google.protobuf.__version__)

# define add protobuf description for message incl. dependencies
def add_proto_desc(file_desc_set, file_desc, inserted_files):
  """ Add the file descriptor and all its dependencies to the protobuf file descriptor set

  :param file_desc_set:  a protobuf file descriptor set
  :param file_desc:      the current file descriptor
  :param inserted_files: accumulator variable for checking whether the file has already been inserted.

  """
  # Do not add descriptors twice
  if not file_desc.name in inserted_files:
    inserted_files.add(file_desc.name)
    for dep in file_desc.dependencies:
      add_proto_desc(file_desc_set, dep, inserted_files)

    desc_proto = file_desc_set.file.add()
    file_desc.CopyToProto(desc_proto)

# define get function for protobuf message descriptor
def get_descriptor_from_type(type_) -> bytes:
  """ Returns a file descriptor set which contains all dependencies of a protobuf type that 
  can be used for reflection purposes in eCAL.

  :param type_: Python type of the generated pb2.py protobuf file.

  """
  file_desc_set = google.protobuf.descriptor_pb2.FileDescriptorSet()
  desc_file = type_.DESCRIPTOR.file
  tracing_set = set()
  add_proto_desc(file_desc_set, desc_file, tracing_set)
  return file_desc_set.SerializeToString()
  
# define get function for protobuf message descriptor
def get_type_from_descriptor(typename : str, descriptor : bytes):
  """ Returns a file descriptor set which contains all dependencies of a protobuf type that
  can be used for reflection purposes in eCAL.

  :param typename:   name of the type of the protobuf message
  :param descriptor: descriptor as stored in a measurement

  """
  file_desc_set = google.protobuf.descriptor_pb2.FileDescriptorSet()
  file_desc_set.ParseFromString(descriptor)
  desc_pool = google.protobuf.descriptor_pool.DescriptorPool()
  for file_desc_proto in file_desc_set.file:
    desc_pool.Add(file_desc_proto)
  desc = desc_pool.FindMessageTypeByName(typename)
  return google.protobuf.message_factory.MessageFactory(desc_pool).GetPrototype(desc)
