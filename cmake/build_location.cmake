# Set up the common directory structure for output libraries and binaries
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  # For Unix-like operating systems we can put all libraries into a dedicated
  # lib/ folder, whereas for Windows the libraries (dll files) need to be put
  # into the same directory as the executable, since Windows doesn't have an
  # RPATH equivalent where a search path can be put into the executable itself
  if(UNIX)
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  else()
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  endif()
endif()