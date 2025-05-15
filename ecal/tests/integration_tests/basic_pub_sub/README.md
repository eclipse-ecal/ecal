# Basic Publisher/Subscriber Test (basic_pub_sub)

## Description

This test verifies the basic functionality of the eCAL communication system using one publisher and one subscriber.

## Scenario

- One publisher sends simple binary messages (e.g., a buffer filled with `42`).
- One subscriber listens to the same topic and checks if it receives the messages.

## Test Objective

Confirm that a subscriber receives messages from a single publisher.

Test this in **all 5 eCAL modes**:
- `local_shm`
- `local_udp`
- `local_tcp`
- `network_udp`
- `network_tcp`

## Success Criteria

- The subscriber prints how many messages it received.
- If at least one message was received, the subscriber exits with code `0`.
- The `.robot` test passes if the subscriber exits successfully.

## Structure

- `publisher.cpp`: Sends a fixed number of messages on the topic.
- `subscriber.cpp`: Receives the messages and validates reception.
- `basic_pub_sub.robot`: Robot Framework test that runs this scenario in all transport modes.

## Notes

- This test is ideal for verifying core communication between components before moving to more complex tests.
- For real-world testing, this basic pattern helps detect network configuration or transport layer issues.
