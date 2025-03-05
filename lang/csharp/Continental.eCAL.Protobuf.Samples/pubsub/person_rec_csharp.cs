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
using Pb.People;


public class PersonReceive
{
  public static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("person subscriber csharp");

    // Print version info.
    System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a protobuf subscriber (topic name "person").
    var subscriber = new ProtobufSubscriber<Pb.People.Person>("person");

    // Register a receive callback.
    subscriber.SetReceiveCallback((topic, data) =>
    {
      Console.WriteLine(String.Format("Receiving: {0}", data.data.ToString()));
    });

    // Idle main thread.
    while (Core.Ok())
    {
      System.Threading.Thread.Sleep(100);
    }

    // Dispose subscriber.
    // TODO: Add destructor to protobuf subscriber
    //subscriber.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
