.. _ecal_5_to_6_migration_guide:

======================
eCAL 6 Migration Guide
======================

eCAL 6 is the next major release after eCAL 5.
There have been a list of breaking changes; this migration guide explains the most important points that need to be taken care of.

C++ Core API
============


Header files
~~~~~~~~~~~~

In general, you can include the convenience header file ``#include <ecal/ecal.h>``.
All additional header files were prefixed with ``ecal_``, e.g., ``ecal/ecal_defs.h``. For eCAL 6, all ``ecal_`` prefixes have been removed.
Publisher/Subscriber-related files have been moved to the ``ecal/pubsub`` folder.


eCAL Publisher
~~~~~~~~~~~~~~

.. _publisher_constructor_changes:

1. Constructor Changes:
-----------------------

If you used the eCAL Publisher with just one parameter, the topic name, then nothing will change for you:

.. code-block:: cpp
  
  eCAL::CPublisher("topic_name");

However, the signatures of the constructor have changed, and the default parameters are different than in eCAL 5.

When setting the topic type, encoding, and descriptor, you can pass the ``SDataTypeInformation`` struct:

.. code-block:: cpp

  struct SDataTypeInformation
  {
    std::string name;          //!< name of the datatype
    std::string encoding;      //!< encoding of the datatype (e.g. protobuf, flatbuffers, capnproto)
    std::string descriptor;    //!< descriptor information of the datatype (necessary for reflection)
  };

Furthermore, you can also pass a publisher configuration object to the constructor.

Example for the full initialization:

.. code-block:: cpp

  // Initialize type information and get the configuration object
  auto data_type_information = eCAL::SDataTypeInformation{ "type_name", "encoding", "descriptor_information" };
  auto publisher_config      = eCAL::GetPublisherConfiguration{};
  
  // Make some individual changes to the publisher 
  publisher_config.layer.shm.zero_copy_mode = true;

  // Initialize the publisher with datatype information and the modified configuration
  eCAL::CPublisher publisher("topic_name", data_type_information, publisher_config);

There is another parameter, the ``eCAL::PubEventCallbackT``, you can pass to the constructor in order to get event information about the publisher events:

.. code-block:: cpp

  eCAL::PubEventCallbackT pub_event_callback = [](const eCAL::STopicId& topic_id_, const eCAL::SPubEventCallbackData& data_) {
    // Process the event data
  };

  eCAL::CPublisher publisher("topic_name", eCAL::SDataTypeInformation{}, pub_event_callback);

2. Removed Methods:
-------------------

As you can see from the previous section, you can configure the publisher with a configuration object.
This makes the convenience methods for setting up the publisher after initialization obsolete. 

In the following table you can see the removed function and, if available, the replacement. 
Please note, that when referenced to ``eCAL::Publisher::Configuration``, you need to set these settings in a configuration object from ``eCAL::GetPublisherConfiguration()`` and pass it to the constructor.
  
