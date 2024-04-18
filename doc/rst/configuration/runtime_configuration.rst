.. include:: /include.txt

.. _configuration_runtime_configuration:

=====================
Runtime configuration
=====================

eCAL provides an interface to access and modify its options before initialization. 
The corresponding structure reflects the configuration file (:ref:`configuration_options`). 

Custom types
============

In order to exclude configuration errors, custom datatypes for IP addresses and sizes are introduced.

For assigning an ip address simply assign a string with the desired address. 
Decimal and hexadecimal format is supported. 
In case the ip address is not valid, the type will throw a std::invalid_argument exception.

You can use the ip address like a normal string object. For example:

.. code-block:: c++

  eCAL::Config::IpAddressV4 ip_address = std::string("192.168.7.1"); // in hex: "C0.A8.7.1"
  std::cout << ip_address << "\n"; 

Sizes are specified with a minimum (default: 0), step (default: 1) and maximum (default: maximum of int) value.
In case the assigned value does not fit into the specified limitation, the type will throw a std::invalid_argument exception.

You can use the size object like a normal integer.

.. code-block:: c++

  eCAL::Config::LimitSize<1024, 512, 8192> size_4mb = 1024 + 6 * 512;
  std::cout << size_4mb << "\n";

For specifying sizes in the ecal configuration object, refer to the .ini file or "ecal/types/ecal_config_types.h" for the limitations.

Initialization of the configuration
===================================

The configuration will be first initialized with the default .ini file found in the system. 
In case the .ini to use is specified vial command line parameter, this one is chosen instead.
If it cannot find any .ini file, default values will be used for the first initialization of the configuration.

* |fa-file-alt| :file:`hello_config/main.cpp`:

  .. literalinclude:: src/hello_config/main.cpp
     :language: cpp
     :linenos:

