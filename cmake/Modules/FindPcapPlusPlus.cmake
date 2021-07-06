set(PcapPlusPlus_FOUND True)

set(PcapPlusPlus_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/npcap/pcapplusplus-21.05-windows-vs2015/")

set(PcapPlusPlus_INCLUDE_DIR "${PcapPlusPlus_DIR}/header/")

if(("${CMAKE_GENERATOR_PLATFORM}" MATCHES "x64") OR ("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)"))
  set(PcapPlusPlus_LIB_DIR "${PcapPlusPlus_DIR}/x64/")
else()
  set(PcapPlusPlus_LIB_DIR "${PcapPlusPlus_DIR}/x86/")
endif()

set(PcapPlusPlus_LIBS
    optimized "${PcapPlusPlus_LIB_DIR}/Release/Common++.lib" 
    optimized "${PcapPlusPlus_LIB_DIR}/Release/Packet++.lib" 
    optimized "${PcapPlusPlus_LIB_DIR}/Release/Pcap++.lib" 

    debug "${PcapPlusPlus_LIB_DIR}/Debug/Common++.lib" 
    debug "${PcapPlusPlus_LIB_DIR}/Debug/Packet++.lib" 
    debug "${PcapPlusPlus_LIB_DIR}/Debug/Pcap++.lib" 
)
