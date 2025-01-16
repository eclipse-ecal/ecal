.. include:: /include.txt

.. _transport_layers:

================
Transport Layers
================

eCAL is able to communicate on different so called transport layers, but first of all you have to decide if you want to communicate in a network or in a local host only mode.
To configure this you need to set the ecal.ini [network/network_enabled] parameter to true (network communication mode) or false (local host only communication mode).

After this you can fine tune the way of message transport for inner-process, interprocess and the interhost connections.
There are different ways to configure these layers.
They can be set up for a whole machine using the central configuration file (ecal.ini) or for a single eCAL process passed by command line arguments or finally for a single publish-subscribe connection using the C++ or python publisher API.
Every single builtin transport layer has it's specific communication properties.

+-----------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+
| Layer           | ini parameter            | Physical Layer     | Comment                                                                                   |
+=================+==========================+====================+===========================================================================================+
| inproc          | [publisher/use_inproc]   | inner process      | inner process, zeroy copy communication (pointer forwarding)                              |
+-----------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+
| shm             | [publisher/use_shm]      | shared memory      | interprocess, shared memory communication, supports N:M connections, 2 memory copies      |
+-----------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+
| udp_mc          | [publisher/use_udp_mc]   | udp multicast      | interhost, topic name based dynamic multicast grouping to optimize pub/sub socket payload |
+-----------------+--------------------------+--------------------+-------------------------------------------------------------------------------------------+

Every layer can set up in 3 different activation modes. Every mode can be configured as default in the ecal.ini file and can be overwritten by the C++/Python publisher API. This is the activation logic

- off: layer is swicthed off
- on: layer is always switched on (i.e. payload will be send no matter if there is any local or network subscription)
- auto: layer will be switched on autmatically

  - inproc = 2 : layer used automatically for inner process subscribers
  - shm = 2 : layer used automatically for inter process subscribers
  - udp_mc = 2 : layer used autmatically for inter host (network) subcribers

Independent from this publisher setting you can switch on/off the receiving (subscription) logic for every layer.
That means you can prevent incoming payload on specific layers.
This can be done in the ecal.ini file [network] section.

- inproc_rec_enabled = true / false : enable / disable inner process subscriptions
- shm_rec_enabled = true / false : enable / disable inter process subscriptions
- udp_mc_rec_enabled = true / false : enable / disable inter host subscriptions