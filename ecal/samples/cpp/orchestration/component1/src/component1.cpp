/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2022 Eclipse Foundation
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

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/server.h>
#include <ecal/msg/protobuf/publisher.h>

#include "orchestrator.pb.h"
#include "component.pb.h"

#include <iostream>

// component service class implementation
class ComponentServiceImpl final : public orchestrator::ComponentService
{
public:
  ComponentServiceImpl()
  {
    // create 2 publisher for sending messages 'foo' and 'vec'
    publisher_foo = std::make_unique<eCAL::protobuf::CPublisher<component::foo>>("foo");
    publisher_vec = std::make_unique<eCAL::protobuf::CPublisher<component::vec>>("vec");
  }

  // the component execute method
  void execute(google::protobuf::RpcController* /* controller */, const orchestrator::request* request,
    orchestrator::response* /*response*/, ::google::protobuf::Closure* /* done */) override
  {
    std::cout << "Component called with ID: " << request->id() << std::endl;

    // send 'foo'
    {
      component::foo msg;
      msg.set_id(request->id());
      msg.set_hello("HELLO");
      publisher_foo->Send(msg);
      std::cout << "Topic 'foo' sent with ID: " << request->id() << std::endl;
    }

    // send 'vec'
    {
      component::vec msg;
      msg.set_id(request->id());
      for (uint64_t i = 0U; i < 8192; ++i) msg.add_uvec(i);
      publisher_vec->Send(msg);
      std::cout << "Topic 'vec' sent with ID: " << request->id() << std::endl;
    }

    std::cout << std::endl;
  }

private:
  std::unique_ptr<eCAL::protobuf::CPublisher<component::foo>> publisher_foo;
  std::unique_ptr<eCAL::protobuf::CPublisher<component::vec>> publisher_vec;
};

int main()
{
  // initialize eCAL API
  const std::string component("component1");
  eCAL::Initialize(component);

  // start the component service
  std::shared_ptr<ComponentServiceImpl> component_service_impl = std::make_shared<ComponentServiceImpl>();
  eCAL::protobuf::CServiceServer<orchestrator::ComponentService> component1_service(component_service_impl, component);

  while (eCAL::Ok())
  {
    // sleep 100 ms
    eCAL::Process::SleepMS(100);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
