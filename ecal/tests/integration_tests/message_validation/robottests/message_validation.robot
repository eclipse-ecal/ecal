*** Settings ***
Library           OperatingSystem
Library           Process
Library           ${CURDIR}/../../lib/MyDockerLibrary.py
Library           ${CURDIR}/../../lib/GlobalPathsLibrary.py
Suite Setup       Init Test Context

*** Variables ***
${NETWORK}        ${EMPTY}
${BUILD_SCRIPT}   ${EMPTY}
${BASE_IMAGE}     message_validation

*** Test Cases ***
Valid Protobuf Transmission
    [Tags]    message_validation_valid
    Set Test Context    message_validation    message_validation
    Run MessageValidation Test    local_shm    local    --payload="Hello"

Malformed Protobuf Transmission
    [Tags]    message_validation_malformed
    Set Test Context    message_validation    message_validation
    Run MessageValidation Test    local_shm    local    --malformed

*** Keywords ***
Init Test Context
    Set Test Context    message_validation    message_validation
    ${build}=    Get Build Script Path
    ${net}=      Get Network Name
    ${args}=     Get Build Script Args
    Set Suite Variable    ${BUILD_SCRIPT}   ${build}
    Set Suite Variable    ${NETWORK}        ${net}

    Log To Console    [SETUP] Building Docker images if not already present...
    ${result}=        Run Process    ${BUILD_SCRIPT}    @{args}
    Should Be Equal As Integers    ${result.rc}    0    Docker build failed!

Run MessageValidation Test
    [Arguments]    ${layer_tag}    ${mode}    ${extra_args}
    ${IMAGE}=    Set Variable    ${BASE_IMAGE}_${layer_tag}
    ${TOPIC}=    Set Variable    topic_${layer_tag}
    ${PUB_NAME}=    Set Variable    msg_validation_pub_${layer_tag}
    ${SUB_NAME}=    Set Variable    msg_validation_sub_${layer_tag}

    Create Docker Network    ${NETWORK}

    # Start Subscriber
    Start Container    ${SUB_NAME}    ${IMAGE}    subscriber    ${layer_tag}    ${TOPIC}    ${SUB_NAME}    ${extra_args}    network=${NETWORK}
    
    # Start Publisher
    Start Container    ${PUB_NAME}    ${IMAGE}    publisher    ${layer_tag}    ${TOPIC}    ${PUB_NAME}    ${extra_args}    network=${NETWORK}
    
    # Wait for Subscriber to finish
    ${exit_code}=    Wait For Container Exit    ${SUB_NAME}
    Should Be Equal As Integers    ${exit_code}    0    Communication failed!

    # Stop Publisher and Subscriber
    Stop Container    ${PUB_NAME}
    Stop Container    ${SUB_NAME}
