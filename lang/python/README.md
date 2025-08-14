# eCAL Python Binding

The Python binding for [eCAL (enhanced Communication Abstraction Layer)](https://github.com/eclipse-ecal/ecal) enables easy use of eCAL features in Python projects. It allows you to build distributed applications and cross-platform inter-process communication.

Find a full getting-started guide at http://ecal.io

## Installation

```bash
pip install eclipse-ecal
```

## Quick Example

Send and receive messages with eCAL in Python:

### Publisher

```python
import ecal.nanobind_core as ecal_core
from ecal.msg.string.core import Publisher as StringPublisher

import time

ecal_core.initialize("HelloWorldPublisher")
publisher = StringPublisher("hello_topic")

for i in range(20):
    message = f"Hello World {i}"
    print(f"Sending: {message}")
    publisher.send(message)
    time.sleep(1)

print("Finished. Shutting down eCAL...")
ecal_core.finalize()
```

### Subscriber

```python
import ecal.nanobind_core as ecal_core
from ecal.msg.string.core import Subscriber as StringSubscriber

def callback(publisher_id, data):
    print(f"Received: {data.message}")

ecal_core.initialize("HelloWorldSubscriber")

print("Waiting for data. Press Enter to exit!")
subscriber = StringSubscriber("hello_topic")
subscriber.set_receive_callback(callback)

input()
ecal_core.finalize()
```

## Further Information

- [eCAL Documentation](http://ecal.io)
- [GitHub Repository](https://github.com/eclipse-ecal/ecal)
