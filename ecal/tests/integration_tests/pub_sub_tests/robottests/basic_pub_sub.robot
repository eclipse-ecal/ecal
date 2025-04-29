*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Build Docker Images

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     basic_pub_sub  

*** Test Cases ***
Local SHM Communication
    Run PubSub Test    local_shm    local

Local UDP Communication
    Run PubSub Test    local_udp    local

Local TCP Communication
    Run PubSub Test    local_tcp    local

Network UDP Communication
    Run PubSub Test    network_udp    network

Network TCP Communication
    Run PubSub Test    network_tcp    network

*** Keywords ***
Build Docker Images
    ${build}=    Get Build Script Path
    ${net}=      Get Network Name
    ${args}=     Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}   ${build}
    Set Suite Variable    ${NETWORK}        ${net}

    Log To Console    [SETUP] Checking and building Docker images if needed...
    ${result}=        Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0    Failed to build Docker images!


Run PubSub Test
    [Arguments]    ${layer_tag}    ${mode}
    ${IMAGE}=    Set Variable    ${BASE_IMAGE}_${layer_tag}

    Log To Console    Running ${mode.capitalize()} Test: ${layer_tag}

    Run Keyword If    '${mode}' == 'network'
    ...    Create Docker Network    ${NETWORK}

    Run Keyword If    '${mode}' == 'local'
    ...    Run Local Container    ${IMAGE}    ${layer_tag}
    ...  ELSE
    ...    Run Network Containers    ${IMAGE}    ${layer_tag}

Run Local Container
    [Arguments]    ${image}    ${layer_tag}
    ${NAME}=    Set Variable    pubsub_${layer_tag}
    Start Container    ${NAME}    ${image}    both    ${layer_tag}    network=${NETWORK}
 
    ${exit_code}=    Wait For Container Exit    ${NAME}
    Should Be Equal As Integers    ${exit_code}    0    Communication failed!
    Log Test Summary    PubSub ${layer_tag}    ${True}
    Stop Container    ${NAME}

Run Network Containers
    [Arguments]    ${image}    ${layer_tag}
    ${SUB_NAME}=    Set Variable    sub_${layer_tag}
    ${PUB_NAME}=    Set Variable    pub_${layer_tag}
    Start Container    ${SUB_NAME}    ${image}    subscriber    ${layer_tag}    network=${NETWORK}
    #Sleep    5s
    Start Container    ${PUB_NAME}    ${image}    publisher    ${layer_tag}    network=${NETWORK}
    #Sleep    10s
    ${exit_code}=    Wait For Container Exit    ${SUB_NAME}
    Should Be Equal As Integers    ${exit_code}    0    Communication failed!
    Log Test Summary    PubSub ${layer_tag}    ${True}
    Stop Container    ${SUB_NAME}
    Stop Container    ${PUB_NAME}

