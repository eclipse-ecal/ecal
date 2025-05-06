# Integration Tests for eCAL Middleware

This directory contains a structured suite of integration tests for the eCAL (enhanced Communication Abstraction Layer) middleware. The goal is to validate the functionality, robustness, and performance of inter-process communication scenarios within distributed systems using eCAL.

## Directory Structure

```text
ecal/tests/integration_tests
├── cfg/                   # Configuration files (e.g., ecal.yaml)
├── lib/                   # Shared Python libraries for Robot Framework
│   ├── MyDockerLibrary.py
│   └── GlobalPathsLibrary.py
├── pub_sub_tests/         # Tests based on Publisher-Subscriber communication
│   ├── docker/            # Generic Dockerfile used across pub-sub tests
│   ├── scripts/           # Shared scripts (build, entrypoint, etc.)
│   ├── src/               # C++ source files for pub/sub implementations
│   └── robottests/        # Robot Framework test cases (.robot)
├── CMakeLists.txt         # CMake integration for building components
├── requirements.txt       # List of requirements
└── README.md              # This file
```

## How to Use

### Prerequisites

* Docker installed and working
* eCAL base image built/tagged as `ecal_base`
* `robot` and dependencies if you want to run tests locally:

```bash
pip install -r requirements.txt
```

To build the base image (required before running tests):

```bash
docker build -t ecal_base -f ./docker/Dockerfile.ecal_base .
```

### Build Docker Images (Optional)

Each test scenario requires its own set of images. It will be builded in the .robot file:

```bash
./pub_sub_tests/scripts/build_pubsub_images.sh basic_pub_sub
./pub_sub_tests/scripts/build_pubsub_images.sh multi_pub_sub
```

This builds five variants per scenario:

* `local_shm`
* `local_udp`
* `local_tcp`
* `network_udp`
* `network_tcp`

### Run Robot Tests

To execute a specific test suite for example:

```bash
robot pub_sub_tests/robottests/basic_pub_sub.robot
```

## Test Categories

| Category                   | Description                                   | Subfolder        | More Info                         |
| -------------------------- | --------------------------------------------- | ---------------- | --------------------------------- |
| Publisher-Subscriber       | 1:1, 1\:N, N:1, N\:N pub/sub communication    | `pub_sub_tests/` | [README](pub_sub_tests/README.md) |
| Message Validation         | Payload correctness, malformed input handling | (planned)        |                                   |
| Fault Injection            | Crash handling, disconnects, recovery         | (planned)        |                                   |
| Scalability                | Many nodes, topics, message rate              | (planned)        |                                   |
| RPC Services               | Request/Response testing                      | (planned)        |                                   |
| Filtering                  | Subscriber-side filtering                     | (planned)        |                                   |

## Adding a New Test Case

1. Create a new `.robot` file in `pub_sub_tests/robottests/` (e.g., `advanced_pub_sub.robot`).
2. Reuse the shared `src/`, `docker/`, and `scripts/` folders if possible.
3. Adjust `GlobalPathsLibrary.py` if a new image/tag is introduced.
4. Update the `README.md` inside `pub_sub_tests/` with the new test case.

## CI Integration

These tests are CI-friendly and can be used in pipelines:

```bash
robot --outputdir results pub_sub_tests/robottests/
```

Artifacts like `output.xml`, `log.html`, and `report.html` can be used for further analysis.

## Author

Emircan Tutar