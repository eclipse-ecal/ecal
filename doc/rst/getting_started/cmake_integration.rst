.. include:: /include.txt

.. _getting_started_cmake_integration:

=================
CMake Integration
=================

In this chapter we will show how to setup a simply CMake project using eCAL.
We will show the two most common use cases to generate eCAL processes.
With this knowledge you will be able to create your own CMake project and transfer the knowledge to other use cases.

Basic string entity integration
===============================

For a basic integration, you will need to call ``find_package(eCAL REQUIRED)`` in addition to your normal project.
Furthermore, you link the ``eCAL::string_core`` in order to be able to create string publishers and subscribers.

The CMakeLists.txt looks as follows:

.. literalinclude:: files/CMakeLists/string/CMakeLists.txt
    :language: cmake


Protobuf entity integration
===========================

In addtition to the previous example, you will need to call ``find_package(Protobuf REQUIRED)``.
Furthermore you want to handle the generation of the protobuf headers for your language.

.. literalinclude:: files/CMakeLists/protobuf/CMakeLists.txt
    :language: cmake

.. tip::
    The ``PROTOBUF_TARGET_CPP`` function is a convenience function from eCAL.
    If you have already worked with Protobuf and CMake, you may be more familiar with the following code, which basically does the same thing:

    .. code-block:: cmake
    
        include_directories(${CMAKE_CURRENT_BINARY_DIR})
        protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protobuf_files})
        add_executable(${PROJECT_NAME} ${source_files} ${PROTO_SRCS} ${PROTO_HDRS}) 


Common CMake targets
====================

It follows a list of common eCAL CMake targets.

.. list-table:: eCAL CMake targets
  :header-rows: 1
  :widths: 10 90
  
  * - CMake target
    - Description
  * - eCAL::core
    - C++ core functionality
  * - eCAL::string_core
    - C++ std::string publisher/subscriber extension (implicitly links eCAL::core)
  * - eCAL::protobuf_core
    - C++ protobuf publisher/subscriber/client/server extension (implicitly links eCAL::core)
  * - eCAL::capnproto_core
    - C++ capnproto publisher/subscriber extension (implicitly links eCAL::core)
  * - eCAL::flatbuffers_core
    - C++ flatbuffers publisher/subscriber extension (implicitly links eCAL::core)
  * - eCAL::measurement
    - read/write binary eCAL measurements
  * - eCAL::string_measurement
    - read/write eCAL measurements with string (implicitly links eCAL::measurement)
  * - eCAL::protobuf_measurement
    - read/write eCAL measurements with protobuf (implicitly links eCAL::measurement)
  * - eCAL::capnproto_measurement
    - read/write eCAL measurements with capnproto (implicitly links eCAL::measurement)
  * - eCAL::flatbuffers_measurement
    - read/write eCAL measurements with flatbuffers (implicitly links eCAL::measurement)
  * - eCAL::core_c
    - C core functionality 

 