.. include:: /include.txt

.. _getting_started_howto_pubsub_string_hello_world:

===================
String: Hello World
===================

In the good habit of every tutorial, we will write a Hello World application, that sends the string “Hello World” to an eCAL topic.
It will utilize the eCAL string publisher to send the message, which is a really thin layer on top of the binary.
The second part will be all about the subscriber, which will receive the message and print it to the console.

Basically, by using eCAL's message API, we no longer send raw binary data, but utf-8 encoded strings.
Hence, depending on the native string handling in the programming language, the serialization is "just" an utf-8 encoding / decoding of a string.

All other available methods for binary publishers and subscribers are also available for message publishers and subscribers.

Hello World Publisher
=====================

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

        .. literalinclude:: /source_code_samples/python/string/hello/nb_string_hello_send.py
            :language: python
            :linenos:
            :lines: 19-

    .. group-tab:: Python (legacy)

        .. literalinclude:: /source_code_samples/python_legacy/string/hello/hello_send.py
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
   │  └─ |fa-file-alt| :download:`hello_send.cs </source_code_samples/csharp/string/hello/hello_send_csharp.cs>`
   │
   ├─ |fa-folder-open| Python
   │  └─ |fa-file-alt| :download:`hello_send.py </source_code_samples/python/string/hello/nb_string_hello_send.py>`
   │
   └─ |fa-folder-open| Python (legacy)
      └─ |fa-file-alt| :download:`hello_send.py </source_code_samples/python_legacy/string/hello/hello_send.py>`
   

Hello World Subscriber
======================

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

        .. literalinclude:: /source_code_samples/python/string/hello/nb_string_hello_receive.py
            :language: python
            :linenos:
            :lines: 19-

    .. group-tab:: Python (legacy)

        .. literalinclude:: /source_code_samples/python_legacy/string/hello/hello_receive.py
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
   │  └─ |fa-file-alt| :download:`hello_receive.cs </source_code_samples/csharp/string/hello/hello_receive_csharp.cs>`
   │
   ├─ |fa-folder-open| Python
   │  └─ |fa-file-alt| :download:`hello_receive.py </source_code_samples/python/string/hello/nb_string_hello_receive.py>`
   │
   └─ |fa-folder-open| Python (legacy)
      └─ |fa-file-alt| :download:`hello_receive.py </source_code_samples/python_legacy/string/hello/hello_receive.py>`