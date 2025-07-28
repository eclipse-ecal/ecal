.. include:: /include.txt

.. _getting_started_howto_pubsub_flatbuffers_monster:

=====================
FlatBuffers: Monster
=====================

`FlatBuffers <https://flatbuffers.dev/>`__ "is an efficient cross platform serialization library for C++, C#, C, Go, Java, Kotlin, JavaScript, Lobster, Lua, TypeScript, PHP, Python, Rust and Swift. It was originally created at Google for game development and other performance-critical applications."
eCAL has implemented flatbuffer serialization on top of the binary publisher and subscriber API, such that users can conveniently send flatbuffer objects.

Monster FlatBuffers
=====================

As the sender and receiver need the same .proto files, we place them in a separate directory next to the source directories for the sender and the receiver.

.. parsed-literal::

   |fa-folder-open| Monster FlatBuffers File
   └─ |fa-file-alt| :download:`monster.fbs </source_code_samples/cpp/flatbuffers/monster/monster_send/monster/monster.fbs>`



Let's start with the :file:`monster/monster.fbs` file!

.. literalinclude:: /source_code_samples/cpp/flatbuffers/monster/monster_send/monster/monster.fbs
   :language: protobuf
   :linenos:
   :lines: 20-


Monster Publisher
==================

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/flatbuffers/monster/monster_send/monster_send.cpp
            :language: cpp
            :linenos:
            :lines: 20-


Monster Publisher Files
========================

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`monster_send.cpp </source_code_samples/cpp/flatbuffers/monster/monster_send/monster_send.cpp>`

Monster Subscriber
======================

.. tabs::

    .. group-tab:: C++

        .. literalinclude:: /source_code_samples/cpp/flatbuffers/monster/monster_receive/monster_receive.cpp
            :language: cpp
            :linenos:
            :lines: 20-

Monster Subscriber Files
==========================

.. parsed-literal::

   |fa-folder-open|
   └─ |fa-folder-open| C++
      └─ |fa-file-alt| :download:`monster_receive.cpp </source_code_samples/cpp/flatbuffers/monster/monster_receive/monster_receive.cpp>`
