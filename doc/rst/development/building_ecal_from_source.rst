.. include:: /include.txt

.. _development_building_ecal_from_source:

=========================
Building eCAL from source
=========================

If you want to build eCAL yourself, this tutorial may help you with that.
To build eCAL, you will need a C++14 compliant compiler, such as Visual Studio 2015 or newer, or GCC 5 or newer.

Currently, we support:

* Windows 7 / 10
* Ubuntu 16.04 / 18.04 / 20.04

.. seealso::

   To learn more about the available CMake options, please check out the ":ref:`development_ecal_cmake_options`" section!

Repository checkout
===================

First check out the eCAL repository and all of the submodules:

.. code-block:: batch

   git clone git://github.com/continental/ecal.git
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

   * `Python for Windows <https://www.python.org/downloads/>`_ (64 Bit, Version 3.x): To build the python extensions and the documentation
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

|fa-ubuntu| Ubuntu 16.04 dependencies
-------------------------------------

#. Add the `official cmake repository <https://apt.kitware.com/>`_, as eCAL needs cmake >= 3.13:

   .. code-block:: bash

      wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
      sudo apt-add-repository -y 'deb https://apt.kitware.com/ubuntu/ xenial main'
      sudo apt-get -y update
      sudo apt-get install kitware-archive-keyring
      sudo rm /etc/apt/trusted.gpg.d/kitware.gpg

#. Add a ppa for protobuf >= 3.0. The following (unofficial) ppa will be sufficient:

   .. code-block:: bash

      sudo add-apt-repository -y ppa:maarten-fonville/protobuf
      sudo apt-get -y update

#. Install the dependencies from the ordinary Ubuntu 16.04 repositories and the ppa we just added:

   .. code-block:: bash

      sudo apt-get install git cmake doxygen graphviz build-essential zlib1g-dev qt5-default libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev

|fa-ubuntu| Ubuntu 18.04 dependencies
-------------------------------------

#. Add the `official CMake repository <https://apt.kitware.com/>`_, as eCAL needs CMake >= 3.13:

   .. code-block:: bash

      wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
      sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
      sudo apt-get -y update
      sudo apt-get install kitware-archive-keyring
      sudo rm /etc/apt/trusted.gpg.d/kitware.gpg

#. Install the dependencies from the ordinary Ubuntu 18.04 repositories:

   .. code-block:: bash

      sudo apt-get install git cmake doxygen graphviz build-essential zlib1g-dev qt5-default libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev

#. If you plan to create the eCAL python language extension (here as an example for the python 3.6 version):

   .. code-block:: bash

      sudo apt-get install python3.6-dev python3-pip
      python3 -m pip install setuptools

|fa-ubuntu| Ubuntu 20.04 dependencies
-------------------------------------

#. Install the dependencies from the ordinary Ubuntu 20.04 repositories:

   .. code-block:: bash

      sudo apt-get install git cmake doxygen graphviz build-essential zlib1g-dev qt5-default libhdf5-dev libprotobuf-dev libprotoc-dev protobuf-compiler libcurl4-openssl-dev

|fa-ubuntu| Ubuntu 16/18/20 build
---------------------------------

#. Compile eCAL with the following options:

   .. code-block:: bash

      mkdir _build
      cd _build
      cmake .. -DCMAKE_BUILD_TYPE=Release -DECAL_THIRDPARTY_BUILD_PROTOBUF=OFF -DECAL_THIRDPARTY_BUILD_CURL=OFF -DECAL_THIRDPARTY_BUILD_HDF5=OFF
      make -j4

#. Create a debian package and install it:

   .. code-block:: bash

      cpack -G DEB
      sudo dpkg -i _deploy/eCAL-*

#. Optional: Create and install the eCAL python wheel (Only available if you enabled the `BUILD_PY_BINDING` CMake option in step 2):

   .. code-block:: bash

      cmake --build . --target create_python_wheel --config Release
      sudo pip3 install _deploy/ecal-*  
