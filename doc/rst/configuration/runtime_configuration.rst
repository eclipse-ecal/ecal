.. include:: /include.txt

.. __configuration_runtime_configuration:

=====================
Runtime configuration
=====================

eCAL provides an interface to access and modify its options before initialization. The corresponding structure reflects the configuration file (:ref:`configuration_options`). 

Initialization of the configuration
===================================

The configuration will be first initialized with the default .ini file found in the system. In case the .ini to use is specified vial command line parameter, this one is chosen instead.
If it cannot find any .ini file, default values will be used for the first initialization of the configuration.

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: src/hello_config/main.cpp
     :language: cpp
     :linenos: