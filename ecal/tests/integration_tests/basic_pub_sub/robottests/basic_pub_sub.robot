*** Comments ***
This test checks basic communication between one publisher and one subscriber.

It verifies that:
- The publisher sends messages on a single topic.
- The subscriber receives the messages correctly.
- Communication works in all 5 eCAL modes: local_shm, local_udp, local_tcp, network_udp, network_tcp.

Success criteria:
- The subscriber receives a message.
- The subscriber exits with code 0 if successful.

*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     basic_pub_sub

*** Test Cases ***
Basic Pub/Sub Local SHM
    [Tags]    basic_pub_sub_local_shm
    Run Local Pub Sub Test    local_shm

Basic Pub/Sub Local UDP
    [Tags]    basic_pub_sub_local_udp
    Run Local Pub Sub Test    local_udp

Basic Pub/Sub Local TCP
    [Tags]    basic_pub_sub_local_tcp
    Run Local Pub Sub Test    local_tcp

Basic Pub/Sub Network UDP
    [Tags]    basic_pub_sub_network_udp
    Run Network Pub Sub Test    network_udp    network

Basic Pub/Sub Network TCP
    [Tags]    basic_pub_sub_network_tcp
    Run Network Pub Sub Test    network_tcp    network

*** Keywords ***
Init Test Context
    Set Test Context    basic_pub_sub    basic_pub_sub
    ${build}=    Get Build Script Path
    ${net}=      Get Network Name
    ${args}=     Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}    ${build}
    Set Suite Variable    ${NETWORK}         ${net}

    Log To Console    [SETUP] Building Docker image...
    ${result}=    Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0    Failed to build Docker image!

    Create Docker Network    ${NETWORK}
    Sleep    3s

Run Local Pub Sub Test
    [Arguments]    ${layer_tag}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${CONTAINER}=  Set Variable    basic_local_${layer_tag}

    Log To Console    \n[INFO] Running local pub/sub test in mode: ${layer_tag}

    Start Container    ${CONTAINER}    ${IMAGE}    local    ${layer_tag}    test_topic    ${CONTAINER}
    
    ${exit_code}=    Wait For Container Exit    ${CONTAINER}
    Should Be Equal As Integers    ${exit_code}    0    Local communication test failed!

    ${logs}=    Get Container Logs    ${CONTAINER}
    Log To Console    \n[CONTAINER LOG: LOCAL PUB+SUB]\n${logs}

    Log Test Summary    Basic Pub/Sub Local ${layer_tag}    ${True}
    Stop Container      ${CONTAINER}
    Sleep               1s

Run Network Pub Sub Test
    [Arguments]    ${layer_tag}    ${mode}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${TOPIC}=      Set Variable    topic_${layer_tag}
    ${SUB_NAME}=   Set Variable    sub_${layer_tag}
    ${PUB_NAME}=   Set Variable    pub_${layer_tag}

    Log To Console    \n[INFO] Running network pub/sub test in mode: ${layer_tag}

    Start Container    ${SUB_NAME}    ${IMAGE}    subscriber    ${layer_tag}    ${TOPIC}    ${SUB_NAME}    network=${NETWORK}
    Start Container    ${PUB_NAME}    ${IMAGE}    publisher     ${layer_tag}    ${TOPIC}    ${PUB_NAME}    network=${NETWORK}

    ${exit_code}=    Wait For Container Exit    ${SUB_NAME}
    Should Be Equal As Integers    ${exit_code}    0    Subscriber failed in ${layer_tag}!

    ${log_sub}=    Get Container Logs    ${SUB_NAME}
    ${log_pub}=    Get Container Logs    ${PUB_NAME}

    Log To Console    \n[CONTAINER LOG: SUBSCRIBER]\n${log_sub}
    Log To Console    \n[CONTAINER LOG: PUBLISHER]\n${log_pub}

    Log Test Summary    Basic Pub/Sub Network ${layer_tag}    ${True}

    Stop Container    ${SUB_NAME}
    Stop Container    ${PUB_NAME}
    Sleep            1s
