.. include:: /include.txt

.. _configuration_runtime_configuration:

=====================
Runtime configuration
=====================

eCAL provides an interface to access and modify its options before initialization. 
The corresponding structure reflects the configuration file (:ref:`configuration_options`). 

Custom types
============

In order to rule out configuration errors, a custom datatype for IP addresses (IpAddressV4) is introduced.

**IpAddressV4:** For assigning an IP address simply assign a string with the desired address. 
Decimal and hexadecimal format is supported. 
In case the IP address is not valid, the type will throw a std::invalid_argument exception.

The IP address can be used like a normal string object. For example:

.. code-block:: c++

  eCAL::Types::IpAddressV4 ip_address = "192.168.7.1"; // in hex: "C0.A8.7.1"
  std::cout << ip_address << "\n"; 


Global configuration initialization
===================================

To get the configuration, create the object by using the function in the init namespace:

.. code-block:: c++

  auto custom_config = eCAL::Init::Configuration();

The configuration will be first initialized with the values specified by your ecal.yaml.

It is also possible to specify the .yaml by calling the function

.. code-block:: c++

  custom_config.InitFromFile("path/to/your/ecal.yaml");

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: /source_code_samples/cpp/configuration/hello_config/main.cpp
     :language: cpp
     :linenos:

Individual publisher/subscriber configuration
=============================================

Like a global configuration to pass at initialization, it is also possible to create indiviual configurations for publisher and subscriber.
That means it is possible to, e.g., create two publisher which send on different transport layers:

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: /source_code_samples/cpp/configuration/publisher_config/main.cpp
     :language: cpp
     :linenos:

