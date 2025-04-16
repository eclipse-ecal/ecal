.. include:: /include.txt

.. _getting_started_howto_pubsub_protobuf_person:

================
Protobuf: Person
================

Let's implement a small application, that lets the user input his name and send a message to an eCAL topic.

For this task we will use the protobuf serialization format.
Protobuf is a language-neutral, platform-neutral extensible mechanism for serializing structured data.
It is used by Google to serialize data in its internal RPC protocols and file formats.

eCAL supports protobuf serialization natively for C++, C# and Python.

The usage of protobuf for data exchange in eCAL is very simple. You know already from the ":ref:`_getting_started_howto_pubsub_string_hello_world`" how to send and receive simple string data.
The basic setup will be the same, but instead of using the string publisher, we will use the protobuf publisher and subscriber.


Person Protobuf
===============

As the sender and receiver need the same .proto files, we place them in a separate directory next to the source directories for the sender and the receiver.

.. parsed-literal::
   
   |fa-folder-open| Person Protobuf File
   ├─ |fa-file-alt| :download:`person.proto </source_code_samples/cpp/protobuf/person/person_send/src/protobuf/person.proto>`
   │
   ├─ |fa-file-alt| :download:`animal.proto </source_code_samples/cpp/protobuf/person/person_send/src/protobuf/animal.proto>`
   │
   └─ |fa-file-alt| :download:`house.proto </source_code_samples/cpp/protobuf/person/person_send/src/protobuf/house.proto>`


Let's start with the :file:`protobuf/person.proto` file!

.. literalinclude:: /source_code_samples/cpp/protobuf/person/person_send/src/protobuf/person.proto
   :language: protobuf
   :linenos:
   :lines: 20-

As you can see, the ``person.proto`` file imports also other message definitions: ``animal.proto`` and ``house.proto``.
So we need them as well. The definitions are straight forward. For more information about protobuf, please refer to the detailed official documentation.

.. literalinclude:: /source_code_samples/cpp/protobuf/person/person_send/src/protobuf/animal.proto
   :language: protobuf
   :linenos:
   :lines: 20-

.. literalinclude:: /source_code_samples/cpp/protobuf/person/person_send/src/protobuf/house.proto
   :language: protobuf
   :linenos:
   :lines: 20-


Person Publisher
================

The main differences to the string publisher are:
  - we need to include the protobuf publisher from ``ecal/msg/protobuf/publisher.h``
  - also we need the compiled protobuf message header file ``person.pb.h`` and include it
  - we need to utilize the protobuf message class ``Person`` instead of the string class ``std::string``

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/protobuf/person/person_send/src/person_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/protobuf/person/person_send.cs
            :language: csharp
            :linenos:
            :lines: 31-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/protobuf/person/person_send.py
            :language: python
            :linenos:
            :lines: 19-


Person Publisher Files
======================

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`person_send.cpp </source_code_samples/cpp/protobuf/person/person_send/src/person_send.cpp>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`person_send.cs </source_code_samples/csharp/protobuf/person/person_send.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`person_send.py </source_code_samples/python/protobuf/person/person_send.py>`
   

Person Subscriber
=================

For the subscriber the same changes apply as for the publisher.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/protobuf/person/person_receive/src/person_receive.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/protobuf/person/person_receive.cs
            :language: csharp
            :linenos:
            :lines: 31-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/protobuf/person/person_receive.py
            :language: python
            :linenos:
            :lines: 19-


Person Subscriber Files
=======================

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`person_receive.cpp </source_code_samples/cpp/protobuf/person/person_receive/src/person_receive.cpp>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`person_receive.cs </source_code_samples/csharp/protobuf/person/person_receive.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`person_receive.py </source_code_samples/python/protobuf/person/person_receive.py>`