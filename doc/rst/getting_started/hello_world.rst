.. include:: /include.txt

.. _getting_started_hello_world:

===========
Hello World
===========

After you have learned a lot about the pre-compiled applications that come with eCAL, let's create our own!
In the good habit of every tutorial, we will write a Hello World Applications, that sends the string "Hello World" to an eCAL topic.

eCAL uses CMake as a cross-plattform build toolchain.
We will explain the CMake commands needed for the tutorial, but not extensively dive into CMake.

.. important::
   .. image:: /img/snail.svg
      :alt: Snail
      :width: 100px
      :class: float-right

   |fa-windows| The Windows version of this tutorial is not finished, yet and will not work.

   Please be patient and use Ubuntu, for now.

   .. rst-class::  clear-both

Dependencies
============

First of all, you have to install some more development dependencies:

* |fa-windows| Windows:

  * Visual Studio (https://visualstudio.microsoft.com/)
  * CMake (https://cmake.org/download/)

    .. tip::
       
       When installing CMake, choose "Add to PATH", so you don't have to always provide the full path to the cmake executable

* |fa-ubuntu| Ubuntu:

  .. code-block:: bash

     sudo apt install cmake g++ libprotobuf-dev protobuf-compiler

Hello World Publisher
=====================

Somewhere on your hard drive create an empty directory and create a file :file:`CMakeLists.txt` and :file:`main.cpp` with the following content:

* |fa-file-alt| :file:`CMakeLists.txt`:

  .. literalinclude:: src/hello_world_snd/CMakeLists.txt
     :language: cmake
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 3** creates a project "hello_world_snd".
     This will also be the name of the executable (**line 11**).

     **Line 5** tells CMake to find the eCAL installation. **Line 13-15** will link the executable against it.

     **Line 7-9** create a list of all our source files, wich currently only contains :file:`main.cpp`.
     We add that source file for compiling our executable in **line 11**.

* |fa-file-alt| :file:`main.cpp`:

  .. literalinclude:: src/hello_world_snd/main.cpp
     :language: cpp
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 1** includes the basic eCAL header.
     As we want to publish raw strings, line 2 includes the eCAL String-Publisher.
     eCAL Supports multiple message formats.

     **Line 10** initialized eCAL.
     You always have to initialize eCAL before using it's API.
     The name of our eCAL Process will be "Hello World Publisher". 
     This name will be visible in the eCAL Monitor, once the Process is running.
     
     **Line 13** creates an eCAL Publisher.
     An eCAL Process can create multiple publishers (and multiple subscribers).
     The topic we are publishing on will be "hello_world_topic".

     The while loop from **line 20** will cause an infinite publish-loop.
     eCAL supports a stop-signal; when an eCAL Process is stopped, ``eCAL::Ok()`` will return false.

     **Line 25** will publish our message and send it to other eCAL Processes that have subsribed on the topic.

     **Line 32** de-initializes eCAL. You should always do that before your application exits.

Now that you have the source code ready, create a :file:`_build` directory and build the code!

* |fa-windows| Windows:

  .. code-block:: batch

     mkdir _build
     cd _build
     cmake .. -A x64

  .. todo:: Continue Windows
   
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

Hello World Subscriber
======================

This section is still under construction. But we are working on it! Just be patient.

.. image:: /img/snail.svg
   :alt: Snail
   :align: center


..
   Publisher
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~

   Subscriber
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~