.. list-table:: Removed functions for CPublisher
  :header-rows: 1
  :widths: 10 90

  * - Removed Function
    - Replacement
  * - ``Create(...)``
    - none. Initialization of the publisher is only done via the constructor
  * - ``Destroy()``
    - none. Destruction is now handled by the publisher destructor
  * - ``SetTypeName(...)``
    - ``SDataTypeInformation::type_name``
  * - ``SetDescription(...)``
    - ``SDataTypeInformation::descriptor``
  * - ``SetDataTypeInformation(...)``
    - Pass an initialized ``eCAL::SDataTypeInformation`` to the constructor
  * - ``SetAttribute(...)``
    - none
  * - ``ClearAttribute()``
    - none
  * - ``ShareType(...)``
    - none
  * - ``ShareDescription(...)``
    - none
  * - ``SetQOS(...)``
    - none
  * - ``GetQOS()``
    - none
  * - ``SetLayerMode(...)``
    - for the layer you want to enable, set ``eCAL::Publisher::Configuration::layer::shm(udp, tcp).enable`` to true
  * - ``SetMaxBandwidthUDP(...)``
    - none
  * - ``ShmSetBufferCount(...)``
    - set ``eCAL::Publisher::layer::shm:memfile_buffer_count``
  * - ``ShmEnableZeroCopy()``
    - set ``eCAL::Publisher::layer::shm::zero_copy_mode`` to true
  * - ``ShmSetAcknowledgeTimeout``
    - set ``eCAL::Publisher::layer::shm:acknowledge_timeout_ms`` 
  * - ``SetID(...)``
    - none
  * - ``SendSynchronized(...)``
    - none
  * - ``AddEventCallBack(...)``
    - Pass the event callback to the constructor instead ``CPublisher(const std::string&, const SDataTypeInformation&, const PubEventCallbackT&, const Publisher::Configuration&)``
  * - ``RemEventCallBack(...)``
    - none
  * - ``IsCreated()``
    - none
  * - ``IsSubscribed()``
    - use ``GetSubscriberCount() > 0`` instead
  * - ``GetTypeName(...)``
    - ``eCAL::CPublisher::GetDataTypeInformation().type_name``
  * - ``GetDescription()``
    - ``eCAL::CPublisher::GetDataTypeInformation().descriptor``
  * - ``Dump(...)``
    - none

3. New Features:
----------------

- The ``CPublisher`` class now supports an optional ``eCAL::Publisher::Configuration`` parameter in its constructor for advanced and individual publisher configuration.
- ``GetTopicId()`` returns the ``STopicId`` struct, which contains the topic_name and a unique topic_id.

4. Example Migration:
---------------------

eCAL 5 Code:

.. code-block:: cpp

  #include <ecal/ecal_publisher.h>

  // ...

  eCAL::CPublisher publisher("topic_name", "topic_type", "topic_descriptor");

  publisher.AddEventCallback(pub_event_connected, [](const char * topic_name_, const struct SPubEventCallbackData * data_) {
    // Process the event data
  });
  
  publisher.SetTypeName("new_type");
  publisher.SetDescription("new_description");
  
  std::string type = publisher.GetTypeName();
  std::string desc = publisher.GetDescription();

  // ...

eCAL 6 Code:

.. code-block:: cpp

  #include <ecal/pubsub/publisher.h>

  // ...

  eCAL::CPublisher publisher("topic_name", { "encoding", "topic_type", "topic_desc" }, [](const STopicId& topic_id_, const SPubEventCallbackData& data_) {
    // Process the event data
  });
  
  auto& data_type_info   = publisher.GetDataTypeInformation();
  std::string type       = data_type_info.type_name;
  std::string descriptor = data_type_info.descriptor;

  // ...


eCAL Subscriber
~~~~~~~~~~~~~~~

1. Constructor Changes:
-----------------------

For the subscriber, similiar changes apply as for the publisher. See `publisher_constructor_changes`_ for more information.

2. Removed Methods:
----------------------

.. list-table:: Removed functions for CSubscriber
  :header-rows: 1
  :widths: 10 90

  * - Removed Function
    - Replacement
  * - ``Create(...)``
    - none. Initialization of the subscriber is only done via the constructor
  * - ``Destroy()``
    - none. Destruction is now handled by the subscriber destructor
  * - ``SetQOS(...)``
    - none
  * - ``GetQOS()``
    - none
  * - ``SetID(...)``
    - none
  * - ``SetAttribut(...)``
    - none
  * - ``ClearAttribute()``
    - none
  * - ``Receive(...)``
    - none - use callbacks instead
  * - ``ReceiveBuffer(...)``
    - none - use callbacks instead
  * - ``bool AddReceiveCallback(...)``
    - ``void SetReceiveCallback(...)``
  * - ``bool RemReceiveCallback(...)``
    - ``void RemoveReceiveCallback(...)``
  * - ``AddEventCallback(...)``
    - none - set EventCallback in constructor instead
  * - ``RemEventCallback(...)``
    - none
  * - ``IsCreated()``
    - none
  * - ``GetTypeName()``
    - ``eCAL::CSubscriber::GetDataTypeInformation().type_name``
  * - ``GetDescription()``
    - ``eCAL::CSubscriber::GetDataTypeInformation().descriptor``
  * - ``SetTimeout(...)``
    - none 
  * - ``Dump(...)``
    - none

