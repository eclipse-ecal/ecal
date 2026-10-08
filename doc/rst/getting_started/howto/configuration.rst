.. include:: /include.txt

.. _getting_started_configuration:

=====================
Runtime Configuration
=====================

In eCAL it is also possible to configure the environment at runtime.
The eCAL API provides a set of structures that reflect the YAML configuration file (:ref:`configuration_options`).

You can pass this configuration structure to ``eCAL::Initialize()``.

But there is more: You can also modify the configuration of each publisher and subscriber individually.
This allows you to be more flexible in the design of your application.

.. important::
    Once set and passed to ``eCAL::Initialize()``, the configuration is set.
    The getters for the configuration structures, e.g. ``eCAL::GetConfiguration()``, return a constant struct.
    So, modifying the configuration object without passing it to ``eCAL::Initialize()`` has no effect.

In our example we will create multiple publishers and subscribers. 
They use the same topic name but communicate through different layers, so they do not interfere with one another.


Cake Publisher
==============

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/misc/cake_send/src/cake_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C
        
        .. literalinclude:: /source_code_samples/c/misc/cake_send/src/cake_send_c.c
            :language: c
            :linenos:
            :lines: 20-

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`cake_send.cpp </source_code_samples/cpp/misc/cake_send/src/cake_send.cpp>`
   │
   └─ |fa-folder-open| C
      └─ |fa-file-alt| :download:`cake_send_c.c </source_code_samples/c/misc/cake_send/src/cake_send_c.c>`


Cake Receiver
=============

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/misc/cake_receive/src/cake_receive.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C
        
        .. literalinclude:: /source_code_samples/c/misc/cake_receive/src/cake_receive_c.c
            :language: c
            :linenos:
            :lines: 20-

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`cake_receive.cpp </source_code_samples/cpp/misc/cake_receive/src/cake_receive.cpp>`
   │
   └─ |fa-folder-open| C
      └─ |fa-file-alt| :download:`cake_receive_c.c </source_code_samples/c/misc/cake_receive/src/cake_receive_c.c>`