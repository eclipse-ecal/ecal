using System.Collections;

namespace Continental
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
          return ("proto:" + message.Descriptor.FullName);
        }
      }
    }
  }
}
