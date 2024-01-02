#[=======================================================================[.rst:
eCALAtomic
-----------

This adds a target to providing a target to link atomic if necessary

Imported Targets
^^^^^^^^^^^^^^^^

``eCAL::atomic``
  A wrapper for linking to atomic if necessary.

Result Variables
^^^^^^^^^^^^^^^^

The following variables are set:

``eCAL_LIB_ATOMIC``
  Value of library linked for atomic.
#]=======================================================================]


# We try and compile some test code and see if it compiles without providing -latomic.
# If it does, atomic is buildin and does not need to be linked separatly.
set(ATOMIC_TEST_CODE "
      #include <atomic>
      int main() { std::atomic<int64_t> i(0); i++; return 0; }
")
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("${ATOMIC_TEST_CODE}" ATOMIC_IS_BUILTIN)

if (ATOMIC_IS_BUILTIN)
  set(eCAL_LIB_ATOMIC )
else()
  set(eCAL_LIB_ATOMIC atomic)
endif()
  
if(NOT TARGET eCAL::atomic)
  add_library(eCAL::atomic INTERFACE IMPORTED)

  if(eCAL_LIB_ATOMIC)
    set_property(TARGET eCAL::atomic PROPERTY INTERFACE_LINK_LIBRARIES "${eCAL_LIB_ATOMIC}")
  endif()
endif()