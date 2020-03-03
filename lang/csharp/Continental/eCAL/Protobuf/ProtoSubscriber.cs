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
       * @brief eCAL class to subscribe to protobuf Data.
      **/
      public class ProtobufSubscriber<T> where T : IMessage<T>, new()
      {
        private Subscriber binarySubscriber;
        private ReceiverCallback callback;

        /**
        * @brief Data which is received from a callback
        **/
        public class ReceiveCallbackData
        {
          public T data;           /*!< Message             */
          public long id;          /*!< Message id          */
          public long time;        /*!< Message time stamp  */
          public long clock;       /*!< Message write clock */

          public ReceiveCallbackData()
          {
            data = new T();
          }
        };

        private ReceiveCallbackData receivedData;

        /**
        * @brief Signature for a data callback.
        **/
        public delegate void ReceiverCallback(String topic, ReceiveCallbackData data);
        private ReceiverCallback delMethods;

        /**
        * @brief Constructor for a Protobuf Subscriber
        * 
        * @param topicName Topic name on which the subscriber subscribes to data.
        **/
        public ProtobufSubscriber(string topicName)
        {
          T msg = new T();
          binarySubscriber = new Subscriber(topicName, Common.ProtobufHelper.GetProtoMessageTypeName(msg), Common.ProtobufHelper.GetProtoMessageDescription(msg));
          receivedData = new ReceiveCallbackData();
        }

        /**
        * @brief Add a callback function to this subscriber
        * 
        * @param callbackFunction function which will be called when new data is available.
        **/
        public void AddReceiveCallback(ReceiverCallback callbackFunction)
        {
          this.callback = callbackFunction;
          delMethods += callbackFunction;
          binarySubscriber.AddReceiveCallback(callBack);
        }

        /**
        * @brief Remove a callback function from this subscriber
        * 
        * @param callbackFunction function to be removed from the callback list.
        **/
        public void RemReceiveCallback(ReceiverCallback del)
        {
          binarySubscriber.RemReceiveCallback(callBack);
          delMethods -= del;
          this.callback = null;
        }

        private void callBack(String topic, Core.Subscriber.ReceiveCallbackData data)
        {
          //String to Stream
          byte[] messageBytes = Encoding.Default.GetBytes(data.data);
          MemoryStream msgStream = new MemoryStream(messageBytes);

          receivedData.data.MergeFrom(msgStream);
          receivedData.id = data.id;
          receivedData.clock = data.clock;
          receivedData.time = data.time;
          //Execute passed methods
          delMethods(topic, receivedData);
        }

        /**
        * @brief Unregister this subscriber, no more data will be received
        **/
        public void finalized()
        {
          delMethods -= callback;
          binarySubscriber.RemReceiveCallback(callBack);
          binarySubscriber.Dispose();
          binarySubscriber.Destroy();
        }

        /**
        * @brief Receive data.
        * 
        * @param receiveTimeout Timeout after which the function returns, even if no data was received.
        *        If -1, it will only return after data was received.
        *        
        * @return  The received data, can be null if no data was received.
        **/
        public ReceiveCallbackData Receive(int receiveTimeout)
        {
          var receivedData = binarySubscriber.Receive(receiveTimeout);
          if (receivedData != null)
          {
            byte[] msgBytes = Encoding.Default.GetBytes(receivedData.data);
            var msgStream = new MemoryStream(msgBytes);

            var protobufCallbackData = new ReceiveCallbackData();
            protobufCallbackData.data = new T();
            protobufCallbackData.data.MergeFrom(msgStream);
            protobufCallbackData.clock = receivedData.clock;
            protobufCallbackData.id = receivedData.id;
            protobufCallbackData.time = receivedData.time;
            return protobufCallbackData;
          }
          else
          {
            return null;
          }
        }
      }
    }
  }
}

