.. include:: /include.txt

.. _getting_started_howto_pubsub_protobuf_person:

================
Protobuf: Person
================

Let's implement a small application, that lets the user input his name and send a message to an eCAL topic.


Person Protobuf
===============

As the sender and receiver need the same .proto files, we place them in a separate directory next to the source directories for the sender and the receiver.

.. parsed-literal::
   
   |fa-folder-open| Person Protobuf File
   └─ |fa-file-alt| :download:`person.proto </source_code_samples/cpp/protobuf/person/person_send/src/protobuf/person.proto>`


Let's start with the :file:`protobuf/person.proto` file!

.. literalinclude:: /source_code_samples/cpp/protobuf/person/person_send/src/protobuf/person.proto
   :language: protobuf
   :linenos:
   :lines: 20-


Person Publisher
================

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