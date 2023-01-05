.. include:: /include.txt

.. _development_building_ecal_from_source:

=========================
Building eCAL from source
=========================

If you want to build eCAL yourself, this tutorial may help you with that.
To build eCAL, you will need a C++14 compliant compiler, such as Visual Studio 2015 or newer, or GCC 5 or newer.

Currently, we support:

* Windows 10 / 11
* Ubuntu 22.04 / 20.04 / 18.04

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

   * CMake (https://cmake.org)
   * Qt5 (>= 5.5 / 64-bit) (https://www.qt.io/download)

   .. note::

      If you have multiple Versions of Qt installed, eCAL will try to pick the latest match for your Visual Studio Version.

      If this fails (e.g. as you have copied the qt directory without properly installing it) or if you want to use a specific Qt5 Version, you may have to manually set the ``CMAKE_PREFIX_PATH`` or ``Qt5_DIR``.
      
#. Optional: Install additional dependendencies

   * `Python for Windows <https://www.python.org/downloads/>`_ (64 Bit, Version 3.9): To build the python extensions and the documentation
   * `Doxygen <https://www.doxygen.nl/download.html#srcbin>`_: To build the documentation
   * `Innosetup <https://jrsoftware.org/isdl.php>`_: To create an executable Installer

|fa-windows| Windows build
--------------------------

* **To just compile eCAL**:

  .. code-block:: batch

     mkdir _build\complete
     cd _build\complete
      
     cmake ../.. -A x64 -DCMAKE_INSTALL_PREFIX=_install -DBUILD_SHARED_LIBS=OFF
     cmake --build . --parallel --config Release

  This will create a :file:`_build\\complete\\` directory in your eCAL root folder and build eCAL there.

* **To build a complete setup**:

  #. For creating a setup, you have to also build the documentation and build the debug SDK binaries.
     
     To build the eCAL QT based applications, you will need to install additionally:

     - Qt5 as ``msvc2015_64`` (Or manually override it by setting ``Qt5_DIR``)

     Execute the following batch files to create the Visual Studio 2019 solution and build it.

     .. code-block:: batch

        build_win\win_make_cmake.bat
        build_win\win_make_build.bat

     This will create a :file:`_build\\` directory in your eCAL root folder and build eCAL in two configurations, there.

  #. Run the following batch files to create an eCAL setup as .exe installer:

     .. code-block:: batch

        build_win\win_make_setup.bat

     You will find the .exe installer in the :file:`_build\\complete\\_deploy\\` directory.

  #. Optional: If you configured CMake to build the python extension by setting `BUILD_PY_BINDING` to `ON` you can create the eCAL python wheel by calling

     .. code-block:: batch
      
        build_win\win_make_python_wheel.bat

     Afterwards you will find the python eCAL wheel in the :file:`_build\\_deploy\\` directory.
     Install the extension by

     .. code-block:: batch
   
        pip install ecal-...-win_amd64.whl

|fa-ubuntu| Building eCAL on Ubuntu
===================================

We support building on currently supported Ubuntu LTS releases.

.. seealso:: 

   The build described here is a very simple (yet complete and fully functional) build that differs from our "official" binaries, e.g. in regards of the library install directory and the :file:`ecal.ini` location-.
   If your goal is to replicate the official build, you should apply the CMake Options exactly as we do.
   You can grab those from our GitHub Action build scripts:

   - `Ubuntu 22.04 <https://github.com/eclipse-ecal/ecal/blob/master/.github/workflows/build-ubuntu-22.yml>`_
   - `Ubuntu 20.04 <https://github.com/eclipse-ecal/ecal/blob/master/.github/workflows/build-ubuntu-20.yml>`_
   - `Ubuntu 18.04 <https://github.com/eclipse-ecal/ecal/blob/master/.github/workflows/build-ubuntu-18.yml>`_

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

   .. tab:: Ubuntu 18.04

      #. Add the `official CMake repository <https://apt.kitware.com/>`_, as eCAL needs CMake >= 3.15:

         .. code-block:: bash

            wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
            echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ bionic main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
            sudo apt-get -y update
            sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
            sudo apt-get install kitware-archive-keyring

      #. Install the dependencies from the ordinary Ubuntu 18.04 repositories:

         .. code-block:: bash

            sudo apt-get install git cmake doxygen graphviz build-essential zlib1g-dev qtbase5-dev libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev libqwt-qt5-dev libyaml-cpp-dev

      #. If you plan to create the eCAL python language extension:

         .. code-block:: bash

            sudo apt-get install python3.6-dev python3-pip
            python3 -m pip install setuptools

|fa-ubuntu| Ubuntu 22/20/18 build
---------------------------------

#. Compile eCAL with the following options (additional set `BUILD_PY_BINDING` to `ON` if plan to build the python extension):

   .. code-block:: bash

      mkdir _build
      cd _build
      cmake .. -DCMAKE_BUILD_TYPE=Release -DECAL_THIRDPARTY_BUILD_PROTOBUF=OFF -DECAL_THIRDPARTY_BUILD_CURL=OFF -DECAL_THIRDPARTY_BUILD_HDF5=OFF -DECAL_THIRDPARTY_BUILD_QWT=OFF
      make -j4

#. Create a debian package and install it:

   .. code-block:: bash

      cpack -G DEB
      sudo dpkg -i _deploy/eCAL-*
      sudo ldconfig

#. Optional: Create and install the eCAL python wheel (Only available if you enabled the `BUILD_PY_BINDING` CMake option in step 1):

   .. code-block:: bash

      cmake --build . --target create_python_wheel --config Release
      sudo pip3 install _deploy/ecal5-*