3. New Feature:
---------------

- Same as the publisher, the ``CSubscriber`` class now supports an optional ``eCAL::Subscriber::Configuration`` parameter in its constructor for advanced and individual subscriber configuration.
- ``GetTopicId()`` returns the ``STopicId`` struct, which contains the topic_name and a unique topic_id.

4. Example Migration:
---------------------

eCAL 5 Code:

.. code-block:: cpp

  #include <ecal/ecal_subscriber.h>

  // ...

  eCAL::CSubscriber subscriber("topic_name", "topic_type", "topic_desc");
  
  subscriber.AddReceiveCallback([](const char * topic_name_, const struct SReceiveCallbackData * data_) {
    // Process received payload
  });
  
  std::string type = subscriber.GetTypeName();
  std::string desc = subscriber.GetDescription();

  // ...

eCAL 6 Code:

.. code-block:: cpp

  #include <ecal/pubsub/subscriber.h>

  // ...

  eCAL::CSubscriber subscriber("topic_name", { "encoding", "topic_type", "topic_desc" });
  
  subscriber.SetReceiveCallback([](const eCAL::STopicId& publisher_id_, const eCAL::SDataTypeInformation& data_type_info_, const eCAL::SReceiveCallbackData& data_) {
    // Process the received payload
  });

  auto& data_type_info = subscriber.GetDataTypeInformation();
  std::string type     = data_type_info.name;
  std::string desc     = data_type_info.descriptor;

  // ...


eCAL Service Server
~~~~~~~~~~~~~~~~~~~

1. Constructor Changes:
-----------------------

If you use the Server constructor only to set up the service name, then nothing will change for you:

.. code-block:: cpp

  eCAL::CServiceServer("service_name");

However, the signature of the constructor changed, so that you can also pass an optional event callback:

.. code-block:: cpp

  eCAL::CServiceServer("service_name", [](const eCAL::SServiceId& service_id_, const struct eCAL::SServerEventCallbackData& data_) {
    // Process event data
  });

The previous methods to set an event callback, ``AddEventCallback(...)`` and ``RemEventCallback(...)``, have been removed.

2. Removed methods:
-------------------
.. list-table:: Removed functions for Service Server
    :header-rows: 1
    :widths: 10 90

    * - Removed Function
      - Replacement
    * - ``Create(...)``
      - none. Initialization of the server is only done via the constructor
    * - ``Destroy(...)``
      - none. Destruction is now handled by the server destructor
    * - ``AddDescription(...)``
      - none - this will be set in the ``SetMethodCallback(...)`` method with the parameter ``SServiceMethodInformation``
    * - ``bool AddMethodCallback(...)``
      - ``bool SetMethodCallback(...)``
    * - ``bool RemMethodCallback(...)``
      - ``bool RemoveMethodCallback(...)``
    * - ``AddEventCallback(...)``
      - none - set EventCallback in constructor instead
    * - ``RemEventCallback(...)``
      - none

3. New Features:
----------------
  
The ``CServiceServer`` class now provides a method ``SServiceId GetServiceId() const`` to retrieve the service ID (entity_id, process_id, host_name) and the service name.

4. Example Migration:
---------------------

eCAL 5 Code:

