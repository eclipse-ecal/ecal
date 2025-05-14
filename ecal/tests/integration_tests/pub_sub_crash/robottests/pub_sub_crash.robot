*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     pub_sub_crash

*** Test Cases ***
Pub/Sub Network UDP
    [Tags]    pub_sub_network_udp
    Run PubSub With Monitor Test    network_udp    network

#Pub/Sub Network TCP
#   [Tags]    pub_sub_network_tcp
#    Run PubSub With Monitor Test    network_tcp    network

#Pub/Sub Local SHM
#    [Tags]    pub_sub_local_shm
#    Run Local All In One Test    local_shm

*** Keywords ***
Init Test Context
    Set Test Context    pub_sub_crash    pub_sub_crash
    ${build}=           Get Build Script Path
    ${net}=             Get Network Name
    ${args}=            Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}   ${build}
    Set Suite Variable    ${NETWORK}        ${net}

    Log To Console    [SETUP] Building Docker image...
    ${result}=        Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0    Docker build failed!

Run PubSub With Monitor Test
    [Arguments]    ${layer_tag}    ${mode}
    ${IMAGE}=        Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${TOPIC}=        Set Variable    test_topic
    ${PUB_NAME}=     Set Variable    pub_${layer_tag}
    ${SUB_NAME}=     Set Variable    sub_${layer_tag}
    ${MONITOR_NAME}=     Set Variable    mon_${layer_tag}

    Log To Console    \nStarting Pub/Sub + Monitor containers with ${layer_tag}

    Create Docker Network    ${NETWORK}

    Start Container    ${MONITOR_NAME}    ${IMAGE}    monitor    ${layer_tag}    ${SUB_NAME}    network=${NETWORK}
    Sleep    1s
    Start Container    ${SUB_NAME}    ${IMAGE}    subscriber    ${layer_tag}    ${TOPIC}    network=${NETWORK}
    Sleep    1s
    Start Container    ${PUB_NAME}    ${IMAGE}    publisher     ${layer_tag}    ${TOPIC}    network=${NETWORK}

    Wait For Container Exit    ${SUB_NAME}
    Wait For Container Exit    ${PUB_NAME}
    Wait For Container Exit    ${MONITOR_NAME}

    ${sub_logs}=        Get Container Logs    ${SUB_NAME}
    Log To Console      \n[SUBSCRIBER CONTAINER OUTPUT]\n${sub_logs}

    ${pub_logs}=        Get Container Logs    ${PUB_NAME}
    Log To Console      \n[PUBLISHER CONTAINER OUTPUT]\n${pub_logs}

    ${monitor_logs}=    Get Container Logs    ${MONITOR_NAME}
    Log To Console      \n[MONITOR CONTAINER OUTPUT]\n${monitor_logs}

    Log Test Summary    Pub/Sub + Monitor ${layer_tag}    ${True}\n

    Stop Container    ${SUB_NAME}
    Stop Container    ${PUB_NAME}
    Stop Container    ${MONITOR_NAME}

Run Local All In One Test
    [Arguments]    ${layer_tag}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${CONTAINER}=  Set Variable    local_all_${layer_tag}

    Log To Console    \nStarting local all-in-one container with ${layer_tag}

    Start Container    ${CONTAINER}    ${IMAGE}    local_all    ${layer_tag}

    Wait For Container Exit    ${CONTAINER}

    ${logs}=    Get Container Logs    ${CONTAINER}
    Log To Console    [LOCAL ALL-IN-ONE OUTPUT]\n${logs}

    Log Test Summary    Local SHM ${layer_tag}    ${True}
    Stop Container    ${CONTAINER}
