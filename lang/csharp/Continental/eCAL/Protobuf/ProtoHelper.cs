using Google.Protobuf;
using System;

namespace Continental
{
  namespace eCAL
  {
    namespace Common
    {
      internal static class ProtobufHelper
      {
        public static string GetProtoMessageDescription(Google.Protobuf.IMessage message)
        {
          return "";
        }

        public static string GetProtoMessageTypeName(Google.Protobuf.IMessage message)
        {
          return ("proto:" + message.Descriptor.FullName);
        }
      }
    }
  }
}