.. include:: /include.txt

.. _development_building_ecal_with_conan_dependencies:

=======================================
Building eCAL with Conan dependencies
=======================================

Conan is a dependency manager, that has a wide range of precompiled open source libraries available. 
Building eCAL with Conan dependencies is mainly a shortcut for Windows users developing eCAL who do not want to build all dependencies from source.
It lets a developer get up to speed very quickly.
We certainly **do not recommend** to build eCAL installers from Conan dependencies at this point.

The Conan build has been tested for VS2019 only, and dependencies might not be aligned with the version included as submodules.


Build instructions
------------------------------------

To build with Conan you need to have a Conan installation.
It's best to install Conan inside a virtual environment:

.. code-block:: bat

   # Create virtual environment
   python3 -m venv .venv
   # Windows
   .venv\Scripts\activate.bat|ps1
   # Linux
   source .venv\bin\activate
   # Install Conan
   pip install conan
  
You need to set up a Conan profile to match your development.
For reference, see https://docs.conan.io/en/latest/reference/profiles.html.

First, let Conan install all dependencies, as specified in ``conanfile.py``:

.. code-block:: bat

   mdkir _build
   cd _build
   conan install .. --profile <your-profile> --build missing
  
eCAL uses the ``WinExtras`` module from QT, which is why, unfortunately, there are no prebuilt QT libraries.
This is why we need to build QT from source and thus pass a ``--build missing`` when running ``conan install``.  
Conan will create a lot of files in the build folder.
Some are CMake scripts to locate dependencies, others are scripts to use / activate special environments to allow you to build / debug eCAL in a way that all libraries can be located at runtime.
Thus we recommend to build eCAL with the following commands:

.. code-block:: bat

   activate.bat|ps1|sh
   mkdir _cmake
   cd _cmake
   cmake ../.. -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../conan_toolchain.cmake
   cmake --build .
  
Alternatively, if you can also open the generated Visual Studio solution when working on Windows, and build / debug from within Visual Studio.:

.. code-block::bat

   .\eCAL.sln
 
Beware, that depending on the settings in your Conan profile, the build supports **either** Debug **or** Release builds. Make sure, when building from Visual Studio, to build the appropriate configuration.
If you have a ``Release`` Conan profile, but try to build for the ``Debug`` configuration from Visual Studio, you will receive build errors.
