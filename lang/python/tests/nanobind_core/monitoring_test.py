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

import os
from time import sleep

import pytest
import ecal.nanobind_core as ecal_core

DataTypeInformation      = ecal_core.DataTypeInformation
ServiceMethodInformation = ecal_core.ServiceMethodInformation
ServiceServer            = ecal_core.ServiceServer
ServiceClient            = ecal_core.ServiceClient

UNIT_NAME = "Monitoring Python Test"

@pytest.fixture(scope="module", autouse=True)
def init_ecal():
    ecal_core.initialize(UNIT_NAME, ecal_core.init.ALL)
    sleep(2)  # allow registration to propagate
    yield
    ecal_core.finalize()


# ---------------------------------------------------------------------------
# SMonitoring structure
# ---------------------------------------------------------------------------

def test_monitoring_returns_object():
    monitoring = ecal_core.monitoring.get_monitoring()
    assert isinstance(monitoring, ecal_core.monitoring.Monitoring)


def test_monitoring_has_all_collections():
    monitoring = ecal_core.monitoring.get_monitoring()
    assert isinstance(monitoring.processes, list)
    assert isinstance(monitoring.publishers, list)
    assert isinstance(monitoring.subscribers, list)
    assert isinstance(monitoring.servers, list)
    assert isinstance(monitoring.clients, list)


# ---------------------------------------------------------------------------
# Current process appears in the process list
# ---------------------------------------------------------------------------

def test_current_process_is_visible():
    """After initialization the test process must appear in the process list."""
    monitoring = ecal_core.monitoring.get_monitoring()
    assert len(monitoring.processes) >= 1, \
        "Expected at least one process entry after initialization"

    pids = [p.process_id for p in monitoring.processes]
    assert os.getpid() in pids, \
        f"Current PID {os.getpid()} not found in monitoring processes: {pids}"


def test_current_process_fields():
    """Validate all SProcess fields for the current process entry."""
    monitoring = ecal_core.monitoring.get_monitoring()
    current = next(
        (p for p in monitoring.processes if p.process_id == os.getpid()), None
    )
    assert current is not None

    # String fields must be str and non-empty where expected
    assert isinstance(current.host_name, str) and current.host_name != ""
    assert isinstance(current.process_name, str) and current.process_name != ""
    assert isinstance(current.unit_name, str)
    assert current.unit_name == UNIT_NAME
    assert isinstance(current.shm_transport_domain, str)
    assert isinstance(current.process_parameter, str)
    assert isinstance(current.state_info, str)
    assert isinstance(current.time_sync_module_name, str)
    assert isinstance(current.component_init_info, str)
    assert isinstance(current.ecal_runtime_version, str) and current.ecal_runtime_version != ""
    assert isinstance(current.config_file_path, str)

    # Integer fields must be int with reasonable values
    assert isinstance(current.process_id, int) and current.process_id > 0 and current.process_id == os.getpid()
    assert isinstance(current.registration_clock, int) and current.registration_clock > 0
    assert isinstance(current.state_severity, int)
    assert isinstance(current.state_severity_level, int)
    assert isinstance(current.time_sync_state, int)
    assert isinstance(current.component_init_state, int) and current.component_init_state == ecal_core.monitoring.Entity.ALL


# ---------------------------------------------------------------------------
# Entity filter
# ---------------------------------------------------------------------------

def test_entity_filter_process_only():
    """Filtering to Process only must return processes but no topics."""
    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.PROCESS
    )
    assert len(monitoring.processes) >= 1
    assert monitoring.publishers == []
    assert monitoring.subscribers == []
    assert monitoring.servers == []
    assert monitoring.clients == []


def test_entity_filter_none_returns_empty():
    """Filtering with NONE must return all empty collections."""
    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.NONE
    )
    assert monitoring.processes == []
    assert monitoring.publishers == []
    assert monitoring.subscribers == []
    assert monitoring.servers == []
    assert monitoring.clients == []


# ---------------------------------------------------------------------------
# Publisher / Subscriber visibility
# ---------------------------------------------------------------------------

TOPIC_NAME = "monitoring_test_topic"
DATATYPE_INFO = ecal_core.DataTypeInformation(name="test_type", encoding="raw", descriptor=b"")


def test_publisher_appears_in_monitoring():
    pub = ecal_core.Publisher(TOPIC_NAME, data_type_info=DATATYPE_INFO)
    sleep(2)

    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.PUBLISHER
    )
    topic_names = [t.topic_name for t in monitoring.publishers]
    assert TOPIC_NAME in topic_names


def test_subscriber_appears_in_monitoring():
    sub = ecal_core.Subscriber(TOPIC_NAME, data_type_info=DATATYPE_INFO)
    sleep(2)

    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.SUBSCRIBER
    )
    topic_names = [t.topic_name for t in monitoring.subscribers]
    assert TOPIC_NAME in topic_names


