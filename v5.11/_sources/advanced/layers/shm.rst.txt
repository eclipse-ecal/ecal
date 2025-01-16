.. include:: /include.txt

.. _transport_layer_shm:

==============
eCAL SHM Layer
==============

The eCAL Shared Memory (SHM) Layer is the default transport layer for publishers and subscribers running on the same machine.
It is based on a shared memory space (therefore the name) that is used to write the data to and again read it from.
This technique ensures both low latency and high throughput - as long as the publisher and subscriber are running on the same machine. 

How does it work
================

By default, the logic is implemented in the following way:

Initialization phase:

* The publisher creates a memory file fitting to the size of the current payload.

* The publisher creates a named shared mutex and a named event used as synchronization primitives.

* The publisher informs all matching subscribers about the name of the memory file, the matching mutex and the update event.

* The subscriber starts observing the memory file for updates by listening to the update event.
   
Communication phase (default configuration):

* The publisher acquires the memory file access-mutex and opens the memory file.

* The publisher writes the payload content and some header information into the memory file.

* The publisher fires the update event, closes the memory file and unlocks the mutex.

* The subscribers are signaled by the update event and acquire the access-mutex.

* The subscribers open the memory file and copy the payload into their process memory.

* The subscribers close the memory file and release the access-mutex.


To support one to many publisher/subscriber connections the publisher creates in fact one named update event per connection.
   
.. note::

   In the standard configuration there is no guarantee that all subscribers have copied the payload before a new message is written to id.
   If a publisher sends its payload faster than the subscriber can copy it, it will be overwritten and the subscriber will only be informed by the message counter that a message has been dropped. 
   You can check for dropped messages in the eCAL Monitor application.

Configuration
=============

The SHM Layer is set to ``auto`` (= 2) by default.
This means, that it is used automatically for all messages that need to be transmitted inside a single host.

The system-configuration-parameters in the :file:`ecal.ini` are:

.. code-block:: ini

   [publisher]
   use_shm                   = 2


There are a few options for tweaking the communication.
Those options are explained below.

.. _transport_layer_shm_memfile_ack:

Handshake mechanism (optional)
------------------------------

Most applications perform very well with the default behavior. 
If subscribers are too slow to process incoming messages then the overall software architecture needs to be checked, software components need to be optimized or parallelized.

There may still be cases where it could make sense to synchronize the transfer of the payload from a publisher to a subscriber by using an additional handshake event.
This event is signaled by a subscriber back to the sending publisher to confirm the complete payload transmission.

The handshake mechanism can be activated in the :file:`ecal.ini`:

.. code-block:: ini

   [publisher]
   ; activate synchronization via memory transfer acknowledge signal with a timeout of 100 milliseconds
   memfile_ack_timeout = 100

If the parameter is set to a non-zero timeout, the publisher will create an additional event and inform the subscriber to fire this event when the transmission of the payload is completed.

The publisher will now wait up to the specified timeout for the acknowledge signals of the connected subscribers after every memory file content update before writing new content.
Finally that means the publishers ``CPublisher::Send`` API function call is now blocked and will not return until all subscriber have read their content or the timeout has been reached.

Zero Copy mode (optional)
-------------------------

*Zero-copy has been added in eCAL 5.10. It is turned off by default. When turned on, old eCAL Version can still receive the data but will not use zero-copy.**

The “normal” eCAL Shared memory communication results in the payload being copied at least twice:

1. Into the SHM file by the publisher

2. From the SHM file the private memory of each subscriber

Usually there is no issue with that.
Copying the payload from the memory file before executing the subscriber callback results in better decoupling, so the publisher can update the memory file with the next message while the subscriber is still processing the last one.
Small messages will be transmitted in a few microseconds and will not benefit from zero-copy.

If it comes to very large messages (e.g. high resolution images) however, copying really matters and it can make sense to activate eCAL's zero-copy mode.
With zero-copy, the communication would look like this:

1. The publisher still has to copy the data into the memory file.

2. The subscriber executes its callback directly on the memory file.
   The memory file is blocked, while being used.

   .. warning::
      
      The memory file is blocked for new publications as long as the user’s callback is processing its content.
      It will also block other subscribers from reading the same SHM file.

3. The subscriber releases the memory file, so the publisher can update it again.

.. note::

   Even though it is called zero-copy, only the subscribers are zero-copy.
   Publishers still have to copy the data into the memory file, as they have to also support other layers like UDP or TCP and therefore cannot directly work on the memory file.

Zero-copy can be enabled in the following ways:

- **Use zero-copy as system-default (not recommended!):**

  Activating zero copy system-wide is not recommended because of the mentioned disadvantages for small payloads.
  But if this is wanted for reasons it can be done by adapting your :file:`ecal.ini` like this.

  .. code-block:: ini

     [publisher]
     memfile_zero_copy = 1

- **Use zero-copy for a single publisher (from your code):**

  Zero copy could be activated either per connection or for a complete system using the eCAL configuration file.
  To activate it for a specific publisher this ``CPublisher`` `API function <https://eclipse-ecal.github.io/ecal/_api/classeCAL_1_1CPublisher.html#_CPPv4N4eCAL10CPublisher17ShmEnableZeroCopyEb>`_ needs to be called.

  .. code-block:: cpp

     // Create a publisher (topic name "person")
     eCAL::protobuf::CPublisher<pb::People::Person> pub("person");

     // Enable zero-copy for this publisher
     pub.ShmEnableZeroCopy(true);

.. note::

   In general, it is advisable to combine zero-copy with multi-buffering to reduce the impact on the publisher.

Multi-buffering mode (optional)
-------------------------------

*Multi-buffering has been added in eCAL 5.10.
Multi-buffered topics cannot be received by older eCAL versions.
The feature is turned off by default.*

As described in the previous sections, eCAL uses one shared memory file per publisher. This can lead to performance reduction if

* the memory file is blocked by a subscriber that is copying (non-zero copy) or working on (zero-copy) the content 

* the memory file is blocked because many connected subscribers acquire read access

* a combination of all

This issue can be relaxed by using multiple memory files per topic, so the publisher can already write the next payload, while some subscribers still keep the previous memory file locked. 
These files work like an SHM-file ring buffer.

.. important::

   Activating the feature will allocate more RAM and increase the number of open file handles!

You can activate the feature in the following ways.

- **Use multi-buffering as system-default**:

  Edit your :file:`ecal.ini` and set a buffer count greater than 1:

  .. code-block:: ini
     
     [publisher]
     memfile_buffer_count      = 3

- **Use multi-buffering for a single publisher (from your code):**

  Multibuffering can be enabled for a specific publisher using this ``CPublisher`` `API function <https://eclipse-ecal.github.io/ecal/_api/classeCAL_1_1CPublisher.html#_CPPv4N4eCAL10CPublisher17ShmSetBufferCountEl>`_:

  .. code-block:: cpp
      
     // Create a publisher (topic name "person")
     eCAL::protobuf::CPublisher<pb::People::Person> pub("person");

     // Set multi-buffering to 3, so it will create 3 SHM files
     pub.ShmSetBufferCount(3);

Combining the zero-copy feature with an increased number of memory buffer files (like 2 or 3) could be a nice setup allowing the subscriber to work on the memory file content without copying its content and nevertheless not blocking the publisher to write new data.
