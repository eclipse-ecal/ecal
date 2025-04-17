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


To support one to many publisher/subscriber connections, the publisher creates one named update event per connection.
   
.. note::

   In the standard configuration there is no guarantee that all subscribers have copied the payload before a new message is written to id.
   If a publisher sends its payload faster than the subscriber can copy it, it will be overwritten and the subscriber will only be informed by the message counter that a message has been dropped. 
   You can check for dropped messages in the eCAL Monitor application.

Configuration
=============

The SHM Layer is set to ``enable`` by default.

The system-configuration-parameters in the :file:`ecal.yaml` are:

.. code-block:: yaml

  # Publisher specific base settings
  publisher:
    layer:
    # Base configuration for shared memory publisher
      shm:
        # Enable layer
        enable: true
        [..]

There are a few options for tweaking the communication.
Those options are explained below.

.. _transport_layer_shm_memfile_ack:

Handshake mechanism (optional)
------------------------------

Most applications perform very well with the default behavior. 
If subscribers are too slow to process incoming messages then the overall software architecture needs to be checked, software components need to be optimized or parallelized.

There may still be cases where it could make sense to synchronize the transfer of the payload from a publisher to a subscriber by using an additional handshake event.
This event is signaled by a subscriber back to the sending publisher to confirm the complete payload transmission.

The handshake mechanism can be activated in the :file:`ecal.yaml`:

.. code-block:: yaml

  # Publisher specific base settings
  publisher:
    layer:
    # Base configuration for shared memory publisher
      shm:
        [..]
        # Force connected subscribers to send acknowledge event after processing the message.
        # The publisher send call is blocked on this event with this timeout (0 == no handshake).
        acknowledge_timeout_ms: 5
        [..]

If the parameter is set to a non-zero timeout, the publisher will create an additional event and inform the subscriber to fire this event when the transmission of the payload is completed.

The publisher will now wait up to the specified timeout for the acknowledge signals of the connected subscribers after every memory file content update before writing new content.
Finally that means the publishers ``CPublisher::Send`` API function call is now blocked and will not return until all subscriber have read their content or the timeout has been reached.

Zero Copy mode (optional)
-------------------------

.. note::

   Zero-copy has been added in eCAL 5.10 for subscription and in 5.12 for publishing.
   It is turned off by default.
   When turned on, old eCAL Versions can still receive the data but will not use zero-copy.

The "normal" eCAL Shared memory communication results in the payload being copied at least twice:

1. Into the SHM file by the publisher

2. From the SHM file the private memory of each subscriber

Copying the payload from the memory file before executing the subscriber callback results in **better decoupling**, so the publisher can update the memory file with the next message while the subscriber is still processing the last one.
**Small messages** will be transmitted in a few microseconds and will not benefit from zero-copy.

If it comes to very **large messages** (e.g. high resolution images) however, copying really matters and it can make sense to activate eCAL's **zero-copy mode**.

.. seealso:: 

   Check out the following Chapter to learn how to enable zero-copy.
   The chapter will also teach you about advantages and disadvantages of zero-copy:

   .. toctree:: 
      :maxdepth: 1

      shm_zerocopy.rst

Multi-buffering mode (optional)
-------------------------------

.. note:: 

   Multi-buffering has been added in eCAL 5.10.
   Multi-buffered topics cannot be received by older eCAL versions.
   The feature is turned off by default.

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

  Edit your :file:`ecal.yaml` and set a buffer count greater than 1:

  .. code-block:: yaml
     
    # Publisher specific base settings
    publisher:
      layer:
      # Base configuration for shared memory publisher
        shm:
          [..]
          # Maximum number of used buffers (needs to be greater than 0, default = 1)
          memfile_buffer_count: 3

- **Use multi-buffering for a single publisher (from your code):**

  Multibuffering can be enabled for a specific publisher using this ``CPublisher`` `API function <https://eclipse-ecal.github.io/ecal/_api/classeCAL_1_1CPublisher.html#_CPPv4N4eCAL10CPublisher17ShmSetBufferCountEl>`_:

  .. code-block:: cpp
      
     #include <ecal/config/publisher.h>

    ...

    // Create a publisher configuration object
    eCAL::Publisher::Configuration pub_config;

    // Set the option for buffer count in layer->shm->memfile_buffer_count to 3, so it will create 3 SHM files
    pub_config.layer.shm.memfile_buffer_count = 3;

    // Create a publisher (topic name "person") and pass the configuration object
    eCAL::protobuf::CPublisher<pb::People::Person> pub("person", pub_config);

    ...


Combining the zero-copy feature with an increased number of memory buffer files (like 2 or 3) could be a nice setup allowing the subscriber to work on the memory file content without copying its content and nevertheless not blocking the publisher to write new data.
Using Multibuffering however will force each Send operation to re-write the entire memory file and disable partial updates.