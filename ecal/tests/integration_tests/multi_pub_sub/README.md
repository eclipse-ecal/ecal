# Multi Publisher/Subscriber Test (multi_pub_sub)

## Description

This test verifies that communication between multiple publishers and subscribers works correctly over the eCAL middleware.

## Scenario

- Two publishers send messages on the same topic:
  - One sends the value `42`.
  - The other sends the value `43`.
- Two subscribers listen on that topic and check if they receive messages from both publishers.

## Test Objective

Confirm that both subscribers can receive messages from both publishers.

Test this in **all 5 eCAL modes**:
- `local_shm`
- `local_udp`
- `local_tcp`
- `network_udp`
- `network_tcp`

## Success Criteria

- Each subscriber prints a summary showing how many `42` and `43` messages it received.
- If both values are received at least once, the subscriber exits with code `0`.
- The `.robot` test passes only if both subscribers exit successfully.

## Structure

- `multi_publisher.cpp`: Sends value `42`.
- `multi_publisher2.cpp`: Sends value `43`.
- `multi_subscriber.cpp` and `multi_subscriber2.cpp`: Receive and verify both values.
- `multi_pub_sub.robot`: Robot Framework test file that runs the scenario in all modes.
