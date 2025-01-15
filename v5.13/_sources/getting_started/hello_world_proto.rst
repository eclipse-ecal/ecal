.. include:: /include.txt

.. _getting_started_hello_world_proto:

=========================
Hello World with protobuf
=========================

In the last section you learned how to send strings to an eCAL Topic.
Using strings is great for simple data that has a textual representation.
Quite often however your data will be more complex, so you need some kind of protocol that defines how your data is structured.

Of course, you can define your own protocol and pass the raw memory to eCAL (you would use the raw `eCAL::CPublisher()` to do that).
Our recommended way however is to use Google protobuf, because:

* It solves the problem of how to serialize and de-serialize data for you
* You get downward compatibility out of the box (if you follow the guidelines)
* It is maintained by Google and the API is stable
* The eCAL Monitor can display a nice reflection view of the data

.. important::
   It is important to remember, that all your applications must agree on the data format.
   As protobuf messages are defined in :file:`.proto` files, all of your applications should be compiled with the same files.

Protobuf sender
===============

Let's implement a small application, that lets the user input his name and send a message to an eCAL topic.
As the sender and receiver need the same .proto files, we place them in a separate directory next to the source directories for the sender and the receiver:

.. parsed-literal::
   
   |fa-folder-open|
   ├─ |fa-folder-open| :file:`proto_messages`
   │  └─ |fa-file-alt| :file:`hello_world.proto`
   ├─ |fa-folder| :file:`protobuf_snd`
   └─ |fa-folder| :file:`protobuf_rec`

Let's start with the :file:`proto_messages/hello_world.proto` file!

.. literalinclude:: src/hello_world_protobuf/proto_messages/hello_world.proto
   :language: protobuf
   :linenos:

.. note::
   **What is happening here?**

   **Line 3** assigns a package name (this will appear as C++ namespace later).

   **Line 5-10** Creates a message "HelloWorld", that holds the fields "name", "id" and "msg".

Now start implementing the actual sender application. Just as in the :ref:`last section <getting_started_hello_world>` create the :file:`CMakeLists.txt` and :file:`main.cpp` in the :file:`protobuf_snd` directory and paste the following content:

* |fa-file-alt| :file:`CMakeLists.txt`:

  .. literalinclude:: src/hello_world_protobuf/protobuf_snd/CMakeLists.txt
     :language: cmake
     :linenos:

  .. note::
     **What is happening here?**

     **Line 10** adds Protobuf as dependency
     
     **Line 16-18** Creates a list of .proto files. We only have one.

     **Line 22** Compiles the .proto file to a C++ header file (:file:`hello_world.pb.h`).
     The ``PROTOBUF_TARGET_CPP`` function is a convenience function from eCAL.
     If you have already worked with Protobuf and CMake, you may be more familiar with the following code, which basically does the same thing:

     .. code-block:: cmake
        
       include_directories(${CMAKE_CURRENT_BINARY_DIR})
       protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protobuf_files})
       add_executable(${PROJECT_NAME} ${source_files} ${PROTO_SRCS} ${PROTO_HDRS}) 

     **Line 26** links the executable against protobuf

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: src/hello_world_protobuf/protobuf_snd/main.cpp
     :language: cpp
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 2**: This time, we include the protobuf publisher.

     **Line7** includes the generated C++ file from the :file:`hello_world.proto`

     **Line 13** creates a protobuf publisher instance.
     Note that it is templated to ``proto_messages::HelloWorld``, so if you would want to send different messages to different topics, you would create one publisher instance per topic.
     Sending different messages to the same topic is a bad idea and will break the de-serialization.

     **Line 32-36** creates the message as protobuf object and sets the fields in it.

     **Line 38** sends the protobuf object out to the topic.

Again, you can already watch the sender doing its work by inspecting the topic in the eCAL Monitor!
(Compile the application just as in the :ref:`last section <getting_started_hello_world_build_instructions>`)

.. image:: img/hello_world_protobuf_snd_monitor.png
   :alt: Hello World Protobuf sender and eCAL Monitor

.. important::
   The eCAL Monitor will not start listening to messages, until you open the reflection window.
   So, you will only see messages that were sent *after* you have opened the window.

Protobuf receiver
=================

* |fa-file-alt| :file:`CMakeLists.txt`:

  .. literalinclude:: src/hello_world_protobuf/protobuf_rec/CMakeLists.txt
     :language: cmake
     :linenos:

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: src/hello_world_protobuf/protobuf_rec/main.cpp
     :language: cpp
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 9** is our subscriber callback (you have already seen a callback in the last Hello World Tutorial).
     This time however it receives a protobuf object.
     **Line 11-13** use the handy protobuf accessor methods to print the data to the terminal.

     **Line 20** Creates an eCAL protobuf subscriber.
     Just like the publisher, it is templated to the ``proto_messages::HelloWorld`` message.

     **Line 23** Sets the callback, so eCAL can call it whenever a new message is received.

Now compile and start both the sender and the receiver application and send some messages!

.. image:: img/hello_world_protobuf_snd_rec.png
   :alt: Hello World Protobuf sender and receiver

Congratulations, you have completed the Getting Started chapter!
Now go ahead and use eCAL in your real-world scenario.

If you experience issues, you can create a `GitHub issue <https://github.com/eclipse-ecal/ecal/issues>`_, to get help.

Files
=====

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| proto_messages
   │  └─ |fa-file-alt| :download:`hello_world.proto <src/hello_world_protobuf/proto_messages/hello_world.proto>`
   │
   ├─ |fa-folder-open| protobuf_snd
   │  ├─ |fa-file-alt| :download:`CMakeLists.txt <src/hello_world_protobuf/protobuf_snd/CMakeLists.txt>`
   │  └─ |fa-file-alt| :download:`main.cpp <src/hello_world_protobuf/protobuf_snd/main.cpp>`
   │
   └─ |fa-folder-open| protobuf_rec
      ├─ |fa-file-alt| :download:`CMakeLists.txt <src/hello_world_protobuf/protobuf_rec/CMakeLists.txt>`
      └─ |fa-file-alt| :download:`main.cpp <src/hello_world_protobuf/protobuf_rec/main.cpp>`
