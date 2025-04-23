.. include:: /include.txt

.. _getting_started_howto_clientserver_binary_mirror:

==============
Binary: Mirror
==============

Under this section you will find a simple example of a client/server application using the eCAL ClientServer API.
You should be already familiar with the base handling of eCAL from the Publisher/Subscriber samples, so this we will not cover in detail anymore.

In this section we will introduce the client/server API and show you how to use it.

Mirror Server
=============

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
            :lines: 31-

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

For the subscriber the same changes apply as for the publisher. In addition you need to take care of the blob handling yourself.

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
            :lines: 31-

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