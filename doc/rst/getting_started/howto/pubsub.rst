.. include:: /include.txt

.. _getting_started_howto_pubsub:

===================
PublisherSubscriber
===================

Exchanging data between different processes is one of the main use cases of eCAL.
In this section you will learn how to utilize the eCAL Publisher and Subscriber classes to send and receive data.
You will find examples for C++, C, C# and Python that will help you understand the basic principles of setting up a publisher/subscriber based communication.

At eCAL level, all publishers and subscribers send and receive binary data.
This basic form of sending data is explained in the section ":ref:`_getting_started_howto_pubsub_binary_blob`".

On top of those binary publishers and subscribers, eCAL has implemented (de-)serialization for popular serialization formats for 

* Native Strings ":ref:`_getting_started_howto_pubsub_string_hello_world`".
* Google Protobuf ":ref:`_getting_started_howto_pubsub_protobuf_person`".
* FlatBuffers ":ref:`_getting_started_howto_pubsub_flatbuffers`".
* Cap'n Proto ":ref:`_getting_started_howto_pubsub_capnproto`".

This allows the user to conveniently send structured data, as eCAL takes care of the serialization and deserialization for you.

Users can easily extend eCAL by providing custom (de-)serialization implementations for their own data structures.

.. toctree::
   :hidden:

    pubsub/string_hello_world
    pubsub/protobuf_person
    pubsub/flatbuffers
    pubsub/capnproto
    pubsub/binary_blob