.. code-block:: cpp

  #include <ecal/ecal_server.h>

  // ...
  
  eCAL::CServiceServer server("service_name");
  
  server.AddEventCallback(eCAL_Server_Event::server_event_connected, [](const char * name_, const struct eCAL::SServerEventCallbackData * data_) {
    // Process event data
  });
  
  server.AddMethodCallback("method_name", "req_type", "resp_type", [](const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_) {
    // Process method call
  });
  
  // ...
  

eCAL 6 Code:

.. code-block:: cpp

  #include <ecal/service/server.h>

  // ...

  eCAL::CServiceServer server("service_name", [](const eCAL::SServiceId& service_id_, const struct eCAL::SServerEventCallbackData& data_) {
    // Process 
  });

  server.SetMethodCallback({ "method_name", "req_type", "resp_type" }, [](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) {
    response_ = "response";
    std::cout << "Method name: " << method_info_.method_name << std::endl;
    // ...
    std::cout << "Request: " << request_ << std::endl;
    return 0;
  });

  // ...


eCAL Service Client
~~~~~~~~~~~~~~~~~~~

1. Constructor Changes:
-----------------------

- The default constructor has been removed
- If you use the Client constructor only to set up the service name, then nothing will change for you:
  
  .. code-block:: cpp

    eCAL::CServiceClient("service_name");

  However, use the new constructor ``CServiceClient(const std::string& service_name_, const ServiceMethodInformationSetT& method_information_set_, const ClientEventCallbackT& event_callback_)`` 
  to define more details and set an optional event callback.

  ``ServiceMethodInformationSetT`` is defined as a set of SServiceMethodInformation:

  .. code-block:: cpp

    struct SServiceMethodInformation
    {
      std::string          method_name;   //!< name of the method
      SDataTypeInformation request_type;  //!< type of the request
      SDataTypeInformation response_type; //!< type of the response
    };

  And ``SDataTypeInformation`` is defined the same as described in the publisher and subscriber sections.

  With this change, the methods to set and unset the event callback, ``AddEventCallback(...)`` and ``RemEventCallback(...)`` have been removed.

2. Removed methods:
-------------------

.. list-table:: Removed functions for Service Client
  :header-rows: 1
  :widths: 10 90

  * - Removed Function
    - Replacement
  * - ``Create(...)``
    - none. Initialization of the client is only done via the constructor
  * - ``Destroy(...)``
    - none. Destruction is now handled by the client destructor
  * - ``SetHostName(...)``
    - none
  * - ``bool Call(...)``
    - ``bool CallWithCallback(...)`` or ``bool CallWithResponse(...)``
  * - ``bool CallAsync(...)``
    - ``bool CallWithCallbackAsync(...)``
  * - ``AddResponseCallback(...)``
    - none - use ``CallWithResponse(...)`` instead
  * - ``RemResponseCallback(...)``
    - none
  * - ``AddEventCallback(...)``
    - none - set EventCallback in constructor instead
  * - ``RemEventCallback(...)``
    - none

3. Method Call Changes:
-----------------------
  
The ``Call`` method has been replaced with more specific methods:

- Use ``bool CallWithResponse(const std::string& method_name_, const std::string&, ServiceResponseVecT&, int timeout_ms_)`` for blocking calls with responses.
- Use ``bool CallWithCallback(const std::string& method_name_, const std::string&, const ResponseCallbackT&, int timeout_ms_)`` for blocking calls with a callback.
- Use ``bool CallWithCallbackAsync(const std::string& method_name_, const std::string&, const ResponseCallbackT&)`` for asynchronous calls.

Note that the timeout_ms parameter is optional and defaults to -1 (which means infinite timeout).

4. New Features:
----------------
  
As the server, the ``eCAL::CServiceClient`` class now provides a method ``SServiceId GetServiceId() const`` to retrieve the service ID (entity_id, process_id, host_name) and the service name.

Furthermore, the method ``GetClientInstances()`` was added. It returns a vector of instances for all matching services.

5. Example Migration:
---------------------

