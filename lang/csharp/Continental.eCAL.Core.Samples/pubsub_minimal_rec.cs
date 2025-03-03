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
using System.Text;
using Continental.eCAL.Core;

public class MinimalReceiveCallback
{
  // Callback matching the new delegate signature.
  static void ReceiveCallback(TopicId publisherId, DataTypeInformation dataTypeInfo, ReceiveCallbackData data)
  {
    Console.WriteLine("Topic name: " + publisherId.TopicName);
    // Convert the received binary buffer to a string.
    string content = Encoding.Default.GetString(data.Buffer);
    Console.WriteLine("Topic content: " + content);
  }

  static void Main()
  {
    // Initialize eCAL API.
    Core.Initialize("minimal_rcv_cb");

    // Print version info.
    Console.WriteLine(String.Format("eCAL {0} ({1})\n", Core.GetVersion(), Core.GetDate()));

    // Create a subscriber (topic name "Hello").
    Subscriber subscriber = new Subscriber("Hello");

    // Register a callback.
    subscriber.SetReceiveCallback(ReceiveCallback);

    // idle main thread
    while (Core.Ok())
    {
      System.Threading.Thread.Sleep(100);
    }

    // Dispose subscriber.
    subscriber.Dispose();

    // Finalize eCAL API.
    Core.Terminate();
  }
}
