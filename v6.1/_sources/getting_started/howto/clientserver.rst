.. include:: /include.txt

.. _getting_started_howto_clientserver:

=========================
Service: Client & Server
=========================

While publish and subscribe is a one way communication pattern, eCAL also supports bidirectional communication, using the client and server pattern.
This chapter provides sample code for all supported language APIs.

Just like with PubSub, the basic communication is based on exchanging binary data.
This basic form of using services is explained in the section :doc:`clientserver/binary_mirror`.

On top of those binary services, eCAL has implemented (de-)serialization for popular serialization formats for

* Google Protobuf :doc:`clientserver/protobuf_math`.

.. toctree::
   :hidden:

   clientserver/binary_mirror
   clientserver/protobuf_math