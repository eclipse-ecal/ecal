.. include:: /include.txt

.. _getting_started_howto_pubsub_capnproto_addressbook:

===========================
Cap'n Proto: AddressBook
===========================

In the Cap'n Proto example, we will send an address book with multiple entries.

AddressBook Cap'n Proto
=======================

As the sender and receiver need the same .capnp files, we place them in a separate directory next to the source directories for the sender and the receiver.

.. parsed-literal::
   |fa-folder-open| AddressBook Cap'n Proto File
   └─ |fa-file-alt| :download:`addressbook.capnp </source_code_samples/cpp/capnproto/addressbook/addressbook_send/src/addressbook.capnp>`

Let's start with the :file:`capnp/addressbook.capnp` file!

.. literalinclude:: /source_code_samples/cpp/capnproto/addressbook/addressbook_send/src/addressbook.capnp
   :language: capnp
   :linenos:
   :lines: 20-

It's the default Cap'n Proto example and we will be using it to send an address book with multiple entries.

AddressBook Publisher
=====================

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/protobuf/person/person_send/src/person_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-


Person Publisher Files
======================

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
     └─ |fa-file-alt| :download:`addressbook_send.cpp <source_code_samples/cpp/capnproto/addressbook/addressbook_send/src/addressbook_send.cpp>`


AddressBook Subscriber
======================


.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/capnproto/addressbook/addressbook_receive/src/addressbook_receive.cpp
            :language: cpp
            :linenos:
            :lines: 20-


Person Subscriber Files
=======================

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`addressbook_receive.cpp </source_code_samples/cpp/capnproto/addressbook/addressbook_receive/src/addressbook_receive.cpp>`


AddressBook Dynamic Subscriber
===============================

Using Capnproto, it is possible to receive data without knowing the structure of the data in advance.
Hence you can use a dynamic subscriber to receive capnproto data, even if you do not have access to the corresponding ``.capnp`` file.
This is useful for generic applications, such as the eCAL Monitor, which can display all data types without knowing them in advance.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/capnproto/addressbook/addressbook_receive_dynamic/src/addressbook_receive_dynamic.cpp
            :language: cpp
            :linenos:
            :lines: 20-


AddressBook Dynamic Subscriber Files
====================================

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`addressbook_receive_dynamic.cpp </source_code_samples/cpp/capnproto/addressbook/addressbook_receive_dynamic/src/addressbook_receive_dynamic.cpp>`