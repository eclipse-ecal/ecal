set(Npcap_FOUND True)

set(Npcap_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/npcap/npcap-sdk")

set(Npcap_INCLUDE_DIR ${Npcap_DIR}/Include/)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(Npcap_LIB_DIR "${Npcap_DIR}/Lib/x64")
else()
  set(Npcap_LIB_DIR "${Npcap_DIR}/Lib")
endif()

set(Npcap_LIBS
  "${Npcap_LIB_DIR}/Packet.lib" 
  "${Npcap_LIB_DIR}/wpcap.lib"
)