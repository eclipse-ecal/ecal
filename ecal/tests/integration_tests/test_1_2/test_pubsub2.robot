*** Settings ***
Library           OperatingSystem
Library           Process
Library           ../lib/MyDockerLibrary.py

*** Variables ***
${IMAGE}          ecal_test_pubsub
${NETWORK}        ecal_test_net
${SUB_NAME}       sub_node
${PUB_NAME}       pub_node

*** Test Cases ***
Test Publisher To Subscriber Communication
    Log To Console    Running Integration Test 1.1 – Publisher to Subscriber Communication
    [Setup]           Setup Docker Network And Start
    Sleep             8s
    ${log}=           Run Process    docker    exec    ${SUB_NAME}    cat    /app/received.log    shell=True
    Should Contain    ${log.stdout}    42
    Log Test Summary  Publisher to Subscriber Communication    ${True}
    [Teardown]        Stop Both Containers

*** Keywords ***
Setup Docker Network And Start
    Create Docker Network    ${NETWORK}
    Start Container    ${SUB_NAME}    ${IMAGE}    subscriber    ${NETWORK}
    Start Container    ${PUB_NAME}    ${IMAGE}    publisher    ${NETWORK}

Stop Both Containers
    Stop Container    ${SUB_NAME}
    Stop Container    ${PUB_NAME}
