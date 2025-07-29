.. include:: /include.txt

.. _getting_started_howto_pubsub_binary_blob:

============
Binary: Blob
============

After you have learned a lot about the pre-compiled applications that come with eCAL, let’s create our own!
We will see how to create publisher and subscriber applications that send and receive binary data.
We will provide samples for all supported programming languages: C++, C, C# and Python.

The most low level publisher and subscriber objects are the binary ones.
We will focus on them in this section.
As we are just sending an arbitrary "blob" of data (filled with random printable characters), the applications are called "blob sender" and "blob receiver".
For sending structured / typed data, please refer to the following sections.


Blob Publisher
==============

Let's begin with the publisher side of our "Blob" application.

The base initialization of the eCAL publisher is the same in all languages:

1. Before you do anything else, you need to initialize eCAL with ``Initialize(..)``.
2. Then you create the publisher and send a message in the frequency you want.
   In our example we will send the message every 500 ms in an infinite loop.
   You can add a stop condition to the loop, if you want to send just a limited amount of messages.
3. After you are done with publishing data and you don't need eCAL anymore, you can call the ``Finalize()`` 
   function to clean up the resources and unregister the process.

For simplicity, we will use the same message type in all languages.

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

        .. literalinclude:: /source_code_samples/python/binary/blob/nb_binary_blob_send.py
            :language: python
            :linenos:
            :lines: 19-


    .. group-tab:: Python (legacy)

        .. literalinclude:: /source_code_samples/python_legacy/binary/blob/blob_send.py
            :language: python
            :linenos:
            :lines: 19-

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
   ├─ |fa-folder-open| Python
   │  └─ |fa-file-alt| :download:`blob_send.py </source_code_samples/python/binary/blob/nb_binary_blob_send.py>`
   │
   └─ |fa-folder-open| Python (legacy)
      └─ |fa-file-alt| :download:`blob_send.py </source_code_samples/python_legacy/binary/blob/blob_send.py>`
   

Blob Subscriber
===============

Now let's have a look at the subscriber side. 
Basically, the initialization is the same as for the publisher.
Instead of sending data, a callback function is assigned to the subscriber, which will be called every time a new message arrives.

1. Call ``Initialize()`` to initialize eCAL.
2. Create the subscriber.
3. Assign a callback function to the subscriber with ``SetReceiveCallback``.
4. Do something to keep the process alive. In our example we will use a simple infinite loop. Process the incoming messages as you wish.
5. After you are done with receiving data and you don't need eCAL anymore, you can call the ``Finalize()`` 
   function to clean up the resources and unregister the process.

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

        .. literalinclude:: /source_code_samples/python/binary/blob/nb_binary_blob_receive.py
            :language: python
            :linenos:
            :lines: 19-
            
    .. group-tab:: Python (legacy)

        .. literalinclude:: /source_code_samples/python_legacy/binary/blob/blob_receive.py
            :language: python
            :linenos:
            :lines: 19-            


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
   ├─ |fa-folder-open| Python
   │  └─ |fa-file-alt| :download:`blob_receive.py </source_code_samples/python/binary/blob/nb_binary_blob_receive.py>`
   │
   └─ |fa-folder-open| Python (legacy)
      └─ |fa-file-alt| :download:`blob_receive.py </source_code_samples/python_legacy/binary/blob/blob_receive.py>`