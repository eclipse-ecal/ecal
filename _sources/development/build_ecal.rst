.. include:: /include.txt

.. _build_ecal_main_page:

=============
Building eCAL
=============

As it holds for most C++ applications, it's not entirely trivial to build eCAL from sources.
The eCAL build is based on CMake, with many options to configure which parts of eCAL (not) to build and how to find all dependencies necessary to build eCAL.

The build has been structured in such a way, that (only) specific components can be compiled (e.g. the pure communication SDK (eCAL core), specific language bindings, specific apps (with or without GUI capabilities).
The complete list of all configuration options can be found in :ref:`development_ecal_cmake_options`.

----------------------------
Providing build dependencies
----------------------------

As there is no standard dependency management system for C++, the eCAL build tries to maintain maximum flexibility for the user wishing to build eCAL from sources and integrating eCAL into their own build system.
At the same time, it tries to provide out of the box support for users not so experienced with the whole C++ ecosystem (reference :ref:`development_building_ecal_from_source` for a quick guide how to get started).
Therefore, the user can decide on a dependency-by-dependency basis if they want a dependency to be built as part of the eCAL build, or not.

To build the dependency alongside eCAL (provided as a submodule), the CMake option ``ECAL_THIRDPARTY_BUILD_<DEPENDENCY_NAME>`` has to be set to ``ON``.
This is the default behavior, except for some Linux builds, where fitting system packages are available.

The user can also choose to rather take care of making the dependency available to CMake by themselves.
This can be done by building and installing the dependency, setting ``ECAL_THIRDPARTY_BUILD_<DEPENDENCY_NAME>`` to ``OFF`` and providing a ``CMAKE_PREFIX_PATH`` to CMake for the install location.

It is also possible to provide dependencies for an eCAL build using Conan, see :ref:`development_building_ecal_with_conan_dependencies`.

--------------------------------------
Building eCAL Core as a static library
--------------------------------------

At the moment, eCAL Core (C++ communication component) can only be built as a dynamic library, regardless of what is passed to CMake through the ``BUILD_SHARED_LIBS`` variable.
There has been quite some discussion about this topic.
Creating a static library might possibly lead to eCAL Core being linked multiple times into the same application.
This will most certainly lead to communication problems due to eCAL's internal implementation.
Therefore, the build disallows the creation of a static library for eCAL Core.

As there may still be use cases in which a static library would be beneficial, the eCAL build may reallow static library builds in the future.


.. toctree::
   :maxdepth: 1
   
   building_ecal_from_source
   ecal_cmake_options
