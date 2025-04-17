# Test 1.1 – Publisher to Subscriber Communication

This test checks whether a basic message can be transmitted from a publisher to a subscriber using the eCAL middleware.

## Purpose

To verify the fundamental functionality of eCAL:
- The publisher sends a message to a topic
- The subscriber receives the message
- The content matches
- No errors occur

This is the most basic test and acts as a foundation for all other integration tests.

## Structure

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

## How It Works

1. Robot Framework starts two containers in a custom Docker network.
    - One runs the publisher
    - One runs the subscriber
2. The subscriber listens on a topic
3. The publisher sends a test message: `"Hello from publisher"`
4. The subscriber writes the received message to `received.log`
5. Robot Framework verifies that the message content is correct

The use of a dedicated Docker network ensures that the test does not rely on shared memory and instead validates communication over TCP/UDP.

## How to Run

### Build the Docker image:
```bash
cd test_1_1_pubsub/docker
docker build -t ecal_test_pubsub .
```

### Create a Docker network for inter-container communication:
```bash
docker network create ecal_test_net
```

### Run the test:
```bash
cd ../robot
pip install -r ../scripts/requirements.txt
robot test_pubsub.robot
```

> Make sure Docker is running, and you have the rights to execute Docker commands.

## Requirements

- eCAL (pre-installed in Docker image)
- Robot Framework
- DockerLibrary
- Docker

## Notes

This test is designed to run in isolation in a Docker network to prevent fallback to shared memory. It ensures that network-based communication (TCP/UDP) is tested explicitly.

---
