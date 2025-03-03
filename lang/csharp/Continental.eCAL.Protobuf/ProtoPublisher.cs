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
          T msg = new T();
          DataTypeInformation dataTypeInfo = new DataTypeInformation(
                  Common.ProtobufHelper.GetProtoMessageTypeName(msg),
                  "proto",
                  Common.ProtobufHelper.GetProtoMessageDescription(msg).ToString()
                );
          binaryPublisher = new Publisher(topicName, dataTypeInfo);
        }

        /**
        * @brief Send a protobuf message
        *
        * @param message Message to send out.
        **/
        public bool Send(T message)
        {
          var serialized = message.ToByteArray();
          return binaryPublisher.Send(serialized);
        }

        /**
         * @brief Send a protobuf message with a specified send time.
         *
         * @param message Message to send out.
         * @param time Send time in microseconds.
         * 
         * @return True if the message was sent successfully; otherwise, false.
         */
        public bool Send(T message, long time)
        {
          var serialized = message.ToByteArray();
          return binaryPublisher.Send(serialized, time);
        }
      }
    }
  }
}
