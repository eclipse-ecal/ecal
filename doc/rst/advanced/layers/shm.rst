.. include:: /include.txt

.. _transport_layer_shm:

==============
eCAL SHM Layer
==============

Motivation
==========

eCAL is designed for transmitting payloads between publisher and subscribers with low latency. For complex, distributed software applications like an autonomous driving control stack this communication performance is the base to guarantee the quality and the reaction time of the overall system.

eCAL ensures this performance by providing different transport layers for inter-process and inter-host communication. For local inter-process communication payloads are transmitted by using shared memory files that are written by publishers and read out by connected subscribers.

How does it works
-----------------

**Standard configuration**

By default (standard configuration) the logic is implemented that way:

Initialization phase:

  1. The publisher creates a memory file fitting to the size of the current payload.
  2. The publisher creates a named shared mutex and a named event used as synchronization primitives.
  3. The publisher informs all matching subscribers about the name of the memory file, the matching mutex and the update event.
  4. The subscriber starts observing the memory file for updates by listening to the update event.
    
Communication phase (default configuration):

  1. The publisher acquires the memory file access mutex and opens the memory file.
  2. The publisher writes the payload content and some header information into the memory file.
  3. The publisher fires the update event, closes the memory file and unlocks the mutex.
  4. The subscribers is signaled by the update event and acquires the access mutex.
  5. The subscriber opens the memory file and copies the payload into it's process memory.
  6. The subscriber closes the memory file and releases the access mutex.

To support one to many publisher/subscriber connections the publisher creates in fact one named update event per connection. In the standard configuration there is no guarantee, no check if all subscriber could make a copy of the payload in time or not. That means if a publisher will send payloads with a frequency that is higher than a connected subscriber can process (copy out) the payload then the content will be overwritten. In this case the subscriber will be informed by a simple counting mechanism that messages where dropped. You can check for dropped messages in the eCAL Monitor application.

**Using an additional handshake mechanism**

Most of the applications perform very well with the described default behavior of eCAL's shared memory layer. If subscriber are in general to slow to process incoming messages then the overall software architecture needs to be checked, software components needs to be optimized or parallelized. But there are use cases where it could make sense to synchronize the transfer of the payload from a publisher to a subscriber by using an additional handshake event. This event is signaled by a subscriber back to the sending publisher to confirm the complete payload transmission.

The handshake mechanisms needs to be activated in the eCAL configuration file (ecal.ini) that way.

.. code-block:: ini

  [publisher]
  ; activate synchronization via memory transfer acknowledge signal with a timeout of 100 milliseconds
  memfile_ack_timeout = 100

By default the parameter `memfile_ack_timeout` is set to zero. That means no additional handshake event is created and used. If that parameter is set to a specific timeout then the publisher will create an additional event and inform the subscriber to fire this event when the transmission of the payload is completed.

The publisher will then wait up to the specified timeout for the acknowledge signals of the connected subscribers after every memory file content update before writing new content. Finally that means the publishers `CPublisher::Send` API function call is now blocked and will not return before all subscriber read their content or timeout has reached.

**Almost true zero copy**

As described in the standard configuration behavior, eCAL is by default not acting zero copy for reasons. One reason is simplicity, there is no demon application needed to assist between acting publisher and subscriber, to establish their connections or to manage a memory file pool. Another reason is to always prefer the low latency decoupling of publisher and subscribers over additional copying of memory. In other words, a publishing process is just writing (copying) content into memory files, fires some signals and can return to work e.g. processing the next image. 

The subscribing processes will start to read out the content in their own process space (so a second copy is performed) and will release this resource back to the system. The publisher can immediately start the next transfer in the background. On modern systems a memory transfer of a hundred kByte should be performed in less then 10 microseconds. So to not decouple connections by copying but to process that small payloads directly will block the sending publisher and will not improve the performance for sure.

But if it comes to very large messages where copying really matters (high resolution, raw images, pointclouds) then it could make sense to switch eCAL's shared memory layer to a kind of zero copy logic. In fact it's still not real zero copy but it's just one copy instead of the described two. Because eCAL is supporting different transport mechanism in parallel like UDP and TCP there is still one copy needed on the publisher side to decouple the transport layers from each other. But let's look how that single copy mechanism is working in detail. The initialization phase is the same as described in the default configuration. The communication phase looks like this now.

Communication phase (Zero copy configuration):
  1.-4. Like described in the default configuration.

  5. The subscriber opens the memory file and calls the connected user callback function without copying the payload.
  6. Like described in the default configuration.

Note that the memory file is blocked for new publications as long as the users callback function is processing it's content.

Zero copy could be activated either per connection or for a complete system using the eCAL configuration file. To activate it for a specific publisher this 'CPublisher' API function needs to be called.

.. code-block:: cpp

  /**
   * @brief Enable zero copy shared memory transport mode.
   *
   * @param state_  Set type zero copy mode for shared memory transport layer (true == zero copy enabled).
   *
   * @return  True if it succeeds, false if it fails.
  **/
  bool ShmEnableZeroCopy(bool state_);

To activate zero copy for a whole system is not recommended because of the mentioned disadvantages for small payloads. But if this is wanted for reasons it can be done by adapting the global eCAL configuration like this.

.. code-block:: ini

  [publisher]
  memfile_zero_copy = 1

**Multibuffering**

BLA BLA
