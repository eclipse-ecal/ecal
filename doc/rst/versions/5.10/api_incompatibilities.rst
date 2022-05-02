:orphan:

=================
eCAL 5.10 changes
=================

.. _compatibility_details_5_10_service_api:

Service API - C
===============

The service info struct has been renamed and extended

Old 5.9.x version:

.. code-block:: cpp

  struct SServiceInfoC
  {
    const char*      host_name;      //!< service host
    const char*      service_name;   //!< service name
    const char*      method_name;    //!< method name
    const char*      error_msg;      //!< error message in case of failure
    int              ret_state;      //!< return state from method callback
    enum eCallState  call_state;     //!< service call state
  };
  
New 5.10.x version (now containing 2 new elements for the service response):

.. code-block:: cpp

  struct SServiceResponseC
  {
    const char*      host_name;     //!< service host name
    const char*      service_name;  //!< name of the service
    const char*      service_id;    //!< id of the service
    const char*      method_name;   //!< name of the service method
    const char*      error_msg;     //!< human readable error message
    int              ret_state;     //!< return state of the called service method
    enum eCallState  call_state;    //!< call state (see eCallState)
    const char*      response;      //!< service response
    int              response_len;  //!< service response length (response string could include zeros)
  };
  
The 2 new elements ``response`` and ``response_len`` are used in the new C client response callback type
  
Old 5.9.x version:

.. code-block:: cpp

  /**
   * @brief eCAL service response callback function (client side)
   *
   * @param service_info_       Service info struct.
   * @param reponse_            Method response payload.
   * @param reponse_len_        Method response payload length.
   * @param par_                Forwarded user defined parameter.
  **/
  typedef void (*ResponseCallbackCT)(const struct SServiceInfoC* service_info_, const char* response_, int response_len_, void* par_);

New 5.10.x version:

.. code-block:: cpp

  /**
   * @brief eCAL service response callback function (client side)
   *
   * @param service_response_   Service response struct containing the (responding) server informations and the response itself.
   * @param par_                Forwarded user defined parameter.
  **/
  typedef void(*ResponseCallbackCT)(const struct SServiceResponseC* service_response_, void* par_);

The signature of the blocking client service call function has been changed.

Old 5.9.x version:

.. code-block:: cpp

  /**
   * @brief Call method of this service (blocking variant with timeout). 
   *
   * @param       handle_        Client handle. 
   * @param       host_name_     Host name.
   * @param       method_name_   Method name.
   * @param       request_       Request message buffer. 
   * @param       request_len_   Request message length. 
   * @param [out] service_info_  Service info struct with additional infos like call state and
   *                             error message.
   * @param [out] response_      Pointer to the allocated buffer for the response message.
   * @param       response_len_  Response message buffer length or ECAL_ALLOCATE_4ME if
   *                             eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Size of response buffer if succeeded, otherwise zero.
  **/
  ECALC_API int eCAL_Client_Call_Wait(ECAL_HANDLE handle_, const char* host_name_, const char* method_name_, const char* request_, int request_len_, struct SServiceInfoC* service_info_, void* response_, int response_len_);

New 5.10.x version:

.. code-block:: cpp

  /**
   * @brief Call a method of this service (blocking variant). 
   *
   * @param       handle_            Client handle. 
   * @param       method_name_       Method name.
   * @param       request_           Request message buffer. 
   * @param       request_len_       Request message length. 
   * @param       timeout_           Maximum time before operation returns (in milliseconds, -1 means infinite).
   * @param [out] service_response_  Service response struct with additional infos like call state and
   *                                 error message.
   * @param [out] response_          Pointer to the allocated buffer for the response message.
   *                                 In case of more then one connected server, only the reponse of the first
   *                                 answering server is returned (use callback variant to get all responses).
   * @param       response_len_      Response message buffer length or ECAL_ALLOCATE_4ME if
   *                                 eCAL should allocate the buffer for you (see eCAL_FreeMem). 
   *
   * @return  Size of response buffer if succeeded, otherwise zero.
  **/
  ECALC_API int eCAL_Client_Call_Wait(ECAL_HANDLE handle_, const char* method_name_, const char* request_, int request_len_, int timeout_, struct SServiceResponseC* service_response_, void* response_, int response_len_);
  
The new signature is using the new ``SServiceResponseC`` struct and an additional ``timeout_`` parameter. The 5.9 version was blocking infinitely by default.



Service API - C++
=================

The service info struct has been renamed and extended.

Old 5.9.x version:

.. code-block:: cpp

  struct SServiceInfo
  {
    std::string  host_name;      //!< service host name
    std::string  service_name;   //!< name of the service
    std::string  method_name;    //!< name of the service method
    std::string  error_msg;      //!< human readable error message
    int          ret_state;      //!< return state of the called service method
    eCallState   call_state;     //!< call state (see eCallState)
  };


New 5.10.x version (now containing the service ``response``):

.. code-block:: cpp

  struct SServiceResponse
  {
    std::string  host_name;      //!< service host name
    std::string  service_name;   //!< name of the service
    std::string  service_id;     //!< id of the service
    std::string  method_name;    //!< name of the service method
    std::string  error_msg;      //!< human readable error message
    int          ret_state;      //!< return state of the called service method
    eCallState   call_state;     //!< call state (see eCallState)
    std::string  response;       //!< service response
  };

