include_guard(GLOBAL)

  # Save old CMake output paths. HDF5 will 1. require them and 2. pollute them
  set(OLD_CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  set(OLD_CMAKE_Fortran_MODULE_DIRECTORY "${CMAKE_Fortran_MODULE_DIRECTORY}")
  set(OLD_CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
  set(OLD_CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_PDB_OUTPUT_DIRECTORY}")
  set(OLD_CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  set(OLD_BUILD_SHARED_LIBS              ${BUILD_SHARED_LIBS})
  
  # Fill in variables that have not been set. If a cache variable has been set already, it will not be set by cmake.
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/" CACHE PATH "")
  set(CMAKE_Fortran_MODULE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/" CACHE PATH "")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/" CACHE PATH "")
  set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/bin/" CACHE PATH "")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/" CACHE PATH "")
  set(BUILD_SHARED_LIBS              ON                         CACHE BOOL "Build hdf5 shared" FORCE)
  
  #we will need threads
  find_package(Threads REQUIRED)

  set(HDF5_GENERATE_HEADERS     OFF CACHE BOOL "Do not re-generate auto-generated files" FORCE)
  set(ONLY_SHARED_LIBS          ON CACHE BOOL  "Only Build Shared Libraries" FORCE)
  set(HDF5_ENABLE_THREADSAFE    ON  CACHE BOOL "Enable thread-safety" FORCE)
  set(HDF5_ENABLE_SZIP_SUPPORT  OFF  CACHE BOOL "Disable szip support" FORCE) 
  set(HDF5_ENABLE_Z_LIB_SUPPORT OFF  CACHE BOOL "Disable zlib support" FORCE)   
  set(BUILD_TESTING             OFF CACHE BOOL "Do not build HDF5 Unit Testing" FORCE)
  set(HDF5_BUILD_UTILS          OFF CACHE BOOL "Do not build HDF5 Utils" FORCE)
  set(HDF5_BUILD_TOOLS          OFF CACHE BOOL "Do not build HDF5 Tools" FORCE)
  set(HDF5_BUILD_EXAMPLES       OFF CACHE BOOL "Do not build HDF5 Library Examples" FORCE)
  set(HDF5_BUILD_CPP_LIB        OFF CACHE BOOL "Do not build C++ lib" FORCE)
  set(HDF5_BUILD_HL_LIB         OFF CACHE BOOL "Do not build hdf5-hl" FORCE)
  set(HDF5_USE_GNU_DIRS         ON  CACHE BOOL "Use GNU install dirs" FORCE)

  # TODO: Let HDF5 use its own targets.
  # Adding the HDF5 targets to the eCALCoreTargets is a hack that we have had
  # before and so I am adding it again, to not break compatibility in eCAL 5.8
  # and 5.9. It however even prevents using a find_package(hdf5).
  # So in the future we should definitively remove this line of code.
  set(HDF5_EXPORTED_TARGETS eCALCoreTargets)                                    
  
  #We need to build hdf5 as shared to enable the threadsafe option. HDF5 uses the BUILD_SHARED_LIBS to check if shared build is on.
  # Hence we need to save the old value, enable it, and then set it back to the old value

  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/hdf5 thirdparty/hdf5 SYSTEM)

  if(NOT TARGET hdf5::hdf5-shared)
    add_library(hdf5::hdf5-shared ALIAS hdf5-shared)
    get_target_property(is_imported hdf5-shared IMPORTED)
    if (NOT is_imported)
      target_compile_options(hdf5-shared PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
      )
    endif()
  endif()

  target_include_directories(hdf5-shared INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/thirdparty/hdf5/hdf5>")
  
  if("${OLD_CMAKE_ARCHIVE_OUTPUT_DIRECTORY}" STREQUAL "")
    unset(CMAKE_ARCHIVE_OUTPUT_DIRECTORY CACHE)
  endif()
  
  if("${OLD_CMAKE_Fortran_MODULE_DIRECTORY}" STREQUAL "")
    unset(CMAKE_Fortran_MODULE_DIRECTORY CACHE)
  endif()
  
  if("${OLD_CMAKE_LIBRARY_OUTPUT_DIRECTORY}" STREQUAL "")
    unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY CACHE)
  endif()
  
  if("${OLD_CMAKE_PDB_OUTPUT_DIRECTORY}" STREQUAL "")
    unset(CMAKE_PDB_OUTPUT_DIRECTORY CACHE)
  endif()
  
  if("${OLD_CMAKE_RUNTIME_OUTPUT_DIRECTORY}" STREQUAL "")
    unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY CACHE)
  endif()
  
  set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS} CACHE BOOL "Build hdf5 shared" FORCE)
