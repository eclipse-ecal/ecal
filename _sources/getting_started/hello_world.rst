.. include:: /include.txt

.. _getting_started_hello_world:

===========
Hello World
===========

.. important::

   |fa-windows| This tutorial will **only work with eCAL 5.7.3** and upwards.
   It will **not** work with older versions that were published as .msi installer (it missed some libraries).

   Please switch to Ubuntu, if you are using an old eCAL Version.

After you have learned a lot about the pre-compiled applications that come with eCAL, let's create our own!
In the good habit of every tutorial, we will write a Hello World Application, that sends the string "Hello World" to an eCAL topic.

eCAL uses CMake as a cross-platform build toolchain.
We will explain the CMake commands needed for the tutorial, but not extensively dive into CMake.

Dependencies
============

First, you have to install some more development dependencies:

* |fa-windows| Windows:

  * Visual Studio (https://visualstudio.microsoft.com/)
  * CMake (https://cmake.org/download/)

    .. tip::
       
       When installing CMake, choose "Add to PATH", so you don't have to always provide the full path to the CMake executable

       .. image:: img/cmake_setup_cut.svg
          :alt: CMake Add to PATH
          :class: with-shadow


* |fa-ubuntu| Ubuntu:

  .. code-block:: bash

     sudo apt install cmake g++ libprotobuf-dev protobuf-compiler

.. _getting_started_hello_world_publisher:

Hello World Publisher
=====================

Somewhere on your hard drive create an empty directory and create a file :file:`CMakeLists.txt` and :file:`main.cpp` with the following content:

* |fa-file-alt| :file:`CMakeLists.txt`:

  .. literalinclude:: src/hello_world/hello_world_snd/CMakeLists.txt
     :language: cmake
     :linenos:

  .. note::
     **What is happening here?**

     **Line 2** makes CMake prefer installed config files instead of generic find scripts.
     This is important for Windows, where eCAL installs Protobuf, HDF5 etc.

     **Line 4** creates a project "hello_world_snd".
     This will also be the name of the executable (**line 15**).

     **Line 6-7** set the C++ standard to C++14

     **Line 9** tells CMake to find the eCAL installation. **Line 17-19** will link the executable against it.

     **Line 11-13** create a list of all our source files, which currently only contains :file:`main.cpp`.
     We add that source file for compiling our executable in **line 15**.

     **Line 17-19** link our executable against the eCAL core library.

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: src/hello_world/hello_world_snd/main.cpp
     :language: cpp
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 1** includes the basic eCAL header.
     As we want to publish raw strings, line 2 includes the eCAL String-Publisher.
     eCAL Supports multiple message formats.
     
     **Line 10** initialized eCAL.
     You always have to initialize eCAL before using its API.
     The name of our eCAL Process will be "Hello World Publisher". 
     This name will be visible in the eCAL Monitor, once the Process is running.

     **Line 13** creates an eCAL Publisher.
     An eCAL Process can create multiple publishers (and multiple subscribers).
     The topic we are publishing on will be "hello_world_topic".

     The while loop from **line 20** will cause an infinite publish-loop.
     eCAL supports a stop-signal; when an eCAL Process is stopped, ``eCAL::Ok()`` will return false.
     
     **Line 25** will publish our message and send it to other eCAL Processes that have subscribed on the topic.

     **Line 32** de-initializes eCAL. You should always do that before your application exits.

.. _getting_started_hello_world_build_instructions:

Now that you have the source code ready, create a :file:`_build` directory and build the code!

* |fa-windows| Windows:

  .. code-block:: batch

     mkdir _build
     cd _build
     cmake .. -A x64
     cmake --build . --parallel
   
* |fa-ubuntu| Ubuntu:

  .. code-block:: bash

     mkdir _build
     cd _build
     cmake ..
     make

Now execute the :file:`hello_world_snd (.exe)` and take a look at the eCAL Monitor!
You will see the "Hello World Publisher" process and the "hello_world_topic".

.. image:: img/hello_world_snd_monitor.png
   :alt: eCAL Monitor Hello World

.. _getting_started_hello_world_subscriber:

Hello World Subscriber
======================

Again, create a new directory somewhere and add create the :file:`CMakeLists.txt` and :file:`main.cpp` with the following content:

* |fa-file-alt| :file:`CMakeLists.txt`:

  .. literalinclude:: src/hello_world/hello_world_rec/CMakeLists.txt
     :language: cmake
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 4** creates a project "hello_world_rec".
     This is the only difference to the hello_world_snd Project.

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: src/hello_world/hello_world_rec/main.cpp
     :language: cpp
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 8-11** Is the receive callback.
     This method will be called whenever a new message arrives.
     
     **Line 19** creates an eCAL subscriber that listens to the "hello_world_topic".

     **Line 22** adds the receive callback from above to the subscriber, so it can be called.

  .. important::
     eCAL Receive callbacks run in the subscriber's receive thread.
     While the callback is running, the subscriber cannot receive new data.
     So, if your callback needs really long to compute, you may have to decouple your computations to not lose messages.

Now that you have the source code ready, create a :file:`_build` directory and build the code!

* |fa-windows| Windows:

  .. code-block:: batch

     mkdir _build
     cd _build
     cmake .. -A x64
     cmake --build . --parallel
   
* |fa-ubuntu| Ubuntu:

  .. code-block:: bash

     mkdir _build
     cd _build
     cmake ..
     make

When you now execute :file:`hello_world_snd` and :file:`hello_world_rec`, the receiver application will receive the messages sent by the sender.

.. image:: img/hello_world_snd_rec.png
   :alt: eCAL Hello World sender and receiver

In the next chapter you will learn how to properly structure your messages with protobuf!

Files
=====

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-folder-open| hello_world_snd
   │  ├─ |fa-file-alt| :download:`CMakeLists.txt <src/hello_world/hello_world_snd/CMakeLists.txt>`
   │  └─ |fa-file-alt| :download:`main.cpp <src/hello_world/hello_world_snd/main.cpp>`
   │
   └─ |fa-folder-open| hello_world_rec
      ├─ |fa-file-alt| :download:`CMakeLists.txt <src/hello_world/hello_world_rec/CMakeLists.txt>`
      └─ |fa-file-alt| :download:`main.cpp <src/hello_world/hello_world_rec/main.cpp>`