Additional to the new ``SServiceResponse`` struct there is a new ``ServiceResponseVecT`` type that is used to return multiple services response as a standard vector in all kinds of service calls.

.. code-block:: cpp

  typedef std::vector<SServiceResponse> ServiceResponseVecT; //!< vector of multiple service responses
  
So the service response callback type been changed too.

Old 5.9.x version:

.. code-block:: cpp

  typedef std::function<void(const struct SServiceInfo& service_info_, const std::string& response_)> ResponseCallbackT ->

New 5.10.x version:

.. code-block:: cpp

  typedef std::function<void(const struct SServiceResponse& service_response_)> ResponseCallbackT

Calling a client in C++ is fully downward compatible excepting that blocking call variant

Old 5.9.x version:

.. code-block:: cpp

  /**
   * @brief Call method of this service, for specific host. 
   *
   * @param       host_name_     Host name.
   * @param       method_name_   Method name.
   * @param       request_       Request string. 
   * @param [out] service_info_  Service info struct for detailed informations.
   * @param [out] response_      Response string.
   *
   * @return  True if successful. 
  **/ 
  bool Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceInfo& service_info_, std::string& response_);

New 5.10.x version (this is deprecated, see samples section how to use the new recommended interface):

.. code-block:: cpp

  /**
   * @brief Call method of this service, for specific host (deprecated). 
   *
   * @param       host_name_         Host name.
   * @param       method_name_       Method name.
   * @param       request_           Request string.
   * @param [out] service_info_      Service response struct for detailed informations.
   * @param [out] response_          Response string.
   *
   * @return  True if successful.
  **/
  [[deprecated]]
  bool Call(const std::string& host_name_, const std::string& method_name_, const std::string& request_, struct SServiceResponse& service_info_, std::string& response_);



Service API - C++ (Protobuf)
============================

Old 5.9.x version:

.. code-block:: cpp

  /**
    * @brief Call method of this service, for specific host.
    *
    * @param       host_name_     Host name.
    * @param       method_name_   Method name.
    * @param       request_       Request message.
    * @param [out] service_info_  Service info struct for detailed informations.
    * @param [out] response_      Response string.
    *
   * @return  True if successful.
   **/
   bool Call(const std::string& host_name_, const std::string& method_name_, const google::protobuf::Message& request_, struct SServiceInfo& service_info_, google::protobuf::Message& response_)

New 5.10.x version (this is deprecated, see samples section how to use the new recommended interface):

.. code-block:: cpp

  /**
    * @brief Call method of this service, for specific host (deprecated).
    * 
    * @param       host_name_         Host name.
    * @param       method_name_       Method name.
    * @param       request_           Request message.
    * @param [out] service_response_  Service response struct for detailed informations.
    * @param [out] response_          Response message.
    *
    * @return  True if successful.
   **/
   [[deprecated]]
   bool Call(const std::string& host_name_, const std::string& method_name_, const google::protobuf::Message& request_, struct SServiceResponse& service_response_, google::protobuf::Message& response_)



Service Sample - C
==================

A minimal service client implementation can been seen here.

Old 5.9.x version:

.. code-block:: cpp

  struct SServiceInfoC service_info;
  char                 request[]                 = "HELLO";
  char                 response[sizeof(request)] = { 0 };
  // call method "echo"
  eCAL_Client_Call_Wait(client_handle, "", "echo", request, sizeof(request), &service_info, &response, sizeof(response));

New 5.10.x version:

.. code-block:: cpp

  struct SServiceResponseC service_response;
  char                     request[]                 = "HELLO";
  char                     response[sizeof(request)] = { 0 };
  // call method "echo"
  eCAL_Client_Call_Wait(client_handle, "echo", request, sizeof(request), -1, &service_response, &response, sizeof(response));



Service Sample - C++
====================

Old 5.9.x version:

.. code-block:: cpp

  eCAL::SServiceInfo service_info;
  std::string method_name("echo");
  std::string request("Hello");
  std::string response;
  minimal_client.Call("", method_name, request, service_info, response);

New 5.10.x version (deprecated):

.. code-block:: cpp

  eCAL::SServiceResponse service_info;
  std::string method_name("echo");
  std::string request("Hello");
  std::string response;
  minimal_client.Call("", method_name, request, service_info, response);

New 5.10.x version (new recommended API):

.. code-block:: cpp

  eCAL::SServiceResponse service_info;
  std::string method_name("echo");
  std::string request("Hello");
  eCAL::ServiceResponseVecT service_response_vec;
  minimal_client.Call(method_name, request, /*timeout_*/ -1, &service_response_vec);
  for (auto service_response : service_response_vec)
  {
    std::cout << "Received response : " << service_response.response << std::endl;
  }



Service Sample - C++ (Protobuf)
===============================

Old 5.9.x version:

.. code-block:: cpp

  eCAL::SServiceInfo service_info;
  PingRequest        ping_request;
  PingResponse       ping_response;
  ping_request.set_message("PING");
  ping_service.Call("", "Ping", ping_request, service_info, ping_response);
  
New 5.10.x version:

.. code-block:: cpp

  PingRequest               ping_request;
  eCAL::ServiceResponseVecT service_response_vec;
  ping_request.set_message("PING");
  ping_service.Call("Ping", ping_request, /*timeout_*/ -1, &service_response_vec);
  for (auto service_response : service_response_vec)
  {
    PingResponse ping_response;
    ping_response.ParseFromString(service_response.response);
  }
