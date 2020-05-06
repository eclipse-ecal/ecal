/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

using System;
using Continental.eCAL.Core;


public class PersonRecJSONCb
{
  static void PersonCallback(String topic, JSONProtobufSubscriber.ReceiveCallbackData data)
  {
    System.Console.WriteLine("Topic name " + topic);
    System.Console.WriteLine("Topic content " + data.data);
  }

  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("Person receive JSON callback");

    // print version info
    System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Util.GetVersion(), Util.GetDate()));

    // create a subscriber (topic name "Hello", type "base:std::string")
    JSONProtobufSubscriber subscriber = new JSONProtobufSubscriber("person");
    JSONProtobufSubscriber.ReceiverCallback callback = PersonCallback;
    subscriber.AddReceiveCallback(callback);

    // idle main thread
    while (Util.Ok())
    {
      // receive content with 100 ms timeout
      System.Threading.Thread.Sleep(100);
    }

    // dispose subscriber
    subscriber.Dispose();

    // finalize eCAL API
    Util.Terminate();
  }
}

