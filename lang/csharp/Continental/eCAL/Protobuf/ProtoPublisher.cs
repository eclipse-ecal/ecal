using System;
using System.Text;
using System.IO;
using Google.Protobuf;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief eCAL class to publish protobuf Data.
      **/
      public class ProtobufPublisher<T> where T : IMessage<T>, new()
      {
        private Publisher binaryPublisher;

        /**
        * @brief Initialize eCAL API.
        *
        * @param topicName Topic name on which the publisher publishes Data.
        **/
        public ProtobufPublisher(string topicName)
        {
          T message = new T();
          binaryPublisher = new Publisher(topicName, Common.ProtobufHelper.GetProtoMessageTypeName(message), Common.ProtobufHelper.GetProtoMessageDescription(message));
        }

        /**
        * @brief Send a protobuf message
        *
        * @param message Message to send out.
        **/
        public bool Send(T message)
        {
          var serialized = message.ToByteArray();
          return (binaryPublisher.Send(Encoding.Default.GetString(serialized), -1) > 0);
        }
      }
    }
  }
}