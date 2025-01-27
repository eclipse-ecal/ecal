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
#include <ecal/msg/protobuf/subscriber.h>

#include "orchestrator.pb.h"
#include "component.pb.h"

#include <iostream>
#include <mutex>

// component service class implementation
class ComponentServiceImpl final : public orchestrator::ComponentService
{
public:
  ComponentServiceImpl()
  {
    // create subscriber for topic 'foo'
    subscriber_foo = std::make_unique<eCAL::protobuf::CSubscriber<component::foo>>("foo");
    subscriber_foo->SetReceiveCallback(std::bind(&ComponentServiceImpl::on_foo_message, this, std::placeholders::_2));

    // create subscriber for topic 'vec'
    subscriber_vec = std::make_unique<eCAL::protobuf::CSubscriber<component::vec>>("vec");
    subscriber_vec->SetReceiveCallback(std::bind(&ComponentServiceImpl::on_vec_message, this, std::placeholders::_2));
  }

  // the component execute method
  void execute(google::protobuf::RpcController* /*controller*/, const orchestrator::request* request,
    orchestrator::response* /*response*/, ::google::protobuf::Closure* /*done*/) override
  {
    std::lock_guard<std::mutex> lock(callback_mtx);
    uint64_t id_component = request->id();

    // component call id and the id's of 'foo' and 'vec' should be equal
    bool consistency = (id_component == id_foo) && (id_foo == id_vec);
    if (!consistency)
    {
      std::cout << std::endl << "Component interface inconsistent !" << std::endl << std::endl << std::endl;
      eCAL::Process::SleepMS(10000);
      err_cnt++;
    }
    else
    {
      std::cout << "Component called with     ID: " << id_component << std::endl;
      if(err_cnt > 0) std::cout << "Error count                 : " << err_cnt << std::endl;
      std::cout << std::endl;
    }
  }

  // 'foo' message callback
  void on_foo_message(const component::foo& msg)
  {
    std::lock_guard<std::mutex> lock(callback_mtx);
    id_foo = msg.id();
    std::cout << "Topic 'foo' received with ID: " << id_foo << std::endl;
  }

  // 'vec' message callback
  void on_vec_message(const component::vec& msg)
  {
    std::lock_guard<std::mutex> lock(callback_mtx);
    id_vec = msg.id();
    std::cout << "Topic 'vec' received with ID: " << id_vec << std::endl;
  }

private:
  std::mutex callback_mtx;
  std::unique_ptr<eCAL::protobuf::CSubscriber<component::foo>> subscriber_foo;
  std::unique_ptr<eCAL::protobuf::CSubscriber<component::vec>> subscriber_vec;
  uint64_t                                    id_foo  = 0;
  uint64_t                                    id_vec  = 0;
  int                                         err_cnt = 0;
};

int main()
{
  // initialize eCAL API
  const std::string component("component2");
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
