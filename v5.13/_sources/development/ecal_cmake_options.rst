.. include:: /include.txt

.. _development_ecal_cmake_options:

=============
CMake options
=============

eCAL is using CMake as build system.
The build makes use of CMake options to configure what features to build and to see what capabilities are available.
The options are grouped by their prefix:

- ``HAS_`` variables indicate if a certain feature is supported for the build. E.g setting ``HAS_QT5`` to off, will disable all components of eCAL that have a GUI attached.
- ``BUILD_`` variables turn on / off certain eCAL components. Those components can be language bindings, application (like monitor / player / recorder), documentation or tests
- ``ECAL_THIRDPARTY_`` variables control on a dependency by dependency basis, if that dependency is to be compiled alongside eCAL.

There are a few addional options which do not fit in these categories, but they are also documentd below.

All options can be passed on the command line cmake ``-D<option>=<value>`` or in the CMake GUI application.

+-------------------------------------------+---------+-----------------------------------------------------------------+
| CMake option                              | Default | Description                                                     |
+===========================================+=========+=================================================================+
| ``HAS_HDF5``                              | ``ON``  | Platform supports HDF5 library                                  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``HAS_QT``                                | ``ON``  | Platform supports Qt 5 6 library.                               |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``HAS_CURL``                              | ``ON``  | Build with CURL (i.e. upload support in the recorder app)       |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``HAS_CAPNPROTO``                         | ``OFF`` | Platform supports Cap'n Proto library                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``HAS_FLATBUFFERS``                       | ``OFF`` | Platform supports flatbuffers library                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``HAS_FTXUI``                             | ``ON``  | Platform supports FTXUI library. Requires C++17 and up.         |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_DOCS``                            | ``OFF`` | Build the eCAL documentation                                    |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_APPS``                            | ``ON``  | Build the eCAL applications                                     |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_SAMPLES``                         | ``ON``  | Build the eCAL samples                                          |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_TIME``                            | ``ON``  | Build the eCAL time interfaces                                  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_PY_BINDING``                      | ``OFF`` | Build eCAL python binding                                       |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_STANDAL``ON``E_PY_WHEEL``         | ``OFF`` | Build eCAL python binding as standalone wheel                   |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_CSHARP_BINDING``                  | ``OFF`` | Build eCAL C# binding                                           |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``BUILD_ECAL_TESTS``                      | ``OFF`` | Build the eCAL google tests                                     |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_INCLUDE_PY_SAMPLES``               | ``OFF`` | Include python language sample projects into CMake              |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_INSTALL_SAMPLE_SOURCES``           | ``ON``  | Install the sources of eCAL samples                             |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_NPCAP_SUPPORT``                    | ``OFF`` | Enable the eCAL Npcap Receiver (i.e. the Win10 performance fix) |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_USE_CLOCKLOCK_MUTEX``              | ``OFF`` | Use native mutex with monotonic clock (requires glibc >= 2.30)  |
+-------------------------------------------+---------+-----------------------------------------------------------------+
| ``ECAL_THIRDPARTY_BUILD_ASIO``            | ``ON``  | Build asio with eCAL                                            |
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
| ``ECAL_THIRDPARTY_BUILD_SIMPLEINI``       | ``ON``  | Build simpleini with eCAL                                       |
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
| ``CPACK_PACK_WITH_INNOSETUP``             | ``ON``  | Create Innosetup installer for the Windows build                |
+-------------------------------------------+---------+-----------------------------------------------------------------+

To build the eCAL csharp extension you need to set the CMake option ``BUILD_CSHARP_BINDING`` to ``ON``.
In order to make the CSharp Google::Protobuf extension work you need to install additionally the Microsoft package management tool `Nuget <https://www.nuget.org/downloads>`_.
Please ensure that the installation path is part of your windows user or system PATH environment variable.
Nuget will be used to download the .Net Google.Protobuf package automatically when building the extension.
