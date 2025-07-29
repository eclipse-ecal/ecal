.. include:: /include.txt

.. _getting_started_howto_pubsub_capnproto_addressbook:

===========================
Cap'n Proto: AddressBook
===========================

"`Cap’n Proto <https://capnproto.org/>`__ is an insanely fast data interchange format and capability-based RPC system."
eCAL has implemented Cap'n Proto serialization on top of the binary publisher and subscriber API, such that users can conveniently send Cap’n Proto objects.
Currently, the eCAL Capnproto Message API is only available for C++.

In the Cap'n Proto example, we will use their default serialization example and send an address book with multiple entries.


AddressBook Cap'n Proto
=======================

Cap'n Proto uses its own schema definition language to define the data structures, which is not unlike the Protobuf schema definition language.

As the sender and receiver need the same .capnp files, we place them in a separate directory next to the source directories for the sender and the receiver.

Let's start with the :file:`capnp/addressbook.capnp` file!

.. literalinclude:: /source_code_samples/cpp/capnproto/addressbook/addressbook_send/src/addressbook.capnp
   :language: capnp
   :linenos:
   :lines: 21-

.. parsed-literal::
   |fa-folder-open| AddressBook Cap'n Proto File
   └─ |fa-file-alt| :download:`addressbook.capnp </source_code_samples/cpp/capnproto/addressbook/addressbook_send/src/addressbook.capnp>`

AddressBook Publisher
=====================

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/capnproto/addressbook/addressbook_send/src/addressbook_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-


.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
     └─ |fa-file-alt| :download:`addressbook_send.cpp </source_code_samples/cpp/capnproto/addressbook/addressbook_send/src/addressbook_send.cpp>`


AddressBook Subscriber
=======================


.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/capnproto/addressbook/addressbook_receive/src/addressbook_receive.cpp
            :language: cpp
            :linenos:
            :lines: 29-


.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`addressbook_receive.cpp </source_code_samples/cpp/capnproto/addressbook/addressbook_receive/src/addressbook_receive.cpp>`


AddressBook Dynamic Subscriber
===============================

Using Capnproto, it is possible to receive data without knowing the structure of the data in advance.
Hence you can use a dynamic subscriber to receive capnproto data, even if you do not have access to the corresponding :file:`.capnp` file.
This is useful for generic applications, such as the eCAL Monitor, which can display all data types without knowing them in advance.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/capnproto/addressbook/addressbook_receive_dynamic/src/addressbook_receive_dynamic.cpp
            :language: cpp
            :linenos:
            :lines: 20-

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`addressbook_receive_dynamic.cpp </source_code_samples/cpp/capnproto/addressbook/addressbook_receive_dynamic/src/addressbook_receive_dynamic.cpp>`