eCAL 5 Code:

.. code-block:: cpp

  #include <ecal/ecal_client.h>

  // ...

  eCAL::CServiceClient client("service_name");
  
  client.AddEventCallback(eCAL_Client_Event::client_event_connected, [](const char * name_, const struct eCAL::SClientEventCallbackData * data_) {
    // Process event data
  });
  
  std::vector<eCAL::SServiceResponse> responses;
  client.Call("method_name", "request_payload", 1000, &responses);

  // ...

eCAL 6 Code:

.. code-block:: cpp

  #include <ecal/service/client.h>

  // ...

  eCAL::CServiceClient client("service_name", eCAL::ServiceMethodInformationSetT(), [](const eCAL::SServiceId& service_id_, const struct eCAL::SClientEventCallbackData& data_) {
    // Process event data
  });
  
  eCAL::ServiceResponseVecT responses;
  client.CallWithResponse("method_name", "request_payload", responses, 1000);

  // ...


Configuration API
~~~~~~~~~~~~~~~~~

Previously, eCAL was configured mainly by its configuration file (``ecal.ini``).
The file format was changed to ``ecal.yaml`` to allow for hierarchical configurations.
At the same time, it's now possible to manually set all configurations through code, too.
This is especially useful for hard configuration of the communication system, like for executing unit tests.
For more information/usage, please read the section about :ref:`configuration <configuration_options>` .


Removed functions
~~~~~~~~~~~~~~~~~

Next to the removed functions in the publisher/subscriber/service classes, there are several functions that have been removed.
Mostly because you can directly access the content via the the configuration Object, so that the convenient functions are not needed anymore.

The following functions were from the config.h (previous ecal_config.h):

.. list-table:: Removed functions for (ecal\_)config.h
  :header-rows: 1
  :widths: 10 90

  * - Removed Function
    - Replacement
  * - ``GetMaxUdpBandwidthBytesPerSecond()``
    - none
  * - ``IsInprocRecEnable()``
    - none
  * - ``std::string GetHostGroupName()``
    - ``std::string GetShmTransportDomain()``
  * - ``GetMonitoringTimeoutMs()``
    - none
  * - ``GetMonitoringFilterExcludeList()``
    - none
  * - ``GetMonitoringFilterIncludeList()``
    - none
  * - ``GetConsoleLogFilter()``
    - ``eCAL::GetConfiguration().logging.provider.console.filter_log``
  * - ``GetFileLogFilter()``
    - ``eCAL::GetConfiguration().logging.provider.file.filter_log``
  * - ``GetUdpLogFilter()``
    - ``eCAL::GetConfiguration().logging.provider.udp.filter_log``
  * - ``GetEcalSysFilterExcludeList()``
    - ``eCAL::GetConfiguration().application.sys.filter_excl``
  * - ``GetPublisherInprocMode()``
    - none
  * - ``GetPublisherShmMode()``
    - ``eCAL::GetConfiguration().publisher.layer.shm.enable``
  * - ``GetPublisherTcpMode()``
    - ``eCAL::GetConfiguration().publisher.layer.tcp.enable``
  * - ``GetPublisherUdpMulticastMode()``
    - none
  * - ``GetMemfileMinsizeBytes()``
    - ``eCAL::GetConfiguration().publisher.layer.shm.memfile_min_size_bytes``
  * - ``GetMemfileOverprovisioningPercentage()``
    - ``eCAL::GetConfiguration().publisher.layer.shm.memfile_reserve_percent``
  * - ``GetMemfileAckTimeoutMs()``
    - ``eCAL::GetConfiguration().publisher.layer.shm.acknowledge_timeout_ms``
  * - ``IsMemfileZerocopyEnabled()``
    - ``eCAL::GetConfiguration().publisher.layer.shm.zero_copy_mode``
  * - ``GetMemfileBufferCount()``
    - ``eCAL::GetConfiguration().publisher.layer.shm.memfile_buffer_count``
  * - ``IsTopicTypeSharingEnabled()``
    - none
  * - ``IsTopicDescriptionSharingEnabled()``
    - none
  * - ``IsServiceProtocolV0Enabled()``
    - none
  * - ``IsServiceProtocolV1Enabled()``
    - is now default
  * - ``IsShmMonitoringEnabled()``
    - none
  * - ``IsNetworkMonitoringDisabled()``
    - none
  * - ``GetDropOutOfOrderMessages()``
    - ``eCAL::GetConfiguration().subscriber.drop_out_of_order_messages``


