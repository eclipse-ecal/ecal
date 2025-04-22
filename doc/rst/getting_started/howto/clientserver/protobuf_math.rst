.. include:: /include.txt

.. _getting_started_howto_clientserver_protobuf_math:

==============
Protobuf: Math
==============

Here will rise some client/server examples using binary data.

Math Server
===========

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/binary/math/math_server/src/math_server.cpp
            :language: cpp
            :linenos:
            :lines: 20-


Math Files
==========

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`math_server.cpp </source_code_samples/cpp/binary/math/math_server/src/math_server.cpp>`


Math Client
===========

For the subscriber the same changes apply as for the publisher. In addition you need to take care of the blob handling yourself.

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/protobuf/math/math_client/src/math_client.cpp
            :language: cpp
            :linenos:
            :lines: 20-


Math Client Files
===================

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`math_client.cpp </source_code_samples/cpp/protobuf/math/math_client/src/math_client.cpp>`