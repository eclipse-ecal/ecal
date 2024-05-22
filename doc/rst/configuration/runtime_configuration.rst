.. include:: /include.txt

.. _configuration_runtime_configuration:

=====================
Runtime configuration
=====================

eCAL provides an interface to access and modify its options before initialization. 
The corresponding structure reflects the configuration file (:ref:`configuration_options`). 

Custom types
============

In order to exclude configuration errors, custom datatypes for IP addresses and sizes (constrained integer) are introduced.

For assigning an ip address simply assign a string with the desired address. 
Decimal and hexadecimal format is supported. 
In case the ip address is not valid, the type will throw a std::invalid_argument exception.

The ip address can be used like a normal string object. For example:

.. code-block:: c++

  eCAL::Config::IpAddressV4 ip_address = std::string("192.168.7.1"); // in hex: "C0.A8.7.1"
  std::cout << ip_address << "\n"; 

ConstrainedInteger are specified with a minimum (default: 0), step (default: 1) and maximum (default: maximum of int) value.
In case the assigned value does not fit into the specified limitation, the type will throw a std::invalid_argument exception.

The size object can be used like a normal integer.

.. code-block:: c++

  eCAL::Config::ConstrainedInteger<1024, 512, 8192> size_4mb = 1024 + 6 * 512;
  std::cout << size_4mb << "\n";

For specifying sizes in the ecal configuration object, refer to the .ini file or "ecal/config/ecal_configuration.h" for the limitations.

Initialization of the configuration
===================================

The configuration will be first initialized with the default values specified by eCAL.
If you want to use the systems eCAL .ini file, call the InitConfigWithDefaultIni() function of the config object.

In case the .ini to use is specified via command line parameter, this one is chosen instead. 
The object will throw an error, in case the specified .ini file cannot be found.

It is also possible to specify the .ini by calling the function InitConfig(std::string _ini_path) of the config object.

* |fa-file-alt| :file:`hello_config/main.cpp`:

  .. literalinclude:: src/hello_config/main.cpp
     :language: cpp
     :linenos:

