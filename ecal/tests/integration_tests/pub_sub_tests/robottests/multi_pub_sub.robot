*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     multi_pub_sub

*** Test Cases ***

# --- Case 1: One Publisher → Multiple Subscribers ---

Local SHM: 1 Pub to N Subs
    [Tags]    multi_PubSub_local_shm_1pub_Nsubs
    Run MultiPubSub Test    local_shm    local    1    3

Local UDP: 1 Pub to N Subs
    [Tags]    multi_PubSub_local_udp_1pub_Nsubs
    Run MultiPubSub Test    local_udp    local    1    3

Local TCP: 1 Pub to N Subs
    [Tags]    multi_PubSub_local_tcp_1pub_Nsubs
    Run MultiPubSub Test    local_tcp    local    1    3

Network UDP: 1 Pub to N Subs
    [Tags]    multi_PubSub_network_udp_1pub_Nsubs
    Run MultiPubSub Test    network_udp    network    1    3

Network TCP: 1 Pub to N Subs
    [Tags]    multi_PubSub_network_tcp_1pub_Nsubs
    Run MultiPubSub Test    network_tcp    network    1    3

# --- Case 2: Multiple Publishers → One Subscriber ---

Local SHM: N Pubs to 1 Sub
    [Tags]    multi_PubSub_local_shm_Npubs_1sub
    Run MultiPubSub Test    local_shm    local    3    1

Local UDP: N Pubs to 1 Sub
    [Tags]    multi_PubSub_local_udp_Npubs_1sub
    Run MultiPubSub Test    local_udp    local    3    1

Local TCP: N Pubs to 1 Sub
    [Tags]    multi_PubSub_local_tcp_Npubs_1sub
    Run MultiPubSub Test    local_tcp    local    3    1

Network UDP: N Pubs to 1 Sub
    [Tags]    multi_PubSub_network_udp_Npubs_1sub
    Run MultiPubSub Test    network_udp    network    3    1

Network TCP: N Pubs to 1 Sub
    [Tags]    multi_PubSub_network_tcp_Npubs_1sub
    Run MultiPubSub Test    network_tcp    network    3    1

# --- Case 3: Multiple Publishers → Multiple Subscribers ---

Local SHM: N Pubs to N Subs
    [Tags]    multi_PubSub_local_shm_Npubs_Nsubs
    Run MultiPubSub Test    local_shm    local    3    3

Local UDP: N Pubs to N Subs
    [Tags]    multi_PubSub_local_udp_Npubs_Nsubs
    Run MultiPubSub Test    local_udp    local    3    3

Local TCP: N Pubs to N Subs
    [Tags]    multi_PubSub_local_tcp_Npubs_Nsubs
    Run MultiPubSub Test    local_tcp    local    3    3

Network UDP: N Pubs to N Subs
    [Tags]    multi_PubSub_network_udp_Npubs_Nsubs
    Run MultiPubSub Test    network_udp    network    3    3

Network TCP: N Pubs to N Subs
    [Tags]    multi_PubSub_network_tcp_Npubs_Nsubs
    Run MultiPubSub Test    network_tcp    network    3    3

*** Keywords ***
Init Test Context
    Set Test Context    pub_sub_tests    multi_pub_sub
    ${build}=           Get Build Script Path
    ${net}=             Get Network Name
    ${args}=            Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}   ${build}
    Set Suite Variable    ${NETWORK}        ${net}

    Log To Console    [SETUP] Checking and building Docker images...
    ${result}=        Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0    Failed to build Docker images!

Run MultiPubSub Test
    [Arguments]    ${layer_tag}    ${mode}    ${pub_count}    ${sub_count}
    ${IMAGE}=    Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${TOPIC}=    Set Variable    topic_${layer_tag}

    Run Keyword If    '${mode}' == 'network'
    ...    Create Docker Network    ${NETWORK}

    FOR    ${i}    IN RANGE    ${sub_count}
        ${NAME}=    Set Variable    sub_${layer_tag}_${i}
        Start Container    ${NAME}    ${IMAGE}    subscriber    --mode=${layer_tag}    --topic=${TOPIC}    --name=${NAME}    network=${NETWORK}
    END

    FOR    ${j}    IN RANGE    ${pub_count}
        ${NAME}=    Set Variable    pub_${layer_tag}_${j}
        Start Container    ${NAME}    ${IMAGE}    publisher    --mode=${layer_tag}    --topic=${TOPIC}    --name=${NAME}    network=${NETWORK}
    END

    Sleep    5s

    FOR    ${i}    IN RANGE    ${sub_count}
        ${NAME}=    Set Variable    sub_${layer_tag}_${i}
        ${exit_code}=    Wait For Container Exit    ${NAME}
        Should Be Equal As Integers    ${exit_code}    0
        Stop Container    ${NAME}
    END

    FOR    ${j}    IN RANGE    ${pub_count}
        ${NAME}=    Set Variable    pub_${layer_tag}_${j}
        Stop Container    ${NAME}
    END