def test_topic_fields():
    """Validate STopic field types via the publisher list."""
    pub = ecal_core.Publisher(TOPIC_NAME, data_type_info=DATATYPE_INFO)
    sleep(2)

    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.PUBLISHER
    )
    entry = next((t for t in monitoring.publishers if t.topic_name == TOPIC_NAME), None)
    assert entry is not None

    assert isinstance(entry.host_name, str) and entry.host_name != ""
    assert isinstance(entry.process_id, int) and entry.process_id > 0 and entry.process_id == os.getpid()
    assert isinstance(entry.process_name, str)
    assert isinstance(entry.unit_name, str)
    assert isinstance(entry.topic_name, str) and entry.topic_name == TOPIC_NAME
    assert isinstance(entry.direction, str)
    assert isinstance(entry.transport_layer, list)
    assert isinstance(entry.registration_clock, int)
    assert isinstance(entry.topic_size, int)
    assert isinstance(entry.connections_local, int)
    assert isinstance(entry.connections_external, int)
    assert isinstance(entry.message_drops, int)


# ---------------------------------------------------------------------------
# Server / Client visibility
# ---------------------------------------------------------------------------

SERVICE_NAME = "monitoring_test_service"
SERVICE_DTYPE = DataTypeInformation(name="test_type", encoding="raw", descriptor=b"")
SERVICE_METHOD = ServiceMethodInformation(
    method_name="echo",
    request_type=SERVICE_DTYPE,
    response_type=SERVICE_DTYPE,
)


def test_server_appears_in_monitoring():
    """A registered ServiceServer must appear in monitoring.servers."""
    server = ServiceServer(SERVICE_NAME, event_callback=None)
    server.set_method_callback(SERVICE_METHOD, lambda method_info, request: (0, request))
    sleep(2)

    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.SERVER
    )
    service_names = [s.service_name for s in monitoring.servers]
    assert SERVICE_NAME in service_names, \
        f"Service '{SERVICE_NAME}' not found in monitoring servers: {service_names}"


def test_client_appears_in_monitoring():
    """A registered ServiceClient must appear in monitoring.clients."""
    client = ServiceClient(SERVICE_NAME, methods={SERVICE_METHOD})
    sleep(2)

    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.CLIENT
    )
    service_names = [c.service_name for c in monitoring.clients]
    assert SERVICE_NAME in service_names, \
        f"Service '{SERVICE_NAME}' not found in monitoring clients: {service_names}"


def test_server_fields():
    """Validate all SServer field types for the registered server entry."""
    server = ServiceServer(SERVICE_NAME, event_callback=None)
    server.set_method_callback(SERVICE_METHOD, lambda method_info, request: (0, request))
    sleep(2)

    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.SERVER
    )
    entry = next((s for s in monitoring.servers if s.service_name == SERVICE_NAME), None)
    assert entry is not None

    assert isinstance(entry.registration_clock, int) and entry.registration_clock > 0
    assert isinstance(entry.host_name, str) and entry.host_name != ""
    assert isinstance(entry.process_name, str) and entry.process_name != ""
    assert isinstance(entry.unit_name, str)
    assert isinstance(entry.process_id, int) and entry.process_id > 0 and entry.process_id == os.getpid()
    assert isinstance(entry.service_name, str) and entry.service_name == SERVICE_NAME
    assert isinstance(entry.service_id, int)
    assert isinstance(entry.version, int)
    assert isinstance(entry.tcp_port_v0, int)
    assert isinstance(entry.tcp_port_v1, int)
    assert isinstance(entry.methods, list)
    assert len(entry.methods) == 1
    method = entry.methods[0]
    assert isinstance(method.method_name, str) and method.method_name == SERVICE_METHOD.method_name


def test_client_fields():
    """Validate all SClient field types for the registered client entry."""
    client = ServiceClient(SERVICE_NAME, methods={SERVICE_METHOD})
    sleep(2)

    monitoring = ecal_core.monitoring.get_monitoring(
        ecal_core.monitoring.Entity.CLIENT
    )
    entry = next((c for c in monitoring.clients if c.service_name == SERVICE_NAME), None)
    assert entry is not None

    assert isinstance(entry.registration_clock, int) and entry.registration_clock > 0
    assert isinstance(entry.host_name, str) and entry.host_name != ""
    assert isinstance(entry.process_name, str) and entry.process_name != ""
    assert isinstance(entry.unit_name, str)
    assert isinstance(entry.process_id, int) and entry.process_id > 0 and entry.process_id == os.getpid()
    assert isinstance(entry.service_name, str) and entry.service_name == SERVICE_NAME
    assert isinstance(entry.service_id, int)
    assert isinstance(entry.version, int)
    assert isinstance(entry.methods, list)
    assert len(entry.methods) == 1
    method = entry.methods[0]
    assert isinstance(method.method_name, str) and method.method_name == SERVICE_METHOD.method_name



# ---------------------------------------------------------------------------
# Serialized monitoring
# ---------------------------------------------------------------------------

def test_get_serialized_monitoring_returns_bytes():
    result = ecal_core.monitoring.get_serialized_monitoring()
    assert isinstance(result, bytes)
    assert len(result) > 0


def test_get_serialized_monitoring_entity_filter():
    """Filtering by NONE should return a shorter (or equal) serialized blob."""
    all_bytes = ecal_core.monitoring.get_serialized_monitoring(
        ecal_core.monitoring.Entity.ALL
    )
    none_bytes = ecal_core.monitoring.get_serialized_monitoring(
        ecal_core.monitoring.Entity.NONE
    )
    assert isinstance(all_bytes, bytes)
    assert isinstance(none_bytes, bytes)
    assert len(all_bytes) >= len(none_bytes)

    