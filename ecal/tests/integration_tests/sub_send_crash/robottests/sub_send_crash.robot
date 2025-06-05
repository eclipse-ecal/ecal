*** Comments ***
Test Objective:
This test verifies that if one subscriber crashes during the reception of a large message,
other subscribers or the publisher remain unaffected and stable.

Test Scenario:
- A `large_publisher` sends very large messages (~50MB+).
- One `crash_subscriber` is designed to crash during message reception (e.g., via simulated failure during callback).
- One `test_subscriber` continues running and must successfully receive all messages.
- In an additional test case, a variant with `zero_copy_mode` enabled is used to evaluate the performance and fault tolerance in a high-throughput configuration.

Success Criteria:
- The `test_subscriber` must receive messages without crashing and exit with code 0.
- The `crash_subscriber` should crash as intended.
- The `large_publisher` must remain operational and not be blocked or destabilized.
- In SHM mode with `zero_copy`, the system should handle the shared memory access without affecting other subscribers or the pubisher.

Notes:
- Zero-Copy mode is only applicable to Shared Memory (SHM) transport.
- The Local UDP test is skipped for large messages because of message size limitations.
- This test failes because of SHM with zero copy mode, which is a known issue in the current implementation. 

*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     sub_send_crash

*** Test Cases ***
Crash During Reception (SHM)
    [Tags]    crash_receive_local_shm
    Run Local Crash Receive Test    local_shm

Crash with Zero Copy Enabled in Reception (SHM)
    [Tags]    zero_copy_crash_receive_local_shm
    Run with Zero Copy Enabled Local Crash Receive Test    local_shm

#Crash During Reception (UDP)
#    [Tags]    crash_receive_local_udp
#    Run Local Crash Receive Test    local_udp
# Is not relevant for UDP Local mode, because of message size limitations

Crash During Reception (TCP)
    [Tags]    crash_receive_local_tcp
    Run Local Crash Receive Test    local_tcp

Crash During Reception (Network UDP)
    [Tags]    crash_receive_network_udp
    Run Network Crash Receive Test    network_udp    network

Crash During Reception (Network TCP)
    [Tags]    crash_receive_network_tcp
    Run Network Crash Receive Test    network_tcp    network

*** Keywords ***
Init Test Context
    Set Test Context    sub_send_crash    sub_send_crash
    ${build}=    Get Build Script Path
    ${net}=      Get Network Name
    ${args}=     Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}    ${build}
    Set Suite Variable    ${NETWORK}         ${net}

    Log To Console    [SETUP] Building image...
    ${result}=    Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0
    Create Docker Network    ${NETWORK}
    Sleep    3s

Run Network Crash Receive Test
    [Arguments]    ${layer_tag}    ${mode}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${TOPIC}=      Set Variable    test_topic

    ${SUB_OK}=     Set Variable    sub_ok_${layer_tag}
    ${SUB_CRASH}=  Set Variable    sub_crash_${layer_tag}
    ${PUBLISHER}=  Set Variable    pub_${layer_tag}

    Log To Console    \n[INFO] Starting crash test in NETWORK mode: ${layer_tag}

    Start Container    ${SUB_OK}     ${IMAGE}    test_subscriber         ${layer_tag}    ${TOPIC}    ${SUB_OK}     network=${NETWORK}
    Start Container    ${SUB_CRASH}  ${IMAGE}    crash_subscriber   ${layer_tag}    ${TOPIC}    ${SUB_CRASH}  network=${NETWORK}
    Start Container    ${PUBLISHER}  ${IMAGE}    large_publisher    ${layer_tag}    ${TOPIC}    ${PUBLISHER}  network=${NETWORK}

    Wait For Container Exit    ${SUB_OK}
    Wait For Container Exit    ${SUB_CRASH}
    Wait For Container Exit    ${PUBLISHER}

    ${log1}=    Get Container Logs    ${SUB_OK}
    ${log2}=    Get Container Logs    ${SUB_CRASH}
    ${log3}=    Get Container Logs    ${PUBLISHER}

    Log To Console    \n[LOG: SUBSCRIBER]\n${log1}
    Log To Console    \n[LOG: CRASH_SUBSCRIBER]\n${log2}
    Log To Console    \n[LOG: PUBLISHER]\n${log3}

    ${exit1}=    Wait For Container Exit    ${SUB_OK}
    Should Be Equal As Integers    ${exit1}    0    Stable subscriber failed unexpectedly!

    Log Test Summary    Network Crash Receive Test ${layer_tag}    ${True}

    Stop Container    ${SUB_OK}
    Stop Container    ${SUB_CRASH}
    Stop Container    ${PUBLISHER}

Run Local Crash Receive Test
    [Arguments]    ${layer_tag}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${CONTAINER}=  Set Variable    local_all_${layer_tag}

    Log To Console    \n[INFO] Starting crash test in LOCAL mode: ${layer_tag}

    Start Container    ${CONTAINER}    ${IMAGE}    local_all    ${layer_tag}    test_topic    ${CONTAINER}    network=${NETWORK}
    Sleep    35s
    ${logs}=    Get Container Logs    ${CONTAINER}
    Log To Console    \n[LOCAL TEST CONTAINER OUTPUT]\n${logs}

    ${exit_code}=    Wait For Container Exit    ${CONTAINER}
    Should Be Equal As Integers    ${exit_code}    0    Local container failed!

    ${logs}=    Get Container Logs    ${CONTAINER}
    #Log To Console    \n[LOCAL TEST CONTAINER OUTPUT]\n${logs}

    Log Test Summary    Local Crash Receive Test ${layer_tag}    ${True}

    Stop Container    ${CONTAINER}

Run with Zero Copy Enabled Local Crash Receive Test

    [Arguments]    ${layer_tag}
    ${IMAGE}=      Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${CONTAINER}=  Set Variable    zero_copy_local_all_${layer_tag}

    Log To Console    \n[INFO] Starting crash test with zero copy enabled in LOCAL mode: ${layer_tag}

    Start Container    ${CONTAINER}    ${IMAGE}    zero_copy_local_all    ${layer_tag}    test_topic    ${CONTAINER}    network=${NETWORK}
    Sleep    35s 
    ${logs}=    Get Container Logs    ${CONTAINER}
    Log To Console    \n[LOCAL SHM WITH ZERO COPY TEST CONTAINER OUTPUT]\n${logs}

    ${exit_code}=    Wait For Container Exit    ${CONTAINER}
    Should Be Equal As Integers    ${exit_code}    0    Local container with zero copy shm failed!

    ${logs}=    Get Container Logs    ${CONTAINER}
    #Log To Console    \n[LOCAL TEST CONTAINER OUTPUT]\n${logs}

    Log Test Summary    Local Crash with zero copy enabled Test ${layer_tag}    ${True}

    Stop Container    ${CONTAINER}