# Integration Tests for eCAL Middleware

This directory contains a structured suite of integration tests for the eCAL (enhanced Communication Abstraction Layer) middleware. The goal is to validate the functionality, robustness, and performance of inter-process communication scenarios within distributed systems using eCAL.

## Directory Structure

```
ecal/tests/integration_tests
├── cfg/                   # Configuration files (e.g., ecal.yaml)
├── lib/                   # Shared Python libraries for Robot Framework
├── pub_sub_tests/         # Tests based on Publisher-Subscriber communication
│   ├── docker/            # Generic Dockerfile used across pub-sub tests
│   ├── scripts/           # Shared scripts (build, entrypoint, etc.)
│   ├── src/               # C++ source files for pub/sub implementations
│   └── robottests/        # Robot Framework test cases
├── CMakeLists.txt         # CMake integration for building components
├── requirements.txt       # List of requirements 
└── README.md              # This file
```

## How to Use

### Prerequisites
- Docker installed and working
- eCAL base image built/tagged as `ecal_base`
- `robot` and dependencies if you want to run tests locally:  
  `pip install -r requirements.txt`

**Note:** Before building the test images, make sure the base image `ecal_base` exists locally.
You can build it from the eCAL source root with the following command:

```bash
docker build -t ecal_base -f ./docker/Dockerfile.ecal_base .
```

### Build Docker Images
```bash
./pub_sub_tests/scripts/build_pubsub_images.sh basic_pub_sub
```

### Run Robot Tests
You can run tests manually using Robot Framework CLI or VSCode extension:
```bash
robot pub_sub_tests/robottests/basic_pub_sub.robot
```

Or with arguments:
```bash
robot --variable BASE_IMAGE:basic_pub_sub pub_sub_tests/robottests/basic_pub_sub.robot
```

## Test Categories

| Category | Description | Subfolder | Details |
|----------|-------------|-----------|---------|
| Basic Publisher-Subscriber | Tests for 1:1, 1:N, and N:1 communication | [pub_sub_tests/](pub_sub_tests/) | [basic_pub_sub](pub_sub_tests/README.md) |
| Message Validation | Payload correctness and malformed handling | (planned) |  |
| Fault Injection | Crash handling, reconnects, network loss | (planned) |  |
| Scalability | Stress tests with many nodes and topics | (planned) |  |
| RPC Services | Tests for Request/Response over eCAL | (planned) |  |
| Filtering | Content/topic-based subscriber filters | (planned) |  |

## How to Add a New Test Case
1. Create a new .robot file in `pub_sub_tests/robottests/` (e.g., `new_pub_sub.robot`).
2. Reuse the shared `src/`, `docker/`, and `scripts/` if applicable.
3. If required, extend `build_pubsub_images.sh` with new tags for additional scenarios.
4. Document the test scenario in `README.md`.

## CI Integration
These tests are designed to be used in CI. You can integrate them into GitHub Actions, GitLab CI, or Jenkins by calling:
```bash
robot --outputdir results pub_sub_tests/robottests/
```

## Author
Emircan Tutar