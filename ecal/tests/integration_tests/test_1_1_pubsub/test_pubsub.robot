*** Settings ***
Library           OperatingSystem
Library           Process
Library           ../lib/MyDockerLibrary.py

*** Variables ***
${NETWORK}        ecal_test_net
${CONFIGS_DIR}    cfg

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
Run PubSub Test
    [Arguments]    ${layer_tag}    ${config_file}    ${mode}
    ${IMAGE}=    Set Variable    ecal_test_pubsub_${layer_tag}
    ${CONFIG_PATH}=    Set Variable    ${CONFIGS_DIR}/${config_file}

    Log To Console    Running ${mode.capitalize()} Test: ${layer_tag}

    Overwrite ECAL Config     ${IMAGE}    ${CONFIG_PATH}
    Create Docker Network     ${NETWORK}

    Run Keyword If    '${mode}' == 'local'
    ...    Run Local Container    ${IMAGE}    ${layer_tag}
    ...  ELSE
    ...    Run Network Containers    ${IMAGE}    ${layer_tag}

Run Local Container
    [Arguments]    ${image}    ${layer_tag}
    ${NAME}=    Set Variable    pubsub_${layer_tag}
    Start Container    ${NAME}    ${image}    both    ${NETWORK}
    Sleep    20s
    ${log}=    Get Container Logs    ${NAME}
    Should Contain    ${log}    [Subscriber] Received value: 42
    Log Test Summary    PubSub ${layer_tag}    ${True}
    Stop Container    ${NAME}

Run Network Containers
    [Arguments]    ${image}    ${layer_tag}
    ${SUB_NAME}=    Set Variable    sub_${layer_tag}
    ${PUB_NAME}=    Set Variable    pub_${layer_tag}
    Start Container    ${SUB_NAME}    ${image}    subscriber    ${NETWORK}
    Start Container    ${PUB_NAME}    ${image}    publisher    ${NETWORK}
    Sleep    8s
    ${log}=    Get Container Logs    ${SUB_NAME}
    Should Contain    ${log}    [Subscriber] Received value: 42
    Log Test Summary    PubSub ${layer_tag}    ${True}
    Stop Container    ${SUB_NAME}
    Stop Container    ${PUB_NAME}
