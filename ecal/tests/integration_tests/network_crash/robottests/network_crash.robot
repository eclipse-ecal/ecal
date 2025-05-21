*** Comments ***
Test Objective:
    Verify that a local UDP-based communication path remains functional even if a separate network UDP publisher is interrupted—
    simulating a real-world scenario such as a cable unplug or network failure.

Test Context:
    This test validates the robustness of the eCAL UDP transport layer under fault conditions by checking that 
    local communication is not affected by failures in network-based communication.

Test Scenario:
    1. A local UDP publisher and subscriber run within the **same container** using eCAL's local mode.
       - The publisher sends payloads with value 42.
       - The subscriber listens on the same topic and tracks received message values.
       
    2. A second publisher (network UDP publisher) runs in a **separate container** using network mode.
       - It sends payloads with value 43.
       - This simulates a system component publishing over the network.

    3. Both publishers operate concurrently for 7 seconds to ensure simultaneous communication.

    4. After 7 seconds, the test **disconnects the network UDP publisher from the Docker network**, simulating a network cable being unplugged.

    5. The subscriber continues running and should:
       - Stop receiving value 43 (as the network publisher is gone).
       - Still receive value 42 from the local publisher.

Success Criteria:
    - The subscriber container exits cleanly (exit code 0).
    - The subscriber reports all messages with value 42 received **after** the network publisher is disconnected.
    - At least two messages with value 43 should appear before disconnection; this is required for pass.

Note:
    This test focuses on **functional separation between local and network communication** in eCAL.
    It is useful to validate system reliability when components fail independently.

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
Local UDP survives Network UDP Publisher crash
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

    Log To Console    \n[INFO] Starting LOCAL UDP PUB+SUB in one container and NETWORK UDP publisher separately...

    Start Container    ${LOCAL_ALL}    ${IMAGE}    udp_local_all     ${TOPIC}    ${LOCAL_ALL}    network=${NETWORK}
    Start Container    ${NET_PUB}      ${IMAGE}    udp_network_pub   ${TOPIC}    ${NET_PUB}      network=${NETWORK}

    Log To Console    \n[INFO] Letting both publishers run in parallel for 7s...
    Sleep    7s

    Log To Console    \n[SIMULATION] Stopping Network UDP Publisher...
    ${log_net}=    Get Container Logs    ${NET_PUB}
    Log To Console    \n[LOG: NETWORK UDP PUB CONTAINER]\n${log_net}
    # Stop Container    ${NET_PUB}
    Disconnect Container From Network    ${NET_PUB}    ${NETWORK}

    Wait For Container Exit    ${LOCAL_ALL}
    ${log_local}=    Get Container Logs    ${LOCAL_ALL}
    Log To Console    \n[LOG: LOCAL UDP PUB+SUB CONTAINER]\n${log_local}

    ${exit_code}=    Wait For Container Exit    ${LOCAL_ALL}
    Should Be Equal As Integers    ${exit_code}    0    LOCAL PUB+SUB container failed unexpectedly!
    Log Test Summary    Network Crash Test (UDP Local survives)    ${True}
