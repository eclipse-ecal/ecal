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

import time
import pytest
import threading

import ecal.nanobind_core as ecal_core

@pytest.fixture(scope="module", autouse=True)
def init_ecal():
    ecal_core.initialize("ecal_pytest_services", ecal_core.init.ALL)
    yield
    ecal_core.finalize()

# Aliases for readability
DataTypeInformation      = ecal_core.DataTypeInformation
ServiceMethodInformation = ecal_core.ServiceMethodInformation
ServiceResponse          = ecal_core.ServiceResponse
ServiceClient            = ecal_core.ServiceClient
ServiceServer            = ecal_core.ServiceServer

# A simple dtype used for our calls
DEFAULT_DTYPE = DataTypeInformation(
    name="test_type",
    encoding="raw",
    descriptor=b""     # empty descriptor
)

# A single-method set for convenience
ECHO_METHOD = { ServiceMethodInformation(
    method_name="echo",
    request_type=DEFAULT_DTYPE,
    response_type=DEFAULT_DTYPE
)}


def create_server(server_name : str):
    server = ServiceServer(server_name, event_callback=None)
    server.set_method_callback(
        ServiceMethodInformation(
            method_name="echo",
            request_type=DEFAULT_DTYPE,
            response_type=DEFAULT_DTYPE
        ),
        # Python callback: return (int, str)
        lambda method_info, request: (0, request)
    )
    return server


def create_client(client_name : str):
    client = ServiceClient(
        client_name,
        methods={ ServiceMethodInformation(
            method_name="echo",
            request_type=DEFAULT_DTYPE,
            response_type=DEFAULT_DTYPE
        )}
    )
    return client


def test_service_client_constructor_plain():
    # 1) No methods, no event callback
    client = ServiceClient("svc_name")
    assert client.get_service_name() == "svc_name"

def test_service_client_constructor_methods():
    # 2) Methods only
    client = ServiceClient("svc_name", methods=ECHO_METHOD)
    assert client.get_service_name() == "svc_name"

def test_service_client_constructor_event_callback():
    # 3) Event‐callback only
    def on_client_event(sid, data):
        # do nothing
        pass
    client = ServiceClient("svc_name",
                       event_callback=on_client_event)
    assert client.get_service_name() == "svc_name"

def test_service_client_constructor_all_arguments():
    # 4) Both methods and callback
    def on_client_event(sid, data):
        # do nothing
        pass
    client = ServiceClient("svc_name",
                       methods=ECHO_METHOD,
                       event_callback=on_client_event)
    assert client.get_service_name() == "svc_name"


def test_client_server_call_with_response():
    """Start a server offering 'echo', then a client calls it."""
    svc_name = "roundtrip_svc"

    # 1) create service and client
    server = create_server(svc_name)
    client = create_client(svc_name)

    # give eCAL a moment to wire up
    time.sleep(2)

    # Check both sides report connected
    assert server.is_connected() is True
    assert client.is_connected() is True

    # 3) Do a blocking call
    result = client.call_with_response("echo", b"hello", timeout_ms=1000)
    assert result is not None, "call_with_response failed"
    # result is a list of ServiceResponse
    assert isinstance(result, list) and len(result) == 1
    resp = result[0]
    assert isinstance(resp, ServiceResponse)
    assert resp.response == b"hello"

    del server
    del client      

def test_client_server_call_with_callback():
    """Start a server offering 'echo', then a client calls it."""
    svc_name = "roundtrip_svc"

    # 1) create service and client
    server = create_server(svc_name)
    client = create_client(svc_name)

    # give eCAL a moment to wire up
    time.sleep(2)

    # Check both sides report connected
    assert server.is_connected() is True
    assert client.is_connected() is True

    service_response = None
    def on_response(resp: ServiceResponse):
        nonlocal service_response
        service_response = resp

    # Do a callback‐style call
    ok = client.call_with_callback("echo", b"world", on_response, timeout_ms=1000)
    # No need to wait, as this is synchronous
    assert ok is True
    assert service_response.response == b"world"

    del server
    del client      


