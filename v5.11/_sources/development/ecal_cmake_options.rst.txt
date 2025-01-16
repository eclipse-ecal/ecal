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

========================================= ========= ============
CMake option                               Default   Description
========================================= ========= ============
``HAS_HDF5``                               ``ON``    Platform supports HDF5 library, necessary to build eCAL recording / replay tools
``HAS_QT5``                                ``ON``    Platform supports Qt 5 library, necessary to build eCAL monitoring tool
``HAS_CURL``                               ``ON``    Build with CURL (i.e. upload support in the recorder app)
``HAS_CAPNPROTO``                          ``OFF``   Platform supports Cap'n Proto library, necessary to use capnp serialization as message system and to enable eCAL monitoring capnp message reflection. eCAL does not add Cap'n Proto as a submodule. If you set this option to ``ON``, please make sure that the library is installed on your system and CMake can find it (consider setting CMAKE_PREFIX_PATH to point to the library).
``BUILD_DOCS``                             ``OFF``   Build the eCAL documentation, requires the installation of doxygen and a recent CMake version (>= 3.14 preferred, but some lower versions might work)
``BUILD_APPS``                             ``ON``    Build the eCAL applications, such as the monitoring tool
``BUILD_SAMPLES``                          ``ON``    Build the eCAL sample applications
``BUILD_TIME``                             ``ON``    Build the eCAL time interfaces, necessary if you want to use eCAL in time synchronized mode (based on PTP for example)
``BUILD_PY_BINDING``                       ``OFF``   Build the eCAL python language binding
``BUILD_STANDALONE_PY_WHEEL``              ``OFF``   Build eCAL python binding as standalone wheel
``BUILD_CSHARP_BINDING``                   ``OFF``   Build the eCAL C# language binding
``BUILD_ECAL_TESTS``                       ``OFF``   Build the eCAL google tests
``ECAL_INCLUDE_PY_SAMPLES``                ``OFF``   Include python language sample projects into CMake
``ECAL_INSTALL_SAMPLE_SOURCES``            ``ON``    Install the sources of eCAL samples
``ECAL_JOIN_MULTICAST_TWICE``              ``OFF``   Specific multicast network bug workaround
``ECAL_NPCAP_SUPPORT``                     ``OFF``   Enable the eCAL to use Npcap for udp socket communication (i.e. the Win10 performance fix)
``ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS``  ``ON``    Build CMakeFunctions with eCAL
``ECAL_THIRDPARTY_BUILD_SPDLOG``           ``ON``    Build Spdlog with eCAL, included as a submodule in the thirdparty folder. You can always use your custom spdlog installation, this is only for convenience.
``ECAL_THIRDPARTY_BUILD_TINYXML2``         ``ON``    Build tinyxml2 with eCAL, included as a submodule in the thirdparty directory.
``ECAL_THIRDPARTY_BUILD_FINEFTP``          ``ON``    Build fineFTP with eCAL, included as a submodule in the thirdparty directory.
``ECAL_THIRDPARTY_BUILD_TERMCOLOR``        ``ON``    Build termcolor with eCAL, included as a submodule in the thirdparty directory.
``ECAL_THIRDPARTY_BUILD_TCP_PUBSUB``       ``ON``    Build tcp_pubsub library with eCAL, included as a submodule in the thirdparty directory.
``ECAL_THIRDPARTY_BUILD_RECYCLE``          ``ON``    Build steinwurf::recylce with eCAL, included as a submodule in the thirdparty directory.
``ECAL_THIRDPARTY_BUILD_GTEST``            ``OFF``   Build GoogleTest with eCAL, included as a submodule in the thirdparty folder. You can always use your custom gtest installation, this is only for convenience.
``ECAL_THIRDPARTY_BUILD_PROTOBUF``         ``ON``    Build Protobuf with eCAL, included as a submodule in the thirdparty folder. You can always use your custom protobuf installation, this is only for convenience. Note, at least protobuf 3.0 is required to compile eCAL, we recommend using 3.11.4 or newer (tested with 3.11.4).
``ECAL_THIRDPARTY_BUILD_CURL``             ``ON``    Build CURL with eCAL, included as a submodule in the thirdparty folder.
``ECAL_THIRDPARTY_BUILD_HDF5``             ``ON``    Build HDF5 with eCAL, included as a submodule in the thirdparty folder.
``ECAL_THIRDPARTY_BUILD_YAML-CPP``         ``ON``    Build yaml-cpp with eCAL, included as a submodule in the thirdparty folder.
``CPACK_PACK_WITH_INNOSETUP``              ``ON``    Use Innosetup to create a Windows installer with cpack.
========================================= ========= ============

To build the eCAL csharp extension you need to set the CMake option ``BUILD_CSHARP_BINDING`` to ``ON``.
In order to make the CSharp Google::Protobuf extension work you need to install additionally the Microsoft package management tool `Nuget <https://www.nuget.org/downloads>`_.
Please ensure that the installation path is part of your windows user or system PATH environment variable.
Nuget will be used to download the .Net Google.Protobuf package automatically when building the extension.
