# Create an imported hdf5 target for use in the cmake scripts.
function(create_targets_hdf5)
  find_package(HDF5 REQUIRED)
  if (NOT TARGET hdf5::hdf5)
    if (WIN32)
      add_library(hdf5::hdf5 STATIC IMPORTED)
      set_target_properties(hdf5::hdf5 PROPERTIES 
              INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIRS}"
              IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
              IMPORTED_LOCATION_DEBUG "${HDF5_LIBRARIES_DEBUG}"
              IMPORTED_LOCATION_RELEASE "${HDF5_LIBRARIES_OPTIMIZED}"
              IMPORTED_IMPLIB_DEBUG "${HDF5_LIBRARIES_DEBUG}"
              IMPORTED_IMPLIB_RELEASE "${HDF5_LIBRARIES_OPTIMIZED}"
              )  
    endif (WIN32)    
  endif(NOT TARGET hdf5::hdf5)
endfunction()