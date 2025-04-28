*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../lib/MyDockerLibrary.py
Library           ${CURDIR}/../lib/GlobalPathsLibrary.py
Suite Setup       Build Docker Images

*** Variables ***
${NETWORK}        ${EMPTY}
${CONFIGS_DIR}    ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}

*** Test Cases ***
Local SHM Communication
    Run PubSub Test    local_shm    ecal_local_shm.yaml    local

Local UDP Communication
    Run PubSub Test    local_udp    ecal_local_udp.yaml    local

Local TCP Communication
    Run PubSub Test    local_tcp    ecal_local_tcp.yaml    local

Network UDP Communication
    Run PubSub Test    network_udp    ecal_network_udp.yaml    network

Network TCP Communication
    Run PubSub Test    network_tcp    ecal_network_tcp.yaml    network

*** Keywords ***
Build Docker Images
    ${cfg}=    Get Configs Dir
    ${build}=   Get Build Script Path
    ${net}=     Get Network Name
    Set Suite Variable    ${CONFIGS_DIR}    ${cfg}
    Set Suite Variable    ${BUILD_SCRIPT}   ${build}
    Set Suite Variable    ${NETWORK}        ${net}

    Log To Console    [SETUP] Checking and building Docker images if needed...
    ${result}=    Run Process    ${BUILD_SCRIPT}
    Should Be Equal As Integers    ${result.rc}    0    Failed to build Docker images!

Run PubSub Test
    [Arguments]    ${layer_tag}    ${config_file}    ${mode}
    ${IMAGE}=    Set Variable    ecal_test_pubsub_${layer_tag}
    ${CONFIG_PATH}=    Set Variable    ${CONFIGS_DIR}/${config_file}

    Log To Console    Running ${mode.capitalize()} Test: ${layer_tag}
    Overwrite ECAL Config    ${IMAGE}    ${CONFIG_PATH}

    Run Keyword If    '${mode}' == 'network'
    ...    Create Docker Network    ${NETWORK}

    Run Keyword If    '${mode}' == 'local'
    ...    Run Local Container    ${IMAGE}    ${layer_tag}
    ...  ELSE
    ...    Run Network Containers    ${IMAGE}    ${layer_tag}

Run Local Container
    [Arguments]    ${image}    ${layer_tag}
    ${NAME}=    Set Variable    pubsub_${layer_tag}
    Start Container    ${NAME}    ${image}    both    ${NETWORK}
    ${exit_code}=    Wait For Container Exit    ${NAME}
    Should Be Equal As Integers    ${exit_code}    0    Communication failed!
    Log Test Summary    PubSub ${layer_tag}    ${True}
    Stop Container    ${NAME}

Run Network Containers
    [Arguments]    ${image}    ${layer_tag}
    ${SUB_NAME}=    Set Variable    sub_${layer_tag}
    ${PUB_NAME}=    Set Variable    pub_${layer_tag}
    Start Container    ${SUB_NAME}    ${image}    subscriber    ${NETWORK}
    Start Container    ${PUB_NAME}    ${image}    publisher    ${NETWORK}
    ${exit_code}=    Wait For Container Exit    ${SUB_NAME}
    Should Be Equal As Integers    ${exit_code}    0    Communication failed!
    Log Test Summary    PubSub ${layer_tag}    ${True}
    Stop Container    ${SUB_NAME}
    Stop Container    ${PUB_NAME}
