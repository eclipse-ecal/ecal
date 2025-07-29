.. include:: /include.txt

.. _getting_started_howto_pubsub:

===============================
Pubsub: Publisher & Subscriber
===============================

Exchanging data between different processes is one of the main use cases of eCAL.
The most frequently used communication pattern for eCAL is the Publisher-Subscriber pattern.
In this section you will learn how to utilize the eCAL Publisher and Subscriber classes to send and receive data.
You will find examples for all supported language APIs that will help you understand the basic principles of setting up a publisher/subscriber based communication.

All samples written for this chapter can be used across language boundaries.
This means that a C# binary blob receiver can receive data from C binary blob sender.
Likewise a Python Protobuf subscriber can receive data from a C# Protobuf publisher (and vice versa).

At eCAL level, all publishers and subscribers send and receive binary data.
This basic form of sending data is explained in the section :doc:`pubsub/binary_blob`.

On top of those binary publishers and subscribers, eCAL has implemented (de-)serialization for popular serialization formats for 

* Native Strings :doc:`pubsub/string_hello_world`
* Google Protobuf :doc:`pubsub/protobuf_person`
* Cap'n Proto :doc:`pubsub/capnproto_addressbook`
* FlatBuffers :doc:`pubsub/flatbuffers_monster`

This allows the user to conveniently send structured data, as eCAL takes care of the serialization and deserialization.

Users can easily extend eCAL by providing custom (de-)serialization implementations for their own data structures.

.. toctree::
   :hidden:

   pubsub/binary_blob
   pubsub/string_hello_world
   pubsub/protobuf_person
   pubsub/capnproto_addressbook
   pubsub/flatbuffers_monster