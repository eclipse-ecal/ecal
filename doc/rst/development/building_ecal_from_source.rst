﻿.. include:: /include.txt

.. _development_building_ecal_from_source:

=========================
Building eCAL from source
=========================

If you want to build eCAL yourself, this tutorial may help you with that.

To build eCAL, you will need:

- A **C++17 compliant compiler**, such as Visual Studio 2019 or newer, or GCC 8 or newer.
  
  *A C++14 compliant compiler is sufficient, if you only want to build the eCAL Core Library.*

- **Qt 5.12 or newer** (Qt6 is supported, too, since eCAL 5.13) for building the eCAL Qt based applications.

Currently, we support:

* Windows 10 / 11
* Ubuntu 22.04 / 20.04

.. seealso::

   To learn more about the available CMake options, please check out the ":ref:`development_ecal_cmake_options`" section!

Repository checkout
===================

First check out the eCAL repository and all of the submodules:

.. code-block:: batch

   git clone https://github.com/eclipse-ecal/ecal.git
   cd ecal
   git submodule init
   git submodule update

|fa-windows| Building eCAL on Windows
=====================================

|fa-windows| Windows Dependencies
---------------------------------

#. Download and install the build-dependencies

   * **Visual Studio** (2019 or newer / Toolset ≥ v142) (https://visualstudio.microsoft.com/downloads/)

   * **CMake** (https://cmake.org)

   * **Qt** (Qt6 ≥ 6.5 or Qt5 ≥ 5.12) (https://www.qt.io/download)

     *Choose the appropriate version for your compiler, e.g. msvc2019_64 for Visual Studio 2019.*

   .. note::

      - Qt 6 is supported since eCAL 5.13.

      - If you have multiple Versions of Qt installed, eCAL will try to pick the latest match for your Visual Studio Version.

        If this fails (e.g. as you have copied the qt directory without properly installing it) or if you want to use a specific Qt Version, you may have to manually set the ``CMAKE_PREFIX_PATH`` or ``QT_DIR``.

      
#. Optional: Install additional dependendencies (not required for the simple build)

   * `Python for Windows <https://www.python.org/downloads/>`_ (64 Bit, Version 3.9): To build the python extensions and the documentation
   * `Doxygen <https://www.doxygen.nl/download.html#srcbin>`_: To build the documentation
   * `Innosetup <https://jrsoftware.org/isdl.php>`_: To create an executable Installer

|fa-windows| Windows build
--------------------------

.. seealso:: 

   The build described here is a very simple (yet complete and fully functional) build that differs from our "official" binaries, e.g. it does not contain the documentation and is not packaged as an installer.
   If your goal is to replicate the official build, you should apply the CMake Options exactly as we do.
   You can grab those from our GitHub Action build scripts:

   - `build-windows.yml <https://github.com/eclipse-ecal/ecal/blob/master/.github/workflows/build-windows.yml>`_

.. code-block:: batch

   mkdir _build\complete
   cd _build\complete
   
   rem Replace with your Qt installation path:
   set "CMAKE_PREFIX_PATH=C:/Qt/5.15.2/msvc2019_64"

   cmake ../.. -A x64 -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake/submodule_dependencies.cmake -DCMAKE_INSTALL_PREFIX=_install -DBUILD_SHARED_LIBS=OFF
   cmake --build . --parallel --config Release

This will create a :file:`_build\\complete\\` directory in your eCAL root folder and build eCAL there.


|fa-ubuntu| Building eCAL on Ubuntu
===================================

We support building on currently supported Ubuntu LTS releases.

.. seealso:: 

   The build described here is a very simple (yet complete and fully functional) build that differs from our "official" binaries, e.g. in regards of the library install directory and the :file:`ecal.yaml` location-.
   If your goal is to replicate the official build, you should apply the CMake Options exactly as we do.
   You can grab those from our GitHub Action build scripts:

   - `Ubuntu 22.04 <https://github.com/eclipse-ecal/ecal/blob/master/.github/workflows/build-ubuntu-22.yml>`_
   - `Ubuntu 20.04 <https://github.com/eclipse-ecal/ecal/blob/master/.github/workflows/build-ubuntu-20.yml>`_

|fa-ubuntu| Build dependencies
------------------------------

.. tabs::

   .. tab:: Ubuntu 22.04

      #. Install the dependencies from the ordinary Ubuntu 22.04 repositories:

         .. code-block:: bash

            sudo apt-get install git cmake doxygen graphviz build-essential zlib1g-dev qtbase5-dev libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev libqwt-qt5-dev libyaml-cpp-dev

      #. If you plan to create the eCAL python language extension:

         .. code-block:: bash

            sudo apt-get install python3.10-dev python3-pip
            python3 -m pip install setuptools

   .. tab:: Ubuntu 20.04

      #. Install the dependencies from the ordinary Ubuntu 20.04 repositories:

         .. code-block:: bash

            sudo apt-get install git cmake doxygen graphviz build-essential zlib1g-dev qtbase5-dev libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev libqwt-qt5-dev libyaml-cpp-dev

      #. If you plan to create the eCAL python language extension:

         .. code-block:: bash

            sudo apt-get install python3.8-dev python3-pip
            python3 -m pip install setuptools

|fa-ubuntu| Ubuntu 22.04 / 20.04 build
--------------------------------------

#. Compile eCAL with the following options (additional set `ECAL_BUILD_PY_BINDING` to `ON` if plan to build the python extension):

   .. code-block:: bash

      mkdir _build
      cd _build
      cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake/submodule_dependencies.cmake -DECAL_THIRDPARTY_BUILD_PROTOBUF=OFF -DECAL_THIRDPARTY_BUILD_CURL=OFF -DECAL_THIRDPARTY_BUILD_HDF5=OFF -DECAL_THIRDPARTY_BUILD_QWT=OFF
      make -j4

#. Create a debian package and install it:

   .. code-block:: bash

      cpack -G DEB
      sudo dpkg -i _deploy/eCAL-*
      sudo ldconfig

#. Optional: Create and install the eCAL python wheel (Only available if you enabled the `ECAL_BUILD_PY_BINDING` CMake option in step 1):

   .. code-block:: bash

      pip install wheel
      pip install -r ../doc/requirements.txt
      python -m pip install build
      python -m build .. --outdir _deploy
      sudo pip install _deploy/ecal5-*