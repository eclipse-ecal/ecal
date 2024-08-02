.. include:: /include.txt

.. _configuration_runtime_configuration:

=====================
Runtime configuration
=====================

eCAL provides an interface to access and modify its options before initialization. 
The corresponding structure reflects the configuration file (:ref:`configuration_options`). 

Custom types
============

In order to rule out configuration errors, custom datatypes for IP addresses (IpAddressV4) and sizes (ConstrainedInteger) are introduced.

**IpAddressV4:** For assigning an IP address simply assign a string with the desired address. 
Decimal and hexadecimal format is supported. 
In case the IP address is not valid, the type will throw a std::invalid_argument exception.

The IP address can be used like a normal string object. For example:

.. code-block:: c++

  eCAL::Types::IpAddressV4 ip_address = "192.168.7.1"; // in hex: "C0.A8.7.1"
  std::cout << ip_address << "\n"; 

**ConstrainedInteger**: ConstrainedInteger are specified with a minimum (default: 0), step (default: 1) and maximum (default: maximum of int) value.
In case the assigned value does not fit into the specified limitation, the type will throw a std::invalid_argument exception.

The size object can be used like a normal integer.

.. code-block:: c++

  eCAL::Types::ConstrainedInteger<1024, 512, 8192> size_4mb = 1024 + 6 * 512;
  std::cout << size_4mb << "\n";

For specifying sizes in the ecal configuration object, refer to the .ini file or "ecal/config/configuration.h" for the limitations.

Global configuration initialization
===================================

The configuration will be first initialized with the default values specified by eCAL.
If you want to use the systems eCAL .ini file, call the ``InitConfigWithDefaultYaml()`` function of the config object.

In case the .ini to use is specified via command line parameter, this one is chosen instead. 
The object will throw an error, in case the specified .ini file cannot be found.

It is also possible to specify the .ini by calling the function ``checkForValidConfigFilePath(const std::string yaml_path_)`` of the config object.

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