C Core API
==========

The C Core API has been reworked from the ground up. Now you will find all functionality that is also available in the C++ API.
The API is now more consistent and easier to use.

There are some rules to apply when using the C Core API:

1. In case eCAL functions return a non-constant pointer by value or reference, use ``eCAL_Free`` to free the memory.

2. In case eCAL functions return a constant pointer by value or reference, do not free the memory.

3. If you create a new entity with eCAL_xxx_New(..), it is your responsibility to free the memory with eCAL_xxx_Delete(..).

For more information, please check out the C API samples.


C# Core API
===========


Python Core API
===============


CMake / Build system
====================

We have made a few changes to the CMake build files.


CMake Options
~~~~~~~~~~~~~

A lot of CMake options were prefixed with `ECAL_` to avoid name clashes with other libraries and/or renamed slightly.

.. csv-table:: Example :rst:dir:`csv-table`
   :header: "Old variable", "New variable"
   
  ``HAS_HDF5``,                  ``ECAL_USE_HDF5``
  ``HAS_QT``,                    ``ECAL_USE_QT``
  ``HAS_CURL``,                  ``ECAL_USE_CURL``
  ``HAS_FTXUI``,                 ``ECAL_USE_FTXUI``
  ``HAS_CAPNPROTO``,             ``ECAL_USE_CAPNPROTO``
  ``HAS_FLATBUFFERS``,           ``ECAL_USE_FLATBUFFERS``
  ``HAS_MESSAGEPACK``,           ``ECAL_USE_MESSAGEPACK``
  ``BUILD_DOCS``,                ``ECAL_BUILD_DOCS``
  ``BUILD_APPS``,                ``ECAL_BUILD_APPS``
  ``BUILD_SAMPLES``,             ``ECAL_BUILD_SAMPLES``
  ``BUILD_TIME``,                ``ECAL_BUILD_TIMEPLUGINS``
  ``BUILD_C_BINDING``,           ``ECAL_BUILD_C_BINDING``
  ``BUILD_PY_BINDING``,          ``ECAL_BUILD_PY_BINDING``
  ``BUILD_CSHARP_BINDING``,      ``ECAL_BUILD_CSHARP_BINDING``
  ``ECAL_NPCAP_SUPPORT``,        ``ECAL_USE_NPCAP``
  ``BUILD_ECAL_TESTS``,          ``ECAL_BUILD_TESTS``
  ``CPACK_PACK_WITH_INNOSETUP``, ``ECAL_CPACK_PACK_WITH_INNOSETUP``


CMake Targets
~~~~~~~~~~~~~

A few targets have changed their name, and there are a few dedicated targets.
Basically, ``eCAL::core`` only contains the core functionality, but no message/serialization support.
If you need to use the functionality, please link the following:

- Protobuf: ``eCAL::protobuf_core``
- Capnproto: ``eCAL::capnproto_core``
- String: ``eCAL::string_core``
 
These targets will automatically publicly link the serialization libraries, so no need to additionally link e.g., ``protobuf::libprotobuf``.


CMake Preset
~~~~~~~~~~~~

From now on, you can use the CMake Preset feature to configure your build. By default, there are the most common presets already available.
If you want to use a custom preset, you can use a ``CMakeUserPresets.json`` file in the root directory.