.. include:: /include.txt

.. _getting_started_howto_pubsub_binary_blob:

============
Binary: Blob
============

Last but not least we want to show how you can send binary data with eCAL.
When you read through the previous chapters, you will see that you are already familiar with most of the code.

The biggest difference is that you basically can send any kind of data. 
But be aware that you also need to handle the binary interpretation across multiple platforms, which is handled before simply through a serialization protocol.

Blob Publisher
==============

The main differences to serialization publishers are:
  - you don't need to include any serialization header file in addition to the convenience header
  - you send a binary buffer with a buffer size

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/binary/blob/blob_send/src/blob_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C

        .. literalinclude:: /source_code_samples/c/binary/blob/blob_send/src/blob_send.c
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/binary/blob/blob_send_csharp.cs
            :language: csharp
            :linenos:
            :lines: 31-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/binary/blob/blob_send.py
            :language: python
            :linenos:
            :lines: 19-


Blob Publisher Files
====================

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`blob_send.cpp </source_code_samples/cpp/binary/blob/blob_send/src/blob_send.cpp>`
   │
   ├─ |fa-folder-open| C
   │  └─ |fa-file-alt| :download:`blob_send.c </source_code_samples/c/binary/blob/blob_send/src/blob_send.c>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`blob_send.cs </source_code_samples/csharp/binary/blob/blob_send_csharp.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`blob_send.py </source_code_samples/python/binary/blob/blob_send.py>`
   

Blob Subscriber
===============

For the subscriber the same changes apply as for the publisher. In addition you need to take care of the blob handling yourself.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/binary/blob/blob_receive/src/blob_receive.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C

        .. literalinclude:: /source_code_samples/c/binary/blob/blob_receive/src/blob_receive.c
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/binary/blob/blob_receive_csharp.cs
            :language: csharp
            :linenos:
            :lines: 31-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/binary/blob/blob_receive.py
            :language: python
            :linenos:
            :lines: 19-


Blob Subscriber Files
=====================

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`blob_receive.cpp </source_code_samples/cpp/binary/blob/blob_receive/src/blob_receive.cpp>`
   │
   ├─ |fa-folder-open| C
   │  └─ |fa-file-alt| :download:`blob_receive.c </source_code_samples/c/binary/blob/blob_receive/src/blob_receive.c>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`blob_receive.cs </source_code_samples/csharp/binary/blob/blob_receive_csharp.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`blob_receive.py </source_code_samples/python/binary/blob/blob_receive.py>`