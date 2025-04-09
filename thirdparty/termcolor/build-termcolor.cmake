include_guard(GLOBAL)

ecal_variable_push(CMAKE_POLICY_VERSION_MINIMUM)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/termcolor" "${eCAL_BINARY_DIR}/thirdparty/termcolor" EXCLUDE_FROM_ALL SYSTEM)
ecal_variable_pop(CMAKE_POLICY_VERSION_MINIMUM)
