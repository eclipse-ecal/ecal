# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

project(core VERSION ${eCAL_VERSION_STRING})

# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
# core internal feature configuration - start
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
option(ECAL_CORE_CONFIG_INIFILE                          "Enables to configure eCAL via ecal.ini file"                                                           ON)
option(ECAL_CORE_COMMAND_LINE                            "Enables eCAL application cmd line interfaces"                                                          ON)
option(ECAL_CORE_REGISTRATION                            "Enables the eCAL registration layer"                                                                   ON)
option(ECAL_CORE_MONITORING                              "Enables the eCAL monitoring functionality"                                                             ON)
option(ECAL_CORE_PUBLISHER                               "Enables the eCAL publisher functionality"                                                              ON)
option(ECAL_CORE_SUBSCRIBER                              "Enables the eCAL subscriber functionality"                                                             ON)
option(ECAL_CORE_SERVICE                                 "Enables the eCAL server/client functionality"                                                          ON)
option(ECAL_CORE_TIMEPLUGIN                              "Enables the eCAL time plugin functionality"                                                            ON)
#option(ECAL_CORE_NPCAP_SUPPORT                           "Enable the eCAL Npcap Receiver (Win10 performance fix for UDP communication)"                         OFF)
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
# core registration layer options (change with care)
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
option(ECAL_CORE_REGISTRATION_SHM                        "Enables the eCAL registration layer over shared memory"                                                ON)
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
# core transport layer options (change with care)
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
option(ECAL_CORE_TRANSPORT_UDP                           "Enables the eCAL to transport payload via UDP multicast"                                               ON)
option(ECAL_CORE_TRANSPORT_TCP                           "Enables the eCAL to transport payload via TCP"                                                         ON)
option(ECAL_CORE_TRANSPORT_SHM                           "Enables the eCAL to transport payload via local shared memory"                                         ON)
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
# core internal feature configuration - end
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------

cmake_dependent_option(ECAL_CORE_NPCAP_SUPPORT "Enable the eCAL Npcap Receiver (Win10 performance fix for UDP communication)" ON "ECAL_NPCAP_SUPPORT" OFF)

add_subdirectory(core)
add_subdirectory(core_pb)
add_subdirectory(service)


message(STATUS "ECAL_CORE Build Options:")
message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "ECAL_CORE_CONFIG_INIFILE                       : ${ECAL_CORE_CONFIG_INIFILE}")
message(STATUS "ECAL_CORE_COMMAND_LINE                         : ${ECAL_CORE_COMMAND_LINE}")
message(STATUS "ECAL_CORE_REGISTRATION                         : ${ECAL_CORE_REGISTRATION}")
message(STATUS "ECAL_CORE_MONITORING                           : ${ECAL_CORE_MONITORING}")
message(STATUS "ECAL_CORE_PUBLISHER                            : ${ECAL_CORE_PUBLISHER}")
message(STATUS "ECAL_CORE_SUBSCRIBER                           : ${ECAL_CORE_SUBSCRIBER}")
message(STATUS "ECAL_CORE_SERVICE                              : ${ECAL_CORE_SERVICE}")
message(STATUS "ECAL_CORE_TIMEPLUGIN                           : ${ECAL_CORE_TIMEPLUGIN}")
message(STATUS "ECAL_CORE_NPCAP_SUPPORT                        : ${ECAL_CORE_NPCAP_SUPPORT}")
message(STATUS "ECAL_CORE_REGISTRATION_SHM                     : ${ECAL_CORE_REGISTRATION_SHM}")
message(STATUS "ECAL_CORE_TRANSPORT_UDP                        : ${ECAL_CORE_TRANSPORT_UDP}")
message(STATUS "ECAL_CORE_TRANSPORT_TCP                        : ${ECAL_CORE_TRANSPORT_TCP}")
message(STATUS "ECAL_CORE_TRANSPORT_SHM                        : ${ECAL_CORE_TRANSPORT_SHM}")
message(STATUS "--------------------------------------------------------------------------------")
