.. include:: /include.txt

.. _getting_started_howto_clientserver_protobuf_math:

==============
Protobuf: Math
==============

In C++ eCAL also supports the protobuf serialization format for the client/server API.

That means that for the request and response, an interface can be defined as a protobuf message in order to make it easier to handle requests.


Protobuf file
=============

We use the special protobuf service definitions in order to implement it in our server/client applications.
The "message" format is already known by you from the publisher/subscriber examples.
The "service" format and the ``rpc xyz (type) returns (type)`` is now added. You will see, which effect this addition has on our example.

.. literalinclude:: /source_code_samples/cpp/protobuf/math/math_client/src/protobuf/math.proto
   :language: protobuf
   :linenos:
   :lines: 20-

.. parsed-literal::
   
   |fa-folder-open| Math Protobuf File
   └─ |fa-file-alt| :download:`math.proto </source_code_samples/cpp/protobuf/math/math_client/src/protobuf/math.proto>`


Math Server
===========

In the server we need to implement the service interface defined by the protobuf file.
For that we derive from the generated class ``MathService`` and implement the methods.

For the data exchange the server reads the input messages from the protobuf message and writes directly to the output message.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/protobuf/math/math_server/src/math_server.cpp
            :language: cpp
            :linenos:
            :lines: 20-

.. parsed-literal::

   |fa-folder-open| C++
      └─ |fa-file-alt| :download:`math_server.cpp </source_code_samples/cpp/protobuf/math/math_server/src/math_server.cpp>`


Math Client
===========

The client handling is similiar to the binary example. The main difference is that our request is now a protobuf message which will be the input for the server.
In the response we can read out the results and print them to the console.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/protobuf/math/math_client/src/math_client.cpp
            :language: cpp
            :linenos:
            :lines: 20-

.. parsed-literal::

   |fa-folder-open| C++
      └─ |fa-file-alt| :download:`math_client.cpp </source_code_samples/cpp/protobuf/math/math_client/src/math_client.cpp>`