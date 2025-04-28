.. include:: /include.txt

.. _getting_started_howto_clientserver_binary_mirror:

==============
Binary: Mirror
==============

Under this section you will find a simple example of a client/server application using the eCAL ClientServer API.
You should be already familiar with the base handling of eCAL from the Publisher/Subscriber samples, so this we will not be covered in detail any more.

Mirror Server
=============

A service server is an eCAL instance that is matched to service client instances by their name.
It can offer multiple methods, which a connected service client can call.
We will set up a simple server that provides two methods: "echo" and "mirror".

The main process is:

- Intialize eCAL
- Create a Service Server
- For each method, add meta information about that method (such as method name, and types of the request and response) and a callback function to be invoked, when a client calls that method
- Keep the program running while the service is supposed to be executed

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/binary/mirror/mirror_server/src/mirror_server.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C

        .. literalinclude:: /source_code_samples/c/binary/mirror/mirror_server_c/src/mirror_server_c.c
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/binary/mirror/mirror_server_csharp.cs
            :language: csharp
            :linenos:
            :lines: 20-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/binary/mirror/minimal_service_server.py
            :language: python
            :linenos:
            :lines: 19-


Mirror Server Files
===================

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`mirror_server.cpp </source_code_samples/cpp/binary/mirror/mirror_server/src/mirror_server.cpp>`
   │
   ├─ |fa-folder-open| C
   │  └─ |fa-file-alt| :download:`mirror_server_c.c </source_code_samples/c/binary/mirror/mirror_server_c/src/mirror_server_c.c>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`mirror_server_csharp.cs </source_code_samples/csharp/binary/mirror/mirror_server_csharp.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`minimal_service_server.py </source_code_samples/python/binary/mirror/minimal_service_server.py>`

Mirror Client
=============

The client will have some more logic to take care of, as its possible that multiple servers are running with the same service name.

- Initialize eCAL
- Create a service client with the service name and (optionally) register the functions it can call (in this case "echo" and "mirror")
- Waiting for a server to be available (this is optional and depends on how you want to design your application)
- Retrieve all client instances.
  You can also call it directly on the service client, but if you iterate through all client instances, you are more flexible and could filter out instances you don't want to call.
- Then we call the methodnames with two different calls: with callback (non blocking) and with response (blocking)
- Handle the received data (or error status)

As a little extra we also added a little bit more eCAL state handling as in the previous examples.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/binary/mirror/mirror_client/src/mirror_client.cpp
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C

        .. literalinclude:: /source_code_samples/c/binary/mirror/mirror_client_c/src/mirror_client_c.c
            :language: cpp
            :linenos:
            :lines: 20-

    .. group-tab:: C#

        .. literalinclude:: /source_code_samples/csharp/binary/mirror/mirror_client_csharp.cs
            :language: csharp
            :linenos:
            :lines: 20-

    .. group-tab:: Python

        .. literalinclude:: /source_code_samples/python/binary/mirror/minimal_service_client.py
            :language: python
            :linenos:
            :lines: 19-


Mirror Client Files
===================

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| C++
   │  └─ |fa-file-alt| :download:`mirror_client.cpp </source_code_samples/cpp/binary/mirror/mirror_client/src/mirror_client.cpp>`
   │
   ├─ |fa-folder-open| C
   │  └─ |fa-file-alt| :download:`mirror_client.c </source_code_samples/c/binary/mirror/mirror_client_c/src/mirror_client_c.c>`
   │
   ├─ |fa-folder-open| C#
   │  └─ |fa-file-alt| :download:`mirror_client_csharp.cs </source_code_samples/csharp/binary/mirror/mirror_client_csharp.cs>`
   │
   └─ |fa-folder-open| Python
      └─ |fa-file-alt| :download:`minimal_service_client.py </source_code_samples/python/binary/mirror/minimal_service_client.py>`