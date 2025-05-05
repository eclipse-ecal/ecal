# Pub/Sub Integration Tests for eCAL

This directory contains containerized integration tests specifically focused on testing pub/sub (publisher/subscriber) communication within the [eCAL](https://github.com/eclipse-ecal/ecal) middleware framework.

## Structure

```text
pub_sub_tests/
├── docker/              # Shared Dockerfile used for all pub/sub test scenarios
├── robottests/          # Robot Framework test definitions (.robot)
│   ├── basic_pub_sub.robot
│   ├── multi_pub_sub.robot
│   └── ...
├── scripts/             # Shared build + entrypoint scripts for all scenarios
│   ├── build_pubsub_images.sh
│   └── entrypoint.sh
├── src/                 # Shared C++ publisher, subscriber, helper logic
│   ├── CMakeLists.txt
│   ├── publisher.cpp
│   ├── subscriber.cpp
│   ├── ecal_config_helper.cpp
│   └── ecal_config_helper.h
└── README.md            # This file
```

## How to Build Images (optional)

To build all pub/sub test images:

```bash
./pub_sub_tests/scripts/build_pubsub_images.sh basic_pub_sub
```

This builds five image variants (local SHM, local UDP, local TCP, network UDP, network TCP) tagged as `basic_pub_sub_<mode>`.

## How to Run Tests

Use Robot Framework CLI:

```bash
robot pub_sub_tests/robottests/basic_pub_sub.robot
```

Or use the `robotcode` extension inside VSCode for step-by-step debugging.

## Adding New Test Scenarios

You can reuse most of the infrastructure for new scenarios (e.g., multi pub/sub, fault injection):

1. Add a new `.robot` file to `pub_sub_tests/robottests/`, e.g. `new_pub_sub.robot`
2. If needed, adjust `src/` or clone it per test scenario
3. The shared `Dockerfile` and `entrypoint.sh` will continue to work as long as the structure remains consistent
4. Extend `build_pubsub_images.sh` only if a separate image tag or build context is needed
5. Document the new scenario in this README (see below)

## Available Test Scenarios

| Scenario      | Description                         | Robot Test File                                         |
| ------------- | ----------------------------------- | ------------------------------------------------------- |
| Basic Pub/Sub | 1:1 communication with 5 transports | [basic\_pub\_sub.robot](robottests/basic_pub_sub.robot) |
| Multi Pub/Sub | Multiple pubs/subs on same topic    | [multi\_pub\_sub.robot](robottests/multi_pub_sub.robot) |

---

## ⚠️ Note on Timing-Related Test Failures

Some tests may occasionally fail due to timing issues — especially in network-based scenarios or when running on slower systems. These are usually caused by:

* Docker containers needing more time to initialize before communication starts
* Subscribers not yet being fully registered when a publisher sends the first message

To mitigate this, `sleep` delays are used between container launches. However, these values are fixed and may not always be sufficient depending on the system or environment.

If a test fails with `Communication failed!: 1 != 0`, try rerunning it. You can also increase the sleep durations in the test logic if needed.
