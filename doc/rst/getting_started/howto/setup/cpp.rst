.. include:: /include.txt

=================
C/C++ Setup
=================

To set up your environment for C++ development with eCAL, you will need:

- A C++ compiler (e.g., GCC, Clang, or MSVC) compatible with C++14 or newer.
- CMake (version 3.16 or higher) for project configuration and build management.
- An installed eCAL SDK, as described in the general installation instructions.
- Optionally, additional libraries such as Protobuf, Cap'n Proto, or FlatBuffers if you plan to use those features.


.. _getting_started_cmake_integration:

Including eCAL in CMake Projects
====================================

For a basic integration, ``find_package(eCAL REQUIRED)`` needs to be called.
CMake locates libraries using the ``find_package`` command, which searches for installed packages and their configuration files. 
When installing eCAL via PPA, Debian package, or the Windows installer, the necessary CMake configuration files are placed in standard system locations. 
This means that ``find_package(eCAL REQUIRED)`` should work out of the box, and CMake will automatically find eCAL without requiring manual path configuration.

Furthermore, the ``eCAL::core`` target needs to be linked in order to be able to create binary publishers and subscribers.

The :file:`CMakeLists.txt` looks as follows:

.. literalinclude:: /source_code_samples/cmake/binary/CMakeLists.txt
    :language: cmake


Protobuf entity integration
===========================

When using special serialization protocols like Protobuf, a few more steps are necessary for a proper CMake integration.
In addition to the previous example, you will need to call ``find_package(Protobuf REQUIRED)``.
Furthermore you want to handle the generation of the protobuf headers for your language.

.. literalinclude:: /source_code_samples/cmake/protobuf/CMakeLists.txt
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

Below, please find an overview of the different CMake targets that are available for eCAL.

.. list-table:: eCAL CMake targets
  :header-rows: 1
  :widths: 10 90
  
  * - CMake target
    - Description
  * - ``eCAL::core``
    - C++ core functionality
  * - ``eCAL::core_c``
    - C core functionality 
  * - ``eCAL::string_core``
    - C++ ``std::string`` publisher/subscriber extension (implicitly links ``eCAL::core``)
  * - ``eCAL::protobuf_core``
    - C++ protobuf publisher/subscriber/client/server extension (implicitly links ``eCAL::core``)
  * - ``eCAL::capnproto_core``
    - C++ capnproto publisher/subscriber extension (implicitly links ``eCAL::core``)
  * - ``eCAL::flatbuffers_core``
    - C++ flatbuffers publisher/subscriber extension (implicitly links ``eCAL::core``)
  * - ``eCAL::measurement``
    - read/write binary eCAL measurements
  * - ``eCAL::string_measurement``
    - read/write eCAL measurements with string (implicitly links ``eCAL::measurement``)
  * - ``eCAL::protobuf_measurement``
    - read/write eCAL measurements with protobuf (implicitly links ``eCAL::measurement``)
  * - ``eCAL::capnproto_measurement``
    - read/write eCAL measurements with capnproto (implicitly links ``eCAL::measurement``)
  * - ``eCAL::flatbuffers_measurement``
    - read/write eCAL measurements with flatbuffers (implicitly links ``eCAL::measurement``)

 