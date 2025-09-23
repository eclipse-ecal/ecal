.. include:: /include.txt

.. _development_ecal_cmake_options:

=============
CMake options
=============

eCAL is using CMake as build system.
The build makes use of CMake options to configure what features to build and to see what capabilities are available.
The options are grouped by their prefix:

- ``ECAL_USE_`` variables indicate if eCAL shall use a certain dependency. E.g setting ``ECAL_USE_QT`` to off, will disable all components of eCAL that have a (Qt) GUI attached.
- ``ECAL_BUILD_`` variables turn on / off certain eCAL components. Those components can be language bindings, application (like monitor / player / recorder), documentation or tests
- ``ECAL_THIRDPARTY_BUILD`` variables control on a dependency by dependency basis, if that dependency is to be compiled alongside eCAL.

There are a few addional options which do not fit in these categories, but they are also documentd below.

All options can be passed on the command line cmake ``-D<option>=<value>`` or in the CMake GUI application.

+-------------------------------------------+---------+-----------------------------------------------------------------+
| CMake option                              | Default | Description                                                     |
+===========================================+=========+=================================================================+
| ``ECAL_USE_HDF5``                         | ``ON``  | Platform supports HDF5 library                                  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_QT``                           | ``ON``  | Platform supports Qt 5 6 library.                               |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_CURL``                         | ``ON``  | Build with CURL (i.e. upload support in the recorder app)       |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_CAPNPROTO``                    | ``OFF`` | Platform supports Cap'n Proto library                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_FLATBUFFERS``                  | ``OFF`` | Platform supports flatbuffers library                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_FTXUI``                        | ``ON``  | Platform supports FTXUI library. Requires C++17 and up.         |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_DOCS``                       | ``OFF`` | Build the eCAL documentation                                    |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_APPS``                       | ``ON``  | Build the eCAL applications                                     |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_SAMPLES``                    | ``ON``  | Build the eCAL samples                                          |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_TIMEPLUGINS``                | ``ON``  | Build the eCAL time interfaces                                  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_PY_BINDING``                 | ``OFF`` | Build eCAL python binding                                       |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_STANDALONE_PY_WHEEL``             | ``OFF`` | Build eCAL python binding as standalone wheel                   |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_CSHARP_BINDING``             | ``OFF`` | Build eCAL C# binding                                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_TESTS``                      | ``OFF`` | Build the eCAL google tests                                     |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_BUILD_BENCHMARKS``                 | ``OFF`` | Build the eCAL benchmarks                                       |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_INSTALL_SAMPLE_SOURCES``           | ``ON``  | Install the sources of eCAL samples                             |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_NPCAP``                        | ``OFF`` | Enable the eCAL Npcap Receiver (i.e. the Win10 performance fix) |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_CLOCKLOCK_MUTEX``              | ``OFF`` | Use native mutex with monotonic clock (requires glibc >= 2.30)  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_ASIO``            | ``ON``  | Build asio with eCAL                                            |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_BENCHMARK``       | ``ON``  | Build Google Benchmark with eCAL                                |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS`` | ``ON``  | Build CMakeFunctions with eCAL                                  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_FINEFTP``         | ``ON``  | Build fineFTP with eCAL                                         |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_FTXUI``           | ``ON``  | Build ftxui with eCAL                                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_GTEST``           | ``OFF`` | Build gtest with eCAL                                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_RECYCLE``         | ``ON``  | Build steinwurf::recylce with eCAL                              |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_SPDLOG``          | ``ON``  | Build spdlog with eCAL                                          |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_TCLAP``           | ``ON``  | Build tclap library with eCAL                                   |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_TCP_PUBSUB``      | ``ON``  | Build tcp_pubsub library with eCAL                              |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_TERMCOLOR``       | ``ON``  | Build termcolor with eCAL                                       |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_TINYXML2``        | ``ON``  | Build tinyxml2 with eCAL                                        |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_UDPCAP``          | ``OFF`` | Build udpcap library with eCAL                                  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_PROTOBUF``        | ``ON``  | Build protobuf with eCAL                                        |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_YAML-CPP``        | ``ON``  | Build yaml-cpp with eCAL                                        |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_CURL``            | ``ON``  | Build CURL with eCAL                                            |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_HDF5``            | ``ON``  | Build HDF5 with eCAL                                            |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_QWT``             | ``ON``  | Build qwt::qwt with eCAL                                        |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_LINK_HDF5_SHARED``                 | ``ON``  | Link shared libs of HDF5                                        |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_CPACK_PACK_WITH_INNOSETUP``        | ``ON``  | Create Innosetup installer for the Windows build                |
+-------------------------------------------+---------+-----------------------------------------------------------------+

To build the eCAL CSharp extension you need to set the CMake option ``ECAL_BUILD_CSHARP_BINDING`` to ``ON``.
In order to make the CSharp Google::Protobuf extension work you need to install additionally the Microsoft package management tool `Nuget <https://www.nuget.org/downloads>`_.
Please ensure that the installation path is part of your windows user or system PATH environment variable.
Nuget will be used to download the .Net Google.Protobuf package automatically when building the extension.
