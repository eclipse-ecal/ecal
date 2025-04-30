/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

/**
 * @file ConfigTest.cs
 * @brief Tests for type wrapper and initializing.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Eclipse.eCAL.Core;
using Microsoft.VisualStudio.TestTools.UnitTesting;

[TestClass]
public class ConfigTest
{
  [TestInitialize]
  public void Initialize()
  {
    Core.Initialize("ConfigTest");
  }

  [TestCleanup]
  public void Cleanup()
  {
    Core.Terminate();
  }

  [TestMethod]
  public void TestConfigPassing()
  {
    var myConfig = InitWrapper.InitConfiguration();
    Console.WriteLine("Dies it mein Test.");
    Assert.IsFalse(myConfig.Publisher.Layer.SHM.Enable, "SHM layer should be enabled by default.");
  }
}
