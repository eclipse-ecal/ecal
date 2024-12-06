/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Threading.Tasks;

[TestClass]
public class eCALBinaryTest
{
  [TestInitialize]
  public void Initialize()
  {
    Continental.eCAL.Core.Util.Initialize("Binary Test C#");
  }

  [TestCleanup]
  public void Cleanup()
  {
    Continental.eCAL.Core.Util.Terminate();
  }

  // This test ensures that 0  values which are not present on the wire, are still deserialized correctly. (#1593)
  [TestMethod]
  public void PublishSubscribeTest()
  {
    // create a subscriber
    Publisher publisher = new Publisher("Hello", "std::string", "base", "");
    Subscriber subscriber = new Subscriber("Hello", "std::string", "base", "");

    {
      string message = "HELLO WORLD FROM C#";
      publisher.Send(message, -1);
      var received_message = subscriber.Receive(100);
      Assert.IsNotNull(received_message);
      Assert.IsTrue(message == received_message.data);
    }

  }
}
