.. include:: /include.txt

.. _transport_layer_shm_zerocopy:

==============
eCAL Zero Copy
==============

.. note:: 

   The eCAL Zero Copy mode has been added in:

   - eCAL 5.10 (Zero-copy **subscriptions**)
   - eCAL 5.12 (Zero-copy **publishing**)
  
   It is turned off by default.

Enabling eCAL Zero Copy
=======================

- **Use zero-copy as system-default:**

  Zero Copy can be enabled as system default from the :file:`ecal.ini` file like follows:

  .. code-block:: ini

     [publisher]
     memfile_zero_copy = 1

- **Use zero-copy for a single publisher (from your code):**

  Zero-copy can be activated for a single publisher from the eCAL API:

  .. code-block:: cpp

     // Create a publisher (topic name "person")
     eCAL::protobuf::CPublisher<pb::People::Person> pub("person");

     // Enable zero-copy for this publisher
     pub.ShmEnableZeroCopy(true);

  Keep in mind, that using protobuf for serialization will still:

  #. Require to set the data in the protobuf object
  #. Later cause a copy by serializing into the SHM buffer.

  If you want to avoid this copy, you can use the :ref:`low-level API <transport_layer_shm_zerocopy_low_level>` to directly operate on the SHM buffer.

Zero Copy behavior
==================

The bevior for publishers differs depending on the used transport layers.
When publishing data over network, those connections cannot make use of any Zero copy features.


Shared-Memory-only connection
-----------------------------

This describes the case, where a publisher publishes it's data **only via shared memory** to 1 or more subscribers.

**Publisher**:

- Protobuf:
  
  #. The user sets the data in the **protobuf object**

  #. The publisher **locks** the SHM buffer.

     *This operation may take some time, as the publisher needs to wait for the subscriber to release the buffer.*
     *This can be relaxed by using th multi-buffering feature.*

  #. The publisher **serializes** the protobuf object **directly into the SHM** buffer

     *Due to the technical implementation of protobuf, this will cause the entire message to be serialized and re-written*

  #. The publisher **unlocks** the SHM buffer

- Low Level Memory access:

  #. The publisher **locks** the SHM buffer

  #. The user **directly** writes data to the **SHM buffer**.

  #. The publisher **unlocks** the SHM buffer

**Subscriber**:

#. The subscriber **locks** the SHM buffer

   *The subscriber will need to wait for any publisher and subscriber to unlock the buffer.*
   *Currently, there is no parallel read access to the SHM buffer.*

#. The subscriber calls the **callback** function directly with the **SHM buffer** as parameter

#. After the callback has **finished**, the subscriber **unlocks** the SHM buffer

Mixed Layer connection
----------------------

This describes the case where a publisher publishes it's data via **shared memory and network**.

**Publisher**:

The publisher cannot work in SHM Zero Copy mode, if there are non-SHM connections.
In this case, the publisher will fall back to the **normal SHM** mode.

#. For Protobuf: The user sets the data in the **protobuf object**

#. For Low Level Memory access: The user modifies parts of the SHM buffer. 

#. The publisher **locks** the SHM buffer

#. The publisher **copies** the private SHM buffer or the serialized protobuf object to all data layers, one of which is the SHM buffer.

#. The publisher **unlocks** the SHM buffer

**Subscriber**:

Subscribers will always use Zero Copy, if enabled.
So they will **directly** read from the SHM buffer.

#. The subscriber **locks** the SHM buffer

#. The subscriber calls the **callback** function directly with the **SHM buffer** as parameter

#. After the callback has **finished**, the subscriber **unlocks** the SHM buffer

.. _transport_layer_shm_zerocopy_low_level:

Low Level Memory Access
=======================

For unleashing the full power of eCAL Zero Copy, the user needs to directly work on the eCAL Shared Memory via the ``CPayload`` API.

**Normal Serialization**

- Overwrites entire memory => kind of a 1-copy instead of 2-copy approach.

- Easy to use, ships with eCAL

- This is meant for message serializations like protobuf.
  Those can then directly serialize into the SHM buffer.

**Partial Serialize**

- Despite the name, this this the **Direct memory access** method, that is meant for **non-serialization** message formats

- Has possibility to only change parts of the memory

- Always operates on 2 Buffers that **needs to be kept in sync**

  - There is a "private" buffer in the CPayload that is modified

  - There is a "public" SHM buffer that the user needs to **manually** keep in sync with the private buffer via partial serialize

  - **The user needs to make sure that both buffers are kept in sync. eCAL does not offer any help with that. A call to PartialSerialize always needs to make sure that the public buffer is updated to the private buffer.**

- Example:

  - User writes 1 byte of the message (-> is written in private buffer)

  - User writes 1 other byte of the message (-> also written in private buffer)

  - User calls ``Send()``

  - eCAL calls the ``PartialSerialize()`` function that the User needs to implementat

  - The ``PartialSerialize()`` now needs to know which bytes have been changed prior to to the ``Send()`` call and also change those in the public buffer

Zero Copy vs. normal eCAL SHM
=============================

.. list-table:: Zero Copy vs. normal eCAL SHM
   :widths: 10 45 45
   :header-rows: 1
   :stub-columns: 1

   * -

     - eCAL SHM - Default

     - eCAL SHM - Zero Copy

   * - Memcopies

     - ❌ 2 additional memcpy (1 for publishing, 1 for each subscriber)

     - ✅ No memcpy (if Low Level API is used)

   * - Partial changes

     - ❌ Changing only 1 byte causes the entire updated message to be copied to the buffer, again

     - ✅ Changing only 1 byte only costs as much as changing that 1 byte in the target memory, independent from the message size

   * - Subscriber decoupling

     - ✅ Good decoupling between subscribers.
       Subscribers only block each other for the duration of that 1 memcpy

     - ❌ Subscribers need to wait for each other to finish their callbacks

   * - Pub/Sub decoupling

     - ✅ Good decoupling between publisher and subscribers.
       
       - If the serialization takes a long time, this can be done beforehand without having a lock on the SHM buffer

       - Publishers don't have to wait for the subscribers to finish their callbacks, only for them to copy the data to their own process memory
     
     - ❌ Subscribers may block publishers
     
       - Publishers need to wait for all subscriber callbacks to finish

       - Publishers need to keep the the SHM buffer locked while performing the message serialization

Combining Zero Copy and Multibuffering
======================================