.. include:: /include.txt

.. _transport_layers:

================
Transport Layers
================

When talking about transport layers, one needs to establish the different scenarios in which data is being exchanged.
eCAL distinguished between three types of communication:

- **Intraprocess Communication**: communication between publishers / subscribers that are located in the same process
- **Interprocess Communication**: communication between publishers / subscribers in different processes, but located on the same host
- **Interhost Communication**: communication between publishers / subscribers that are located on different hosts, inside a network

.. note::
   Per default, interhost communication is turned off by configuration. To configure this you need to set the ecal.yaml [network/network_enabled] parameter to true (network communication mode) or false (local host only communication mode)
   
In general the latency of the data transport increases depending on the "distance" of the communication participants.
The "closer" the participants, the "quicker" the transport.
Interhost communication requires using a network stack to transport said data, which generates an overhead and latency.
Transport on the same host is significantly faster, as shared memory techniques may be applied.
Intraprocess communication may be fastest, as it may require as little as sharing a pointer.

Depending on the communication scenario, eCAL offers different means to transport data between the communication participants, the so called **transport layers**.


Available transport layers
==========================

CAL supports four different transport layers. 
Every single builtin transport layer has it's specific communication properties.
Most of them can be configured additionally.

+----------------------------------------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+
| Layer                                        | ini parameter            | Physical Layer     | Comment                                                                                   |
+==============================================+==========================+====================+===========================================================================================+
| :ref:`shm <transport_layer_shm>`             | [publisher/use_shm]      | shared memory      | interprocess, shared memory communication, supports N:M connections, 2 memory copies      |
+----------------------------------------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+
| :ref:`udp_mc <transport_layer_udp_mc>`       | [publisher/use_udp_mc]   | udp multicast      | interhost, topic name based dynamic multicast grouping to optimize pub/sub socket payload |
+----------------------------------------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+
| :ref:`tcp <transport_layer_tcp>`             | [publisher/use_tcp]      | tcp                | Network (interhost), simulates N:M connections. Meant for single large payloads.          |
+----------------------------------------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+

Default transport layers
========================

Not all transport layers may be used for all communication scenarios, e.g. it's not possible to use shared memory transport on interhost communication.
eCAL provides sensible default transport methods, depending on the type of communication taking place.

+-----------------------------+-----------+------------+------------+
| Header 1                    | shm       | udp_mc     | tcp        |
+=============================+===========+============+============+
| Intraprocess Communication  | default   | available  | available  |
+-----------------------------+-----------+------------+------------+
| Interprocess Communication  | default   | available  | available  |
+-----------------------------+-----------+------------+------------+
| Interhost Communication     |           | default    | available  |
+-----------------------------+-----------+------------+------------+


Configuration of transport layers
=================================

In case the default configuration is not sensible enough, the eCAL user can finetune the configuration.
The layers can be set up

- for a whole machine using the central configuration file (ecal.yaml) 
- for a single eCAL process passed by command line arguments 
- for a single publish-subscribe connection using the C++ or python publisher API.

Every layer can set up in 3 different activation modes. Every mode can be configured as default in the ecal.yaml file and can be overwritten by the C++/Python publisher API. This is the activation logic

- off: layer is switched off
- on: layer is always switched on (i.e. payload will be send no matter if there is any local or network subscription)
- auto: layer will be switched on automatically

  - shm = 2 : layer used automatically for inter process subscribers
  - udp_mc = 2 : layer used automatically for inter host (network) subscribers

Independent from this publisher setting you can switch on/off the receiving (subscription) logic for every layer.
That means you can prevent incoming payload on specific layers.
This can be done in the ecal.yaml file [network] section.

- shm_rec_enabled = true / false : enable / disable inter process subscriptions
- udp_mc_rec_enabled = true / false : enable / disable inter host subscriptions

.. seealso:: 

   .. toctree::
      :maxdepth: 1

      layers/shm.rst
      layers/udp_mc.rst
      layers/tcp.rst
