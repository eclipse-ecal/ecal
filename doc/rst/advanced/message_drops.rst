.. include:: /include.txt

.. _message_drops:

=============
Message Drops
=============

eCAL operates on a best-effort paradigm, meaning that it will try to deliver all messages, but never guarantee it.
On certain occasions, messages may be dropped.
Drops can e.g. occur on the transport layer, or in the applications layer.

This chapter aims to give you some insight on how drops can occur and what you can do against it.

Dropping on Transport Layer
===========================

- On **UDP**:

  - The send-buffer of the network stack may overflow and drop packages / fragment of packages.
    If a fragment of an eCAL Message is lost, the entire message is lost, as eCAL cannot reconstruct it anymore.

  - While transmitting, UDP frames are sent "best effort".
    This also means that things can go wrong, and a packet can be lost or transmitted incorrectly.
    Again, the entire eCAL message must be dropped then.

  - The receive-buffer of the network stack may overflow.
    Again, the entire eCAL message must be dropped, in this case.

  - Having a higher link speed (10 Gbit vs 1 Gbit for instance) increases the chance of buffers to overflow, as the data is put 10x as fast.

  - Having different link speeds in your network will almost always cause many drops for large messages, as you are e.g. sending with 10 Gbit to a 1Gbit port and so 90% of all packages are dropped.
    This has to be viewed on a traffic-per-message-basis, not on a traffic-per-second basis. For instance, you may be sending only 1 message per second from a 10Gibt link. This message is maybe sent in 10ms, while the 1 Gbit link would need 100ms to receive all the data.
    Consequently, the 1Gbit connection will inevitably start dropping fragments, even though the network is not even close to its capacity.
    You can work around it by just connecting everything with the same link-speed, and by using the ``bandwidth_max_udp`` setting from the :ref:`ecal.yaml <configuration_options>`

  - Larger eCAL Messages are more likely to be corrupt, as they consist of more fragments.

  - You can tune the eCAL UDP Layer by tuning the UDP settings of your operating system.
    Usually this means to just increase the send- and receive-buffer sizes, in order to be better prepared for bursts of data.
    Keep in mind, that you may have a Network Switch in your topology, which also has to buffer and re-transmit the data.

- On **TCP**: 

  - TCP guarantees that every part of a message is actually transmitted.
    However, if your publisher sends faster than the link speed can handle, entire messages are dropped before even sending them.

  - The TCP Layer performs significantly better for large messages than UDP.
    While TCP has a great protocol overhead compared to UDP, the fact that there will be almost no "garbage traffic" (-> fragments that cannot be used, as 1 fragment of the message has been lost) is an enormous advantage here.

  - The TCP congestion control can handle different link speeds just fine.
    If there is one slow connection involved, everything will just slow down.

  - TCP only handles 1:1 connections.
    If you create :math:`n` subscribers for a TCP publisher, the data has to be transferred :math:`n` times.

  - Even messages that have successfully been transmitted via TCP may be dropped on application layer if the subscriber is too slow. See the :ref:`next section <message_drops_application_layer>` for details.

- On **Shared Memory**: 

  - There should be no dropping in the sender-side of eCAL SHM

  - The subscriber still has to read the data from the SHM file.
    If it is too slow, the publisher may have already updated the data and the last message is lost.
    (Also see "Dropping in application layer")

  - The SHM Layer supports an acknowledgement to prevent message drops.
    This will however inevitably cause the publisher to slow down, if a subscriber cannot process the message in time.
    Check out :ref:`this section <transport_layer_shm_memfile_ack>` for more information.

.. _message_drops_application_layer:

Dropping in application layer
=============================

- Even if a message has been received, if may still be dropped by eCAL, if the Subscriber callback is still running.
  So, callbacks that need a lot of computation time are prone to more drops.
  If that is OK or undesirable depends on your application and the type of data.

- eCAL will not buffer data for you.
  But you can easily do that yourself from the callback by just moving the data to some queue, triggering some kind of worker thread and directly returning from the callback execution.
