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


public class PersonReceiveCallback
{
  static void PersonCallback(String topic, ProtobufSubscriber<Pb.People.Person>.ProtobufData data)
  {
    System.Console.WriteLine("Topic name " + topic);
    System.Console.WriteLine("Topic content " + data.data);
  }

  static void Main()
  {
    // initialize eCAL API
    Core.Initialize("Person Receive Callback C#");
    var subscriber = new ProtobufSubscriber<Pb.People.Person>("person");
    // print version info
    System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // create a subscriber (topic name "Hello", type "base:std::string")

    ProtobufSubscriber<Pb.People.Person>.ReceiverCallback callback = PersonCallback;
    subscriber.SetReceiveCallback(callback);

    // idle main thread
    while (Core.Ok())
    {
      // receive content with 100 ms timeout
      System.Threading.Thread.Sleep(100);
    }

    // finalize eCAL API
    Core.Terminate();
  }
}
