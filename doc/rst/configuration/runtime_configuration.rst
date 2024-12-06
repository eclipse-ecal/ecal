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

The configuration will be first initialized with the default values specified by eCAL.
If you want to use the systems eCAL .yaml file, call the ``InitFromConfig()`` function of the config object.

In case the .yaml to use is specified via command line parameter, this one is chosen instead. 
The object will throw an error, in case the specified .yaml file cannot be found.

It is also possible to specify the .yaml by calling the function ``InitFromFile(const std::string yaml_path_)`` of the config object.

* |fa-file-alt| :file:`hello_config/main.cpp`:

  .. literalinclude:: src/hello_config/main.cpp
     :language: cpp
     :linenos:

Individual publisher/subscriber configuration
=============================================

Like a global configuration to pass at initialization, it is also possible to create indiviual configurations for publisher and subscriber.
That means it is possible to, e.g., create two publisher which send on different transport layers:

* |fa-file-alt| :file:`publisher/main.cpp`:

  .. literalinclude:: src/publisher_config/main.cpp
     :language: cpp
     :linenos:

