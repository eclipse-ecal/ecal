*** Comments ***
 Test Objective:
   Ensure that communication to the subscriber continues even if one publisher crashes.

 Test Scenario:
   - Two publishers are started: 
       1. crash_publisher: sends a few messages (value 42) and then crashes.
       2. test_publisher: continuously sends messages (value 43).
   - One subscriber receives data from both publishers.
   - A monitoring process tracks process appearance.

 Success Criteria:
   - Subscriber receives enough messages with value 43 after crash_publisher crashes.
   - Subscriber exits with code 0, indicating continued communication.
   - All containers terminate cleanly.

*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     pub_crash

*** Test Cases ***
Communication Continues After Publisher Crash (UDP)
    [Tags]    pub_crash_network_udp
    Run Network Communication Test    network_udp    network

Communication Continues After Publisher Crash (TCP)
   [Tags]    pub_crash_network_tcp
   Run Network Communication Test    network_tcp    network

Communication Continues After Publisher Crash (SHM)
    [Tags]    pub_crash_local_shm
    Run Local Communication Test    local_shm

*** Keywords ***
Init Test Context
    Set Test Context    pub_crash    pub_crash
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
    ${CRASH_PUB}=        Set Variable    crash_pub_${layer_tag}
    ${TEST_PUB}=         Set Variable    test_pub_${layer_tag}
    ${SUB_NAME}=         Set Variable    sub_${layer_tag}
    #${MONITOR_NAME}=     Set Variable    mon_${layer_tag}

    Log To Console    \n[INFO] Starting communication continuity test with ${layer_tag}

    #Start Container    ${MONITOR_NAME}    ${IMAGE}    monitor    ${layer_tag}    ${SUB_NAME}    network=${NETWORK}
    Start Container    ${SUB_NAME}    ${IMAGE}    subscriber    ${layer_tag}    ${TOPIC}    network=${NETWORK}
    Start Container    ${CRASH_PUB}   ${IMAGE}    crash_publisher     ${layer_tag}    ${TOPIC}  network=${NETWORK}
    Start Container    ${TEST_PUB}    ${IMAGE}    test_publisher    ${layer_tag}    ${TOPIC}    ${TEST_PUB}    network=${NETWORK}

    Wait For Container Exit    ${SUB_NAME}
    Wait For Container Exit    ${CRASH_PUB}
    Wait For Container Exit    ${TEST_PUB}
    #Wait For Container Exit    ${MONITOR_NAME}

    ${sub_logs}=        Get Container Logs    ${SUB_NAME}
    Log To Console      \n[SUBSCRIBER CONTAINER OUTPUT]\n${sub_logs}

    ${crash_logs}=      Get Container Logs    ${CRASH_PUB}
    Log To Console      \n[CRASH PUBLISHER OUTPUT]\n${crash_logs}

    #${monitor_logs}=    Get Container Logs    ${MONITOR_NAME}
    #Log To Console      \n[MONITOR CONTAINER OUTPUT]\n${monitor_logs}
      
    ${exit_code}=    Wait For Container Exit    ${SUB_NAME}
    Should Be Equal As Integers    ${exit_code}    0    Communication failed!

    Log Test Summary  Network Communication after one Publisher crash ${layer_tag}    ${True}

    Stop Container    ${SUB_NAME}    
    Stop Container    ${CRASH_PUB}
    Stop Container    ${TEST_PUB}
    #Stop Container    ${MONITOR_NAME}
    Sleep    1s

Run Local Communication Test
    [Arguments]    ${layer_tag}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${CONTAINER}=  Set Variable    local_all_${layer_tag}

    Log To Console    \nStarting local test container with ${layer_tag}

    Start Container    ${CONTAINER}    ${IMAGE}    local_all    ${layer_tag}

    ${exit_code}=    Wait For Container Exit    ${CONTAINER}
    Should Be Equal As Integers    ${exit_code}    0    Local test failed!

    ${logs}=    Get Container Logs    ${CONTAINER}
    Log To Console    \n\n[CONTAINER LOG: LOCAL PUB+SUB]\n\n${logs}

    Log Test Summary    Local Communication after one Publisher crash ${layer_tag}    ${True}
    Stop Container    ${CONTAINER}
    Sleep    1s
