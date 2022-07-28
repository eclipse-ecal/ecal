.. include:: /include.txt

.. _transport_layer_tcp:

==============
eCAL TCP Layer
==============

The eCAL TCP Transport Layer has been added with eCAL 5.10.

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

The eCAL TCP Layer is a **publisher** setting.
When using network communication, the data is sent **either** via UDP **or** via TCP.
That means, that (by default), any subscriber can receive data from UDP and TCP subscribers (even simultaneously if they send on the same topic).

You can activate TCP in the following ways:

#. **Use TCP as system default (only recommended for testing):**

   Modify the :file:`ecal.ini` and change the following:

   .. code-block:: ini

      [publisher]
      use_tcp                   = 2
      use_udp_mc                = 0
   
   This will

   - Turn on TCP automatically (in the means of automatic switching between Shared Memory and TCP)
   - Turn off UDP Multicast

   .. important::

      While this may be fine for testing, you may run into side effects that you didn't expect, mostly from the fact that each additional subscriber will cause more network usage.
      Transmitting small messages is way more efficient using UDP Multicast.

#. **Use TCP selectively from your code for a single publisher:**
   
   Usually, you can somewhat estimate the size of your messages when writing your publisher code.
   Therefore, we recommend enabling TCP from the eCAL Publisher API only for those that actually benefit from that:

   .. code-block:: cpp

      // Create a publisher (topic name "person")
      eCAL::protobuf::CPublisher<pb::People::Person> pub("person");

      // Switch UDP Multicast layer off
      pub.SetLayerMode(eCAL::TLayer::tlayer_udp_mc, eCAL::TLayer::smode_off);

      // Switch TCP layer on for Network connections (-> smode_uto)
      pub.SetLayerMode(eCAL::TLayer::tlayer_tcp, eCAL::TLayer::smode_auto);


   .. seealso:: 

      Also see the ``person_snd_tcp`` sample:

      https://github.com/eclipse-ecal/ecal/tree/master/samples/cpp/person/person_snd_tcp
