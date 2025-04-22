.. include:: /include.txt

.. _getting_started_howto_pubsub_string_hello_world:

===================
String: Hello World
===================

After you have learned a lot about the pre-compiled applications that come with eCAL, let’s create our own!
In the good habit of every tutorial, we will write a Hello World Application, that sends the string “Hello World” to an eCAL topic.
The second part will be all about the subscriber, which will receive the message and print it to the console.

You will see how it is done in the major programming languages we support: C++, C, C# and Python.

For CMake usage tips, please refer to the CMake section of this documentation.


Hello World Publisher
=====================

Let's begin with the publisher side of our "Hello World" application.

The base initialization of the eCAL publisher is the same in all languages:

1. Before you do anything else, you need to initialize eCAL with ``Initialize(..)``.
2. Then you create the publisher and send a message in the frequency you want.
   In our example we will send the message every 500 ms in an infinite loop.
   You can add a stop condition to the loop, if you want to send just a limited amount of messages.
3. After you are done with publishing data and you don't need eCAL anymore, you can call the ``Finalize()`` 
   function to clean up the resources and unregister the process.

For simplicity, we will use the same message type in all languages.
As this is simple string message, we will use the eCAL string publisher to send.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/string/hello/hello_send/src/hello_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C
        
        .. literalinclude:: /source_code_samples/c/string/hello/hello_send/src/hello_send.c
            :language: c
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/string/hello/hello_send_csharp.cs
            :language: csharp
            :linenos:
            :lines: 31-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/string/hello/hello_send.py
            :language: python
            :linenos:
            :lines: 19-


Hello World Publisher Files
===========================

.. parsed-literal::

   |fa-folder-open| Publisher samples
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`hello_send.cpp </source_code_samples/cpp/string/hello/hello_send/src/hello_send.cpp>`
   │
   ├─ |fa-folder-open| C
   │  └─ |fa-file-alt| :download:`hello_send.c </source_code_samples/c/string/hello/hello_send/src/hello_send.c>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`hello_send_csharp.cs </source_code_samples/csharp/string/hello/hello_send_csharp.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`hello_send.py </source_code_samples/python/string/hello/hello_send.py>`
   

Hello World Subscriber
======================

Now let's have a look at the subscriber side. Basically, the initialization is the same as for the publisher.
The only difference is that you need to assign a callback function to the subscriber:

1. Call ``Initialize()`` to initialize eCAL.
2. Create the subscriber.
3. Assign a callback function to the subscriber with ``SetReceiveCallback``.
4. Do something to keep the process alive. In our example we will use a simple infinite loop. Process the incoming messages as you wish.
5. After you are done with receiving data and you don't need eCAL anymore, you can call the ``Finalize()`` 
   function to clean up the resources and unregister the process.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/string/hello/hello_receive/src/hello_receive.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C
        
        .. literalinclude:: /source_code_samples/c/string/hello/hello_receive/src/hello_receive.c
            :language: c
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/string/hello/hello_receive_csharp.cs
            :language: csharp
            :linenos:
            :lines: 31-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/string/hello/hello_receive.py
            :language: python
            :linenos:
            :lines: 19-


Hello World Subscriber Files
============================

.. parsed-literal::

   |fa-folder-open| Subscriber samples
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`hello_receive.cpp </source_code_samples/cpp/string/hello/hello_receive/src/hello_receive.cpp>`
   │
   ├─ |fa-folder-open| C
   │  └─ |fa-file-alt| :download:`hello_receive.c </source_code_samples/c/string/hello/hello_receive/src/hello_receive.c>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`hello_receive_csharp.cs </source_code_samples/csharp/string/hello/hello_receive_csharp.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`hello_receive.py </source_code_samples/python/string/hello/hello_receive.py>`