*** Comments ***
Test Objective:
    Verify that local UDP-based communication remains functional even if a separate network UDP publisher is disconnected —
    simulating a network failure scenario such as a cable unplug.

Test Context:
    This test validates the robustness of eCAL's UDP transport layer under fault conditions by ensuring that 
    local communication is not affected by network-layer interruptions.

Test Scenario:
    1. A local UDP subscriber and two local UDP publishers run in the **same container** using eCAL's local mode.
       - Publisher 1 sends payloads with value 42 from the beginning.
       - Publisher 2 sends value 44, but only **after the network publisher is disconnected** (simulated delay).

    2. A network UDP publisher runs in a **separate container**, using eCAL network mode and sends value 43.

    3. All publishers run in parallel for a defined period (publisher 2 starts delayed after ~7s).

    4. The test then **disconnects** the network UDP publisher from the Docker network.

Success Criteria:
    - The subscriber receives:
      • Messages with value 42 throughout the test.
      • Messages with value 43 only **before** the disconnection.
      • Messages with value 44 after the network crash (i.e., from local UDP Publisher 2).
    - The subscriber container exits with code 0.
    - The log confirms all expected messages have been received.

Note:
    This test ensures **fault isolation** between local and network layers in eCAL middleware.

*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     network_crash

*** Test Cases ***
Local communication survives Network crash
    [Tags]    network_crash_test
    Run Network Crash Survival Test

*** Keywords ***
Init Test Context
    Set Test Context    network_crash    network_crash
    ${build}=    Get Build Script Path
    ${net}=      Get Network Name
    ${args}=     Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}    ${build}
    Set Suite Variable    ${NETWORK}         ${net}

    Log To Console    [SETUP] Building image...
    ${result}=    Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0    Docker image build failed!
    Create Docker Network    ${NETWORK}
    Sleep    2s

Disconnect Container From Network
    [Arguments]    ${container}    ${network}
    Log To Console    \n[SIMULATION] Disconnecting ${container} from ${network} (simulate network failure)...
    ${result}=    Run Process    docker    network    disconnect    ${network}    ${container}
    Should Be Equal As Integers    ${result.rc}    0    Failed to disconnect ${container} from ${network}
    Sleep    1s

Run Network Crash Survival Test
    ${IMAGE}=       Set Variable    ${BASE_IMAGE}_local_udp
    ${TOPIC}=       Set Variable    test_topic
    ${NET_PUB}=     Set Variable    udp_network_pub
    ${LOCAL_ALL}=   Set Variable    udp_local_all

    Log To Console    \n\n[INFO] Starting "LOCAL UDP (PUB1 + SUB + PUB3)" in one container and "NETWORK UDP (PUB2)" separately...

    Start Container    ${LOCAL_ALL}    ${IMAGE}    udp_local_all     ${TOPIC}    ${LOCAL_ALL}    network=${NETWORK}
    Start Container    ${NET_PUB}      ${IMAGE}    udp_network_pub   ${TOPIC}    ${NET_PUB}      network=${NETWORK}

    Log To Console    \n[INFO] Letting PUB1 and PUB2 run in parallel for 7s...
    Sleep    7s

    Log To Console    \n[SIMULATION] Disconnecting NETWORK UDP Publisher...
    #${log_net}=    Get Container Logs    ${NET_PUB}
    #Log To Console    \n[LOG: NETWORK UDP PUB2 CONTAINER]\n${log_net}
    Disconnect Container From Network    ${NET_PUB}    ${NETWORK}

    Wait For Container Exit    ${LOCAL_ALL}
    ${log_local}=    Get Container Logs    ${LOCAL_ALL}
    Log To Console    \n[LOG: LOCAL UDP PUB + SUB + PUB3 CONTAINER]\n${log_local}

    ${exit_code}=    Wait For Container Exit    ${LOCAL_ALL}
    Should Be Equal As Integers    ${exit_code}    0    LOCAL container (PUB + SUB + PUB3) failed unexpectedly!
    Log Test Summary    Network Crash Test with Local Recovery Publisher    ${True}
