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
#include <thread>
#include <chrono>
#include <mutex>

class ComponentServiceImpl final : public orchestrator::ComponentService
{
public:
  ComponentServiceImpl()
  {
    subscriber_foo = eCAL::protobuf::CSubscriber<component::foo>("foo");
    subscriber_foo.AddReceiveCallback(std::bind(&ComponentServiceImpl::on_foo_message, this, std::placeholders::_2));

    subscriber_vec = eCAL::protobuf::CSubscriber<component::vec>("vec");
    subscriber_vec.AddReceiveCallback(std::bind(&ComponentServiceImpl::on_vec_message, this, std::placeholders::_2));
  }

  void execute(google::protobuf::RpcController* /*controller*/, const orchestrator::request* request,
    orchestrator::response* /*response*/, ::google::protobuf::Closure* /*done*/) override
  {
    std::lock_guard<std::mutex> lock(callback_mtx);
    uint64_t id_component = request->id();

    bool consistency = (id_component == id_foo) && (id_foo == id_vec);
    if (!consistency)
    {
      std::cout << std::endl << "Component interface inconsistent !" << std::endl << std::endl << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      err_cnt++;
    }
    else
    {
      std::cout << "Component called with     ID: " << id_component << std::endl;
      if(err_cnt > 0) std::cout << "Error count                 : " << err_cnt << std::endl;
      std::cout << std::endl;
    }
  }

  void on_foo_message(const component::foo& msg)
  {
    std::lock_guard<std::mutex> lock(callback_mtx);
    id_foo = msg.id();
    std::cout << "Topic 'foo' received with ID: " << id_foo << std::endl;
  }

  void on_vec_message(const component::vec& msg)
  {
    std::lock_guard<std::mutex> lock(callback_mtx);
    id_vec = msg.id();
    std::cout << "Topic 'vec' received with ID: " << id_vec << std::endl;
  }

private:
  std::mutex callback_mtx;
  eCAL::protobuf::CSubscriber<component::foo> subscriber_foo;
  eCAL::protobuf::CSubscriber<component::vec> subscriber_vec;
  uint64_t                                    id_foo  = 0;
  uint64_t                                    id_vec  = 0;
  int                                         err_cnt = 0;
};

int main(int argc, char** argv)
{
  const std::string component("component2");

  eCAL::Initialize(argc, argv, component.c_str());

  std::shared_ptr<ComponentServiceImpl> component_service_impl = std::make_shared<ComponentServiceImpl>();
  eCAL::protobuf::CServiceServer<orchestrator::ComponentService> component1_service(component_service_impl, component);

  while (eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  eCAL::Finalize();
}
