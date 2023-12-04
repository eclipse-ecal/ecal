add_subdirectory(thirdparty/fineftp/fineftp-server/fineftp-server EXCLUDE_FROM_ALL)
set_property(TARGET server PROPERTY FOLDER lib/fineftp)
add_library(fineftp::server ALIAS server)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/Modules)