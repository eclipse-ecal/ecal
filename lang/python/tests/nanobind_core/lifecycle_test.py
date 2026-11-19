# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2025 Continental Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ========================= eCAL LICENSE =================================

from collections import deque
from pathlib import Path
import subprocess
import sys
import threading
import time
import uuid
import weakref

import pytest
import ecal.nanobind_core as ecal


def _command(action, topic):
    return [sys.executable, str(Path(__file__).resolve()), action, topic]


def _initialize(name, domain):
    config = ecal.Configuration()
    config.registration.local.transport_type = ecal.LocalTransportType.SHM
    config.registration.local.shm.domain = domain
    config.registration.registration_refresh = 100
    assert ecal.initialize(config, name)


@pytest.fixture(scope="module")
def publisher_topic():
    topic = f"python_lifecycle_{uuid.uuid4().hex}"
    publisher = subprocess.Popen(
        _command("publish", topic),
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    try:
        yield topic
        assert publisher.poll() is None
    finally:
        publisher.terminate()
        try:
            publisher.wait(timeout=5)
        except subprocess.TimeoutExpired:
            publisher.kill()
            publisher.wait()


@pytest.mark.parametrize("action", ["churn", "remove", "finalize", "exit"])
def test_subscriber_lifecycle_during_receive(publisher_topic, action):
    # A separate publisher is essential: in-process delivery does not reproduce
    # the SHM reader / subscriber gate / GIL lock inversion.
    result = subprocess.run(
        _command(action, publisher_topic),
        capture_output=True,
        text=True,
        timeout=45,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert "finished" in result.stdout


def test_service_lifecycle():
    result = subprocess.run(
        _command("service", f"python_service_{uuid.uuid4().hex}"),
        capture_output=True,
        text=True,
        timeout=45,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert "finished" in result.stdout


def _publish(topic):
    _initialize("python_lifecycle_publisher", topic)
    publishers = [ecal.Publisher(f"{topic}_{index}") for index in range(4)]
    while ecal.ok():
        for publisher in publishers:
            publisher.send(b"x" * 20_000)
        time.sleep(0.001)


def _subscribe(action, topic):
    _initialize("python_lifecycle_subscriber", topic)

    def create(index):
        received = threading.Event()

        def on_receive(*_args):
            if not received.is_set():
                received.set()
                # Retain native receive locks while the main thread takes the
                # GIL, then require it again before returning to native code.
                time.sleep(0.02)

        subscriber = ecal.Subscriber(
            f"{topic}_{index}", event_callback=lambda *_args: None
        )
        subscriber.set_receive_callback(on_receive)
        return index, subscriber, received

    live = deque(create(index) for index in range(4))
    for cycle in range(24):
        index, subscriber, received = live.popleft()
        assert received.wait(15), "No samples received from external publisher"
        if action in ("finalize", "exit"):
            live.append((index, subscriber, received))
            if action == "finalize":
                assert ecal.finalize()
            print("finished", flush=True)
            return live  # Keep subscribers alive until interpreter shutdown.
        if action == "remove":
            subscriber.remove_receive_callback()
        del subscriber
        live.append(create(index))

    live.clear()
    assert ecal.finalize()
    print("finished", flush=True)


def _service(name):
    _initialize("python_lifecycle_service", name)
    server = ecal.ServiceServer(name, event_callback=lambda *_args: None)

    def on_event(*_args):
        time.sleep(0.02)

    client = ecal.ServiceClient(name, event_callback=on_event)
    method = ecal.ServiceMethodInformation(method_name="echo")
    entered = threading.Event()

    def echo(_info, request):
        entered.set()
        time.sleep(0.02)
        return 0, request

    assert server.set_method_callback(method, echo)
    deadline = time.monotonic() + 15
    while not client.is_connected():
        assert time.monotonic() < deadline, "Service did not connect"
        time.sleep(0.01)
    assert any(instance.is_connected() for instance in client.get_client_instances())

    payload = b"\x00\xffrequest"
    responses = client.call_with_response("echo", payload, timeout_ms=2000)
    assert responses and responses[0].response == payload
    received = threading.Event()

    def on_response(response):
        assert response.response == payload
        received.set()

    assert client.call_with_callback("echo", payload, on_response, timeout_ms=2000)
    assert received.wait(5)
    received.clear()
    entered.clear()
    assert client.call_with_callback_async("echo", payload, on_response)
    assert entered.wait(5)
    assert server.remove_method_callback("echo")
    assert received.wait(5)
    instances = client.get_client_instances()
    del client
    assert instances and instances[0].is_connected()
    del instances, server

    # Callback ownership must be released safely when any native entity dies.
    for factory in (ecal.Publisher, ecal.Subscriber, ecal.ServiceClient, ecal.ServiceServer):
        callback = lambda *_args: None
        reference = weakref.ref(callback)
        entity = factory(name, event_callback=callback)
        del callback
        assert reference() is not None
        del entity
        assert reference() is None

    assert ecal.finalize()
    print("finished", flush=True)


if __name__ == "__main__":
    action, topic = sys.argv[1:]
    if action == "publish":
        _publish(topic)
    elif action == "service":
        _service(topic)
    else:
        live = _subscribe(action, topic)
