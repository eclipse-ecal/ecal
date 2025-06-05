*** Comments ***
 Test Objective:
   Ensure that a second subscriber continues to receive messages even if one subscriber crashes.

 Test Scenario:
   - One publisher (`test_publisher`) sends continuous messages (value 43).
   - Two subscribers are started:
       1. `crash_subscriber`: receives a few messages and then crashes.
       2. `test_subscriber`: receives messages throughout the test duration.
   - A monitoring process tracks all components.

 Success Criteria:
   - `test_subscriber` must receive a number of messages.
   - `test_subscriber` must exit with code 0.
   - The crash of one subscriber does not affect the continued communication of the other.

*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     sub_crash

*** Test Cases ***
Network Communication Continues After Subscriber Crash (UDP)
    [Tags]    sub_crash_network_udp
    Run Network Communication Test   network_udp    network

Network Communication Continues After Publisher Crash (TCP)
   [Tags]    pub_crash_network_tcp
    Run Network Communication Test    network_tcp    network

Local Communication Continues After Subscriber Crash (SHM)
    [Tags]    sub_crash_local_shm
    Run Local Communication Test    local_shm

Local Communication Continues After Subscriber Crash (UDP)
    [Tags]    sub_crash_local_udp
    Run Local Communication Test    local_udp

Local Communication Continues After Subscriber Crash (TCP)
    [Tags]    sub_crash_local_tcp
    Run Local Communication Test    local_tcp

*** Keywords ***
Init Test Context
    Set Test Context    sub_crash    sub_crash
    ${build}=           Get Build Script Path
    ${net}=             Get Network Name
    ${args}=            Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}   ${build}
    Set Suite Variable    ${NETWORK}        ${net}

    Log To Console    [SETUP] Building Docker image...
    ${result}=        Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0    Docker build failed!
    Create Docker Network    ${NETWORK}
    Sleep    3s

Run Network Communication Test
    [Arguments]          ${layer_tag}    ${mode}
    ${IMAGE}=            Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${TOPIC}=            Set Variable    test_topic
    ${CRASH_SUB}=        Set Variable    crash_sub_${layer_tag}
    ${TEST_SUB}=         Set Variable    test_sub_${layer_tag}
    ${TEST_PUB}=         Set Variable    test_pub_${layer_tag}
    #${MONITOR_NAME}=     Set Variable    mon_${layer_tag}

    Log To Console    \n[INFO] Starting subscriber crash continuity test with ${layer_tag}

    Create Docker Network    ${NETWORK}

    #Start Container    ${MONITOR_NAME}    ${IMAGE}    monitor    ${layer_tag}    ${CRASH_SUB}    network=${NETWORK}
    Start Container    ${CRASH_SUB}      ${IMAGE}    crash_subscriber    ${layer_tag}    ${TOPIC}    ${CRASH_SUB}    network=${NETWORK}
    Start Container    ${TEST_SUB}       ${IMAGE}    test_subscriber     ${layer_tag}    ${TOPIC}    ${TEST_SUB}     network=${NETWORK}
    Start Container    ${TEST_PUB}       ${IMAGE}    test_publisher      ${layer_tag}    ${TOPIC}    ${TEST_PUB}     network=${NETWORK}

    Wait For Container Exit    ${CRASH_SUB}
    Wait For Container Exit    ${TEST_SUB}
    Wait For Container Exit    ${TEST_PUB}
    #Wait For Container Exit    ${MONITOR_NAME}

    ${test_sub_logs}=    Get Container Logs    ${TEST_SUB}
    Log To Console        \n[SUBSCRIBER CONTAINER OUTPUT]\n${test_sub_logs}

    ${crash_sub_logs}=    Get Container Logs    ${CRASH_SUB}
    Log To Console        \n[CRASH SUBSCRIBER CONTAINER OUTPUT]\n${crash_sub_logs}

    #${monitor_logs}=      Get Container Logs    ${MONITOR_NAME}
    #Log To Console        \n[MONITOR CONTAINER OUTPUT]\n${monitor_logs}

    ${exit_code}=    Wait For Container Exit    ${TEST_SUB}
    Should Be Equal As Integers    ${exit_code}    0    Communication failed!

    Log Test Summary  Network Communication after one Subscriber crash ${layer_tag}    ${True}

    Stop Container    ${CRASH_SUB}
    Stop Container    ${TEST_SUB}
    Stop Container    ${TEST_PUB}
    #Stop Container    ${MONITOR_NAME}

Run Local Communication Test
    [Arguments]    ${layer_tag}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${CONTAINER}=  Set Variable    local_all_${layer_tag}

    Log To Console    \n[INFO] Starting local test container with ${layer_tag}
    Create Docker Network    ${NETWORK}
    
    Start Container    ${CONTAINER}    ${IMAGE}    local_all    ${layer_tag}

    ${exit_code}=    Wait For Container Exit    ${CONTAINER}
    Should Be Equal As Integers    ${exit_code}    0   Local test failed!

    ${logs}=    Get Container Logs    ${CONTAINER}
    Log To Console    \n\n[LOCAL TEST CONTAINER OUTPUT]\n\n${logs}

    Log Test Summary    Local Communication after one Subscriber crash ${layer_tag}    ${True}
    Stop Container    ${CONTAINER}
