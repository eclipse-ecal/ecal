add_subdirectory(thirdparty/tcpub/tcpub EXCLUDE_FROM_ALL)
set_property(TARGET tcpub PROPERTY FOLDER lib/tcpub)
add_library(tcpub::tcpub ALIAS tcpub)