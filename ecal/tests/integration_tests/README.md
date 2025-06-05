# Integration Tests for eCAL Middleware

This directory provides a complete set of **integration tests** for the **eCAL (enhanced Communication Abstraction Layer)** middleware. The tests are designed to validate the behavior of publisher-subscriber communication under various scenarios, including fault handling and network reliability.

---

## Overview

These tests simulate real-world communication setups using **Docker** and are executed via **Robot Framework**. Each test focuses on specific IPC (inter-process communication) aspects, such as crash resilience, message delivery consistency, and transport mode reliability.

---

## Repository Layout

```text
ecal/tests/integration_tests
├── basic_pub_sub/          # Simple 1:1 communication test
├── multi_pub_sub/          # N:N publisher-subscriber test
├── sub_send_crash/         # Subscriber crashes during send
├── pub_crash/              # Publisher crashes mid-stream
├── sub_crash/              # Subscriber crashes completely
├── network_crash/          # Simulates network failure (UDP vs UDP)
├── message_validation/     # Checks message payload integrity (e.g., Protobuf)
├── cfg/                    # Configuration files like ecal.yaml
├── lib/                    # Shared Robot Framework libraries (Python) and helper functions
├── requirements.txt        # Dependencies
├── CMakeLists.txt          # Builds test sources
└── README.md               # This file
```

---

## Requirements

* Docker (including permission to run containers)
* Python 3.10+
* Robot Framework and dependencies:

```bash
pip install -r requirements.txt
```

* eCAL base image built locally:

```bash
docker build -t ecal_base ./docker/Dockerfile.ecal_base
```

---

## How to Run Tests

Each test is self-contained and can be executed as follows:

```bash
robot <test_directory>/robottests/<test_case>.robot
```

Example:

```bash
robot network_crash/robottests/network_crash.robot
```

---

## Test Coverage

| Folder                | Scenario Description                                |
| --------------------- | --------------------------------------------------- |
| `basic_pub_sub/`      | One publisher, one subscriber (basic functionality) |
| `multi_pub_sub/`      | Multiple publishers/subscribers on same topic       |
| `sub_send_crash/`     | Subscriber crashes during data reception            |
| `sub_crash/`          | Subscriber process crashes completely               |
| `pub_crash/`          | Publisher process crashes during message stream     |
| `network_crash/`      | Network-based publisher is disconnected             |
| `message_validation/` | Payload and Protobuf message validation             |

Each test supports different **eCAL transport modes**:

* `local_shm`
* `local_udp`
* `local_tcp`
* `network_udp`
* `network_tcp`

---

## Common Utilities

The `lib/` folder contains shared Robot Framework libraries:

* `MyDockerLibrary.py`: Starts, stops, and monitors Docker containers
* `GlobalPathsLibrary.py`: Manages test folder and image/tag resolution
* `ecal_config_helper.cpp/h`: Helper functions for configurations of the binarys

---

## Creating a New Test

To add a new test:

1. Create a new folder (e.g. `advanced_filtering/`)
2. Add C++ source files in `src/`
3. Create test logic in `robottests/<test_name>.robot`
4. Add `build_images.sh` and `entrypoint.sh` in `scripts/`
5. Create a `docker/Dockerfile`
6. Update this README if relevant

---

## CI Integration

All tests are designed to run in CI/CD pipelines:

```bash
robot --outputdir results <path-to-robot>
```

Artifacts:

* `output.xml`: Test results
* `log.html`, `report.html`: Detailed reports

---

## Author

**Emircan Tutar**
