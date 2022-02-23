.. include:: /include.txt

.. _development_building_ecal_with_conan_dependencies:

=====================================
Building eCAL with Conan dependencies
=====================================

.. important::
   As eCAL uses the ``CMakeDeps`` generator, unfortunately a rather old Conan version (1.41.0) has to be used at the moment, because internal Conan behavior was changed after that version and not all recipes on CCI yet support ``CMakeDeps`` generator.
   The release of a new ``markupsafe`` package has broken the API and breaks older ``Jinja2`` versions which Conan 1.41.0 uses.

   Hence the Conan build of eCAL **may not be functional** at this point of time.


Conan is a dependency manager, that has a wide range of precompiled open source libraries available. 
Building eCAL with Conan dependencies is mainly a shortcut for Windows users developing eCAL who do not want to build all dependencies from source.
It lets a developer get up to speed very quickly.
We certainly **do not recommend** to build eCAL installers from Conan dependencies at this point.

The Conan build has been tested for VS2019 only, and dependencies might not be aligned with the version included as submodules.


Build instructions
------------------

To build with Conan you need to have a Conan installation.
It's best to install Conan inside a virtual environment:

.. code-block:: console

   # Create virtual environment
   python3 -m venv .venv
   # Windows
   .venv\Scripts\activate.bat|ps1
   # Linux
   source .venv\bin\activate
   # Install Conan
   pip install conan==1.41.0
  
  
You need to set up a Conan profile to match your development.
For reference, see https://docs.conan.io/en/latest/reference/profiles.html.

First, let Conan install all dependencies, as specified in ``conanfile.py``:

.. code-block:: bat

   mdkir _build
   cd _build
   conan install .. --profile <your-profile> --build missing
  
eCAL uses the ``WinExtras`` module from Qt, which is why, unfortunately, there are no prebuilt QT libraries.
This is why we need to build Qt from source and thus pass a ``--build missing`` when running ``conan install``.  
Conan will create a lot of files in the build folder.
Some are CMake scripts to locate dependencies, others are scripts to use / activate special environments to allow you to build / debug eCAL in a way that all libraries can be located at runtime.
Thus, we recommend to build eCAL with the following commands:

.. code-block:: bat

   activate.bat|ps1|sh
   mkdir _cmake
   cd _cmake
   cmake ../.. -DCMAKE_TOOLCHAIN_FILE:FILEPATH=../conan_toolchain.cmake
   cmake --build .
  
Alternatively, if you can also open the generated Visual Studio solution when working on Windows, and build / debug from within Visual Studio.:

.. code-block::bat

   .\eCAL.sln
 
Beware, that depending on the settings in your Conan profile, the build supports **either** Debug **or** Release builds.
Make sure, when building from Visual Studio, to build the appropriate configuration.
If you have a ``Release`` Conan profile, but try to build for the ``Debug`` configuration from Visual Studio, you will receive build errors.
