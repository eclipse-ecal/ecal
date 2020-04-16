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

public class minimal_rcv
{
  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("minimal_rcv");

    // print version info
    System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Util.GetVersion(), Util.GetDate()));

    // create a subscriber (topic name "Hello", type "base:std::string")
    Subscriber subscriber = new Subscriber("Hello", "base:std::string", "");

    // idle main thread
    while (Util.Ok())
    {
      // receive content with 100 ms timeout
      Subscriber.ReceiveCallbackData message = subscriber.Receive(100);

      // print message
      if (message != null) System.Console.WriteLine(String.Format("Received:  {0}", message.data));
    }

    // dispose subscriber
    subscriber.Dispose();

    // finalize eCAL API
    Util.Terminate();
  }
}
