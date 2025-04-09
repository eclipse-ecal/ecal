/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

using System.Collections;

namespace Eclipse
{
  namespace eCAL
  {
    namespace Common
    {
      internal static class ProtobufHelper
      {

#if ProtobufReflectionSupport
        public static void AddProtoDescriptor(Google.Protobuf.Reflection.FileDescriptorSet descriptor_set, Google.Protobuf.Reflection.FileDescriptor descriptor, System.Collections.Generic.HashSet<string> inserted_files)
        {
          string name = descriptor.Name;
          if (!inserted_files.Contains(name))
          {
            inserted_files.Add(name);
            foreach (var dependency in descriptor.Dependencies)
            {
              AddProtoDescriptor(descriptor_set, dependency, inserted_files);
            }
            descriptor_set.File.Add(descriptor.ToProto());
          }
        }

        public static byte[] GetProtoMessageDescription(Google.Protobuf.IMessage message)
        {

          var descriptor_set = new Google.Protobuf.Reflection.FileDescriptorSet();
          var descriptor = message.Descriptor.File;
          var inserted_files = new System.Collections.Generic.HashSet<string>();

          AddProtoDescriptor(descriptor_set, descriptor, inserted_files);

          var memory_stream = new System.IO.MemoryStream();
          var stream = new Google.Protobuf.CodedOutputStream(memory_stream);
          descriptor_set.WriteTo(stream);
          stream.Flush();
          memory_stream.Flush();

          byte[] descriptor_array = memory_stream.ToArray();
          return descriptor_array;
        }
#else
        public static byte[] GetProtoMessageDescription(Google.Protobuf.IMessage message)
        {
          return new byte[0];
        }
#endif



        public static string GetProtoMessageTypeName(Google.Protobuf.IMessage message)
        {
          return (message.Descriptor.FullName);
        }
      }
    }
  }
}
