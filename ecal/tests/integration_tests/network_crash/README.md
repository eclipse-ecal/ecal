# Network Crash Test (UDP Local vs. UDP Network)

## Objective

This test checks if local UDP communication in eCAL continues to work when a separate UDP publisher using the network fails.

It simulates a real-world case such as unplugging a network cable.

---

## Test Setup

### Components

| Component           | Container         | eCAL Mode   | Role                                | Message Value |
| ------------------- | ----------------- | ----------- | ----------------------------------- | ------------- |
| `udp_local_pub`     | `udp_local_all`   | local UDP   | Sends messages continuously         | 42            |
| `network_crash_sub` | `udp_local_all`   | local UDP   | Receives messages from both sources | -             |
| `udp_network_pub`   | `udp_network_pub` | network UDP | Sends network-based messages        | 43            |

### Communication Structure

```
                +---------------------+
                | udp_network_pub     | (sends 43)
                +---------------------+
                        |
               Docker network (ecal_test_net)
                        |
     +----------------------------------------+
     | Container: udp_local_all               |
     |                                        |
     |  +-------------------------+           |
     |  | udp_local_pub (42)      |           |
     |  +-------------------------+           |
     |  | network_crash_sub       | <--- receives 42 and 43
     |  +-------------------------+           |
     +----------------------------------------+
```

---

## Test Flow

1. Start the local UDP publisher and subscriber inside one container.
2. Start the network UDP publisher in a second container.
3. Let both publishers send data for 7 seconds.
4. Disconnect the network publisher from the Docker network to simulate a network failure.
5. The subscriber continues to receive messages:

   * It should stop receiving messages with value 43.
   * It should still receive messages with value 42.

---

## Pass Conditions

* The subscriber exits with code `0` (clean shutdown).
* It receives **all 30 messages with value 42** after the network crash.
* It receives **at least 2 messages with value 43** before the crash.

---

## Folder Structure

```
network_crash/
├── scripts/
│   ├── build_images.sh          # Build the test image
│   └── entrypoint.sh            # Starts publishers and subscriber
├── src/
│   ├── network_crash_sub.cpp    # The subscriber
│   ├── udp_local_pub.cpp        # Local UDP publisher (42)
│   └── udp_network_pub.cpp      # Network UDP publisher (43)
├── robottests/
│   └── network_crash_test.robot # Robot Framework test script
├── Dockerfile
└── README.md
```

---

## Running the Test

```bash
robot robottests/network_crash_test.robot
```

---

## Why This Test Is Important

In distributed systems, some modules may run locally and others remotely. If a network path fails, it is important that local parts continue working.

This test confirms that eCAL's local UDP communication is not affected when a remote component disconnects from the network.

---

## Note

You can also try a similar test using TCP instead of UDP to compare behavior. Or try out SHM for local behaviour.
