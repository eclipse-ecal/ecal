.. _migration:

======================
eCAL 6 Migration Guide
======================

eCAL 6 is the next major release after eCAL 5.
There have been a list of braking changes, this migration guide explains the most important points that need to be taken care of.


C++ Core API
======================================================


Header files
~~~~~~~~~~~~~~

In general, you can include the convenience header file ``#include <ecal/ecal.h>``.
All additional header files were prefixed with ``ecal_``, e.g. ``ecal/ecal_defs.h``. For eCAL6, all ``ecal_`` prefixes have been removed.
Publisher / Subscriber related files have been moved to the ``ecal/pubsub`` folder.


Removed functions
~~~~~~~~~~~~~~~~~~~~~~




Configuration API
~~~~~~~~~~~~~~~~~~~~~

Previously, eCAL was configured mainly by it's configuration file (``eCAL.ini``).
The file format was changed to ``eCAL.yaml`` to allow for hierarchical configurations.
At the same time, it's now possible, to manually set all configuration through code, too.
This is especially useful for hard configuration of communication system, like for executing unit tests.
For more information / usage, please read the section about configuration.


C Core API
======================================================




C# Core API
======================================================



Python Core API
======================================================


CMake / Build system
=====================================

We have made a few changes to the CMake build files

CMake Options
~~~~~~~~~~~~~~

A lot of CMake options were prefixed with `ECAL_` to avoid nameclashes with other libraries and / or renamed slightly.


.. csv-table:: Example :rst:dir:`csv-table`
   :header: "HOld variable", "New variable"
   
  ``HAS_HDF5``,                  ``ECAL_USE_HDF5``
  ``HAS_QT``,                    ``ECAL_USE_QT``
  ``HAS_CURL``,                  ``ECAL_USE_CURL``
  ``HAS_FTXUI``,                 ``ECAL_USE_FTXUI``
  ``HAS_CAPNPROTO``,             ``ECAL_USE_CAPNPROTO``
  ``HAS_FLATBUFFERS``,           ``ECAL_USE_FLATBUFFERS``
  ``HAS_MESSAGEPACK``,           ``ECAL_USE_MESSAGEPACK``
  ``BUILD_DOCS``,                ``ECAL_BUILD_DOCS``
  ``BUILD_APPS``,                ``ECAL_BUILD_APPS``
  ``BUILD_SAMPLES``,             ``ECAL_BUILD_SAMPLES``
  ``BUILD_TIME``,                ``ECAL_BUILD_TIMEPLUGINS``
  ``BUILD_C_BINDING``,           ``ECAL_BUILD_C_BINDING``
  ``BUILD_PY_BINDING``,          ``ECAL_BUILD_PY_BINDING``
  ``BUILD_CSHARP_BINDING``,      ``ECAL_BUILD_CSHARP_BINDING``
  ``ECAL_NPCAP_SUPPORT``,        ``ECAL_USE_NPCAP``
  ``BUILD_ECAL_TESTS``,          ``ECAL_BUILD_TESTS``
  ``CPACK_PACK_WITH_INNOSETUP``, ``ECAL_CPACK_PACK_WITH_INNOSETUP``


CMake Targets
~~~~~~~~~~~~~~

A few targets have changed their name, and there are a few dedicated targets.
Basically, ``eCAL::core`` only contains the core functionality, but no message / serialization support.
If you need to use the functionality, please link the following:

- Protobuf: ``eCAL::protobuf_core`
- Capnproto: ``eCAL::capnproto_core`
- String: ``eCAL::string_core`
 
These targets will automatically publicly link the serialization libraries, so no need to additionally link e.g. `protobuf::libprotobuf``.