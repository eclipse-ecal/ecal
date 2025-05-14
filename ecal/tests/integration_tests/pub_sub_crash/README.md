# eCAL Pub/Sub Crash Integration Test

This test checks the communication between a **Publisher**, **Subscriber**, and **Monitoring** process in an eCAL environment. It is useful for testing crash scenarios and process visibility using Docker and Robot Framework.

## 🔍 Overview

* The **Publisher** sends messages and then crashes on purpose.
* The **Subscriber** waits for messages and logs the value.
* The **Monitor** checks if all processes are running and prints their names.
* All parts run in Docker containers and use eCAL with different transport layers.

## Project Structure

```
pub_sub_crash/
├── docker/
│   └── Dockerfile
├── scripts/
│   └── entrypoint.sh
├── src/
│   ├── crash_publisher.cpp
│   ├── crash_subscriber.cpp
│   └── monitoring.cpp
├── robottests/
│   └── pub_sub_crash.robot
└── build_images.sh
```

## Active Test Case

The `.robot` test file currently runs only the **`network_udp`** test:

```robot
*** Test Cases ***
Pub/Sub Network UDP
    Run PubSub With Monitor Test    network_udp    network
```

### Enable Other Layers

Other tests are already prepared but commented out:

```robot
# Pub/Sub Network TCP
#     Run PubSub With Monitor Test    network_tcp    network

# Pub/Sub Local SHM (all-in-one)
#     Run Local All-in-One Test    local_shm
```

You can enable them by removing the `#` symbol.

## Build Docker Image

```bash
./build_images.sh pub_sub_crash
```

This builds an image like `pub_sub_crash_network_udp`.

## 🚀 Run the Test

```bash
robot robottests/pub_sub_crash.robot
```

You will see the logs of all three containers (Publisher, Subscriber, Monitor) at the end.

## Supported Modes

* `network_udp` (default)
* `network_tcp` (can be enabled)
* `local_shm` (runs all in one container)

## Notes

* All containers use the shared Docker network `ecal_test_net`.
* The Monitor runs for 20 seconds and reports all running process names.
* The Subscriber saves the received value in a file: `/app/received.log`

## Purpose

This test ensures:

* eCAL processes can communicate with each other.
* Process crashes are detected.
* Different transport layers can be tested.