def test_client_server_call_with_callback_async():
    """Start a server offering 'echo', then a client calls it."""
    svc_name = "roundtrip_svc"

    # 1) create service and client
    server = create_server(svc_name)
    client = create_client(svc_name)

    # give eCAL a moment to wire up
    time.sleep(2)

    # Check both sides report connected
    assert server.is_connected() is True
    assert client.is_connected() is True

    # 4) Do a callback‐style call
    service_response = None
    def on_response(resp: ServiceResponse):
        nonlocal service_response
        service_response = resp

    ok2 = client.call_with_callback_async("echo", b"async", on_response)
    assert ok2 is True
    time.sleep(0.2)
    assert service_response.response == b"async"    

    del server
    del client  


def test_client_server_call_instances_with_response():
    """Start a server offering 'echo', then a client calls it."""
    svc_name = "roundtrip_svc"

    # 1) create service and 2 clients
    server_1 = create_server(svc_name)
    server_2 = create_server(svc_name)
    client = create_client(svc_name)

    # give eCAL a moment to wire up
    time.sleep(2)

    # Check both sides report connected
    assert server_1.is_connected() 
    assert server_2.is_connected()    
    assert client.is_connected()

    client_instances = client.get_client_instances()
    assert len(client_instances) == 2 # we have two servers, e.g. two instances

    for client_instance in client_instances:
        assert client_instance.is_connected()
        (ok, response) = client_instance.call_with_response("echo", b"hello", timeout_ms=1000)
        assert ok, "call_with_response failed"
        assert isinstance(response, ServiceResponse)
        assert response.response == b"hello"

    del server_1
    del server_2
    del client      

def test_client_server_call_instances_with_callback():
    """Start a server offering 'echo', then a client calls it."""
    svc_name = "roundtrip_svc"

    # 1) create service and 2 clients
    server_1 = create_server(svc_name)
    server_2 = create_server(svc_name)
    client = create_client(svc_name)

    # give eCAL a moment to wire up
    time.sleep(2)

    # Check both sides report connected
    assert server_1.is_connected() 
    assert server_2.is_connected()    
    assert client.is_connected()

    # 4) Do a callback‐style call
    service_response = None
    def on_response(resp: ServiceResponse):
        nonlocal service_response
        service_response = resp
    client_instances = client.get_client_instances()
    assert len(client_instances) == 2 # we have two servers, e.g. two instances

    for client_instance in client_instances:
        assert client_instance.is_connected()
        # Do a callback‐style call
        ok = client_instance.call_with_callback("echo", b"world", on_response, timeout_ms=1000)
        # No need to wait, as this is synchronous
        assert ok is True
        assert service_response.response == b"world"
        service_response = None    

    del server_1
    del server_2
    del client      

def test_client_server_call_instances_with_callback_async():
    """Start a server offering 'echo', then a client calls it."""
    svc_name = "roundtrip_svc"

    # 1) create service and 2 clients
    server_1 = create_server(svc_name)
    server_2 = create_server(svc_name)
    client = create_client(svc_name)

    # give eCAL a moment to wire up
    time.sleep(2)

    # Check both sides report connected
    assert server_1.is_connected() 
    assert server_2.is_connected()    
    assert client.is_connected()

    # 4) Do a callback‐style call
    service_response = None
    callback_synchronization = threading.Event()
    def on_response(resp: ServiceResponse):
        nonlocal service_response
        service_response = resp
        callback_synchronization.set()

    client_instances = client.get_client_instances()
    assert len(client_instances) == 2 # we have two servers, e.g. two instances

    for client_instance in client_instances:
        assert client_instance.is_connected()
        ok2 = client_instance.call_with_callback_async("echo", b"async", on_response)
        assert ok2 is True
        callback_synchronization.wait()
        callback_synchronization.clear()
        assert service_response.response == b"async"
        service_response = None      

    del server_1
    del server_2
    del client  