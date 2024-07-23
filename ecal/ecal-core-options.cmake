# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
# available (optional) thirdparty dependencies
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
option(ECAL_CORE_HAS_PROTOBUF                            "Protobuf library is available, eCAL protobuf support will be enabled"                                  ON)

# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
# additional builds (adapt to your needs)
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
option(ECAL_CORE_BUILD_SAMPLES                           "Build the eCAL samples"                                                                                ON)
option(ECAL_CORE_BUILD_TESTS                             "Build the eCAL google tests"                                                                           ON)

# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
# core internal feature configuration (adapt to your needs)
# ------------------------------------------------------------------------------------------------------------------------------------------------------------------
option(ECAL_CORE_CONFIGURATION                           "Enables to configure eCAL at runtime via ecal.yaml file"                                               ON)
option(ECAL_CORE_COMMAND_LINE                            "Enables eCAL application cmd line interfaces"                                                          ON)
option(ECAL_CORE_REGISTRATION                            "Enables the eCAL registration layer"                                                                   ON)
option(ECAL_CORE_MONITORING                              "Enables the eCAL monitoring functionality"                                                             ON)
option(ECAL_CORE_PUBLISHER                               "Enables the eCAL publisher functionality"                                                              ON)
option(ECAL_CORE_SUBSCRIBER                              "Enables the eCAL subscriber functionality"                                                             ON)
option(ECAL_CORE_SERVICE                                 "Enables the eCAL server/client functionality"                                                          ON)
option(ECAL_CORE_TIMEPLUGIN                              "Enables the eCAL time plugin functionality"                                                            ON)
option(ECAL_CORE_NPCAP_SUPPORT                           "Enable the eCAL Npcap Receiver (Win10 performance fix for UDP communication)"                         OFF)

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
