#include <ecal/ecal.h>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

/*
  We define the callback function that will be called when a client calls the service method "echo".
  This callback will simply return the request as the response.
*/
int OnCallback(eCAL::CServiceClient& client, const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
{
  static int call_count = 0;
  std::cout << "Forwarding call " << ++call_count << "\n";
  
  auto instances = client.GetClientInstances();
  if (instances.size() > 0)
  {
    // Call only first instance
    auto relay_response = instances[0].CallWithResponse(method_info_.method_name, request_);
    response_ = relay_response.second.response;
    /*
    std::cout << "Finished forwarding service call" << "\n";
    */
  }
  else
  {
    std::cout << "Cannot forward service call" << "\n";
  }

  // The return value here has no actual meaning. It's made available to the caller.
  return 0;
}

int main()
{
  std::cout << "--------------------" << "\n";
  std::cout << " C++: MIRROR SERVER" << "\n";
  std::cout << "--------------------" << "\n";

  /*
    As always: initialize the eCAL API and give your process a name.
  */
  eCAL::Initialize("relay");

  std::cout << "eCAL " << eCAL::GetVersionString() << " (" << eCAL::GetVersionDateString() << ")" << "\n";
  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "I feel good!");

  /*
    Now we create the mirror server and give it the name "mirror".
  */
  eCAL::CServiceClient relay_client("relayed_mirror", { {"echo", {}, {} }, {"reverse", {}, {} } });
  eCAL::CServiceServer relay_server("mirror");

  /*
    The server will have two methods: "echo" and "reverse".
    To set a callback, we need to set a ServiceMethodInformation struct as well as the callback function.
    We simplify the struct creation of ServiceMethodInformationStruct and the other two
    fields can be left empty for our example.
  */
  relay_server.SetMethodCallback({ "echo", {}, {} }, [&relay_client](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
  {
    return OnCallback(relay_client, method_info_, request_, response_);
  });
  relay_server.SetMethodCallback({ "reverse", {}, {} }, [&relay_client](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)
  {
    return OnCallback(relay_client, method_info_, request_, response_);
  });
  /*
    Now we will go in an infinite loop, to wait for incoming service calls that will be handled with the callbacks.
  */
  while (eCAL::Ok())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  /*
    After we are done, as always, finalize the eCAL API.
  */
  eCAL::Finalize();

  return(0);
}