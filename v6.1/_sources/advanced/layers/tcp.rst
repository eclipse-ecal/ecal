.. include:: /include.txt

.. _transport_layer_tcp:

==============
eCAL TCP Layer
==============

The eCAL TCP Transport Layer has been added with eCAL 5.10.

.. tip:: 

   You can test TCP communication by executing the ``ecal_sample_person_snd_tcp`` samples.
   That sample will **always** send data via TCP, regardless of you system configuration.

   You can use the eCAL Monitor or the regular ``ecal_sample_person_rec`` sample to receive the data.

Motivation
==========

The eCAL TCP Layer has been added with having single **large messages** in mind (let's say 1 Megabyte or more).
These messages e.g. occur when transmitting camera images or anything else where each individual payload may be big.

Let's compare the "normal" UDP Layer with the TCP Layer and assume transferring a 16 MiB frame.

- **UDP Multicast**

  UDP is wrapped with an IP and an Ethernet frame.
  The Ethernet MTU however is only 1518 bytes, which usually leaves at most 1472 bytes for the actual payload.
  Therefore, the 16 MiB message results in more than 11000 Ethernet frames.

  If your network stack now loses only one of those, eCAL will have to drop the whole message; as UDP has no recovery technique for that case.

  .. note::

     The larger the messages, the greater is the chance of one part of it getting lost!
     
     With large messages like in the example you will quickly run into the situation, that basically every message suffers from a loss of at least 1 packet, so all messages have to be dropped.

- **TCP**
  
  TCP is a single-cast protocol that automatically recovers message drops and handles conguestion avoidance.
  If one of our 11000 Ethernet packets is lost, TCP will automatically take care of that and make sure the whole message is transmitted without errors.

  On the downside, TCP will have to transmit the same message n-times for n subscribers.
  Additionally, TCP is often regarded to be too inefficient and have too much latency for realtime publish subscribe applications.

  In our 16 MiB example however, it may be much more important to transmit one entire 16 MiB frame and recover from packet drops.

  .. note::

     eCAL's TCP Layer will make sure that any message that it sends will be transmitted entirely.
     It will however act in a best-effort way and skip messages, if you overload your system.

How to use
==========

Which transport layer will be used for transferring data from a publisher to a subscriber is determined by:

1. **Publisher** settings: The **enabled layers** as well as the **layer priority**

2. **Subscriber** settings: The **enabled layers**

Force TCP via :file:`ecal.yaml`
-------------------------------

TCP can be enabled via the :file:`ecal.yaml` configuration file.
Using the configuration file however has the drawback, that TCP will be used by all nodes that load that particular file for all data transfers.

**By default**, the :file:`ecal.yaml` should look as follows:

.. code-block:: yaml

   publisher:
     layer:
       # [...]
       udp:
         enable: true
       tcp:
         enable: true                  # TCP already enabled for publishing
     priority_network: ["udp", "tcp"]  # Default: publisher prioritize UDP over TCP!!!

   subscriber:
     layer:
       # [...]
       udp:
         enable: true
       tcp:
         enable: false                 # Default: TCP disabled for subscribers

In order to make eCAL use TCP, you have to **enable TCP for subscribing** and also either change the **publisher's layer priority** or **disable UDP for subscribing**.

For example, you can change the :file:`ecal.yaml` to the following:

.. code-block:: yaml

   publisher:
     # [...]
     priority_network: ["tcp", "udp"]  # Change priority: publisher prioritize TCP over UDP!!!

   subscriber:
     layer:
       # [...]
       udp:
         enable: false                 # Disable UDP for subscribing
       tcp:
         enable: true                  # Enable TCP for subscribing

Force TCP from your application code
------------------------------------

Usually, you can somewhat estimate the size of your messages when writing your publisher code.
Therefore, we recommend enabling TCP from the eCAL Publisher API only for those that actually benefit from that:

.. code-block:: cpp

   #include <ecal/config/publisher.h>

   // [...]

   // Create a publisher configuration object
   eCAL::Publisher::Configuration pub_config;

   // Set enable shm and tcp, disable udp layer
   pub_config.layer.shm.enable = true;
   pub_config.layer.udp.enable = false;
   pub_config.layer.tcp.enable = true;

   // Create a publisher (topic name "person") and pass the configuration object
   eCAL::protobuf::CPublisher<pb::People::Person> pub("person");

   // [...]

