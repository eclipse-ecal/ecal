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

public class PersonSnd
{
  static void Main()
  {
    // initialize eCAL API
    Util.Initialize("Person Send C#");

    // print version info
    System.Console.WriteLine(String.Format("eCAL {0} ({1})\n", Util.GetVersion(), Util.GetDate()));

    // create a publisher (topic name "Hello", type "base:std::string", description "")
    var publisher = new ProtobufPublisher<Pb.People.Person>("person");

    // idle main thread
    int loop = 0;
    var person = new Pb.People.Person
    {
      Id = 0,
      Email = "max@online.de",
      Name = "Max",
      Stype = Pb.People.Person.Types.SType.Female,
      Dog = new Pb.Animal.Dog { Name = "Brandy" },
      House = new Pb.Environment.House { Rooms = 4 }
    };

    while (Util.Ok())
    {
      // message to send
      person.Id = loop;
      loop++;

      // print message
      System.Console.WriteLine(String.Format("Sending:  {0}", person.ToString()));

      // send the content
      publisher.Send(person);

      // cool down
      System.Threading.Thread.Sleep(500);
    }

    // finalize eCAL API
    Util.Terminate();
  }
}
