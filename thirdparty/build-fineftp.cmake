add_subdirectory(thirdparty/fineftp-server/fineftp-server EXCLUDE_FROM_ALL)
set_property(TARGET server PROPERTY FOLDER lib/fineftp)
add_library(fineftp::server ALIAS server)