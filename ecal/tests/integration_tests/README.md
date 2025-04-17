# Integration Tests for eCAL

This directory contains integration-level tests for the eCAL middleware.

Unlike unit tests, integration tests verify communication between multiple components and processes. These tests simulate real-world usage of eCAL, such as message transmission, subscriber behavior, fault handling, and performance under load.

## Purpose

The goal is to ensure that eCAL works correctly in distributed systems with realistic scenarios. These tests are also designed to be reusable and automation-friendly, making them suitable for CI/CD environments.

## Structure

Each test is placed in its own folder:

```
integration_tests/
├── test_1_1_pubsub/         # Test for basic publisher-to-subscriber communication
├── test_1_2_multisub/       # (Example) Test with multiple subscribers
├── test_4_1_fault_injection/
└── ...
```

Each test folder is organized as follows for example:

```
test_1_1_pubsub/
├── src/
│   ├── publisher.cpp            # C++ source for the publisher node
│   └── subscriber.cpp           # C++ source for the subscriber node
├── robot/
│   └── test_pubsub.robot        # Robot Framework test orchestration file
├── docker/ 
│   └── Dockerfile               # Builds the container image with both binaries 
├── scripts/
│   ├── entrypoint.sh            # Entry point for launching publisher or subscriber  
│   └── requirements.txt         # Python dependencies for Robot Framework 
└── README.md
```

## Running Tests

Most tests are designed to be run using [Robot Framework](https://robotframework.org/) and [DockerLibrary](https://pypi.org/project/robotframework-dockerlibrary/). This allows tests to be executed in isolated containers and automated pipelines.

To run a test:
```bash
cd test_1_1_pubsub/robot
robot test_pubsub.robot
```

Make sure Docker is installed and running on your machine.

## Requirements

- Docker
- Python 3
- Robot Framework
- DockerLibrary (`pip install robotframework-dockerlibrary`)
