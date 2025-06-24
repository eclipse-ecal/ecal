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

import pytest
import time
import ecal.nanobind_core as ecal_core
from typing import Tuple

TOPIC_NAME_PUB = "demo_pytest_pub"
TOPIC_NAME_SUB = "demo_pytest_sub"
DATATYPE_INFORMATION = ecal_core.DataTypeInformation(
        name="demo_type",
        encoding="raw",
        descriptor=b"")

@pytest.fixture(scope="module", autouse=True)
def init_ecal():
    ecal_core.initialize("ecal_pytest_registration", ecal_core.init.ALL)
    yield
    ecal_core.finalize()


def test_publisher_registration():
    pub = ecal_core.Publisher(TOPIC_NAME_PUB, data_type_info=DATATYPE_INFORMATION)
    pub_id = pub.get_topic_id()

    time.sleep(2)

    registration_pub_ids = ecal_core.registration.get_publisher_ids()
    assert pub_id in registration_pub_ids

    registration_datatype_info = ecal_core.registration.get_publisher_info(pub_id)
    assert registration_datatype_info == DATATYPE_INFORMATION

    # we cannot use a subscriver id to get publisher information
    assert ecal_core.registration.get_subscriber_info(pub_id) == None



def test_subscriber_registration():
    sub = ecal_core.Subscriber(TOPIC_NAME_SUB, data_type_info=DATATYPE_INFORMATION)
    sub_id = sub.get_topic_id()

    time.sleep(2)

    registration_sub_ids = ecal_core.registration.get_subscriber_ids()
    assert sub_id in registration_sub_ids

    registration_datatype_info = ecal_core.registration.get_subscriber_info(sub_id)
    assert registration_datatype_info == DATATYPE_INFORMATION
    
    # we cannot use a subscriver id to get publisher information
    assert ecal_core.registration.get_publisher_info(sub_id) == None

def test_published_and_subscribed_topics():
    pub = ecal_core.Publisher(TOPIC_NAME_PUB, data_type_info=DATATYPE_INFORMATION)
    sub = ecal_core.Subscriber(TOPIC_NAME_SUB, data_type_info=DATATYPE_INFORMATION)

    time.sleep(2)

    pub_topics = ecal_core.registration.get_published_topic_names()
    sub_topics = ecal_core.registration.get_subscribed_topic_names()

    assert TOPIC_NAME_PUB in pub_topics
    assert TOPIC_NAME_SUB in sub_topics


def test_server_registration():
    def echo_req_callback(
            method_information : ecal_core.ServiceMethodInformation,
            request : bytes) -> Tuple[int, bytes]:
        response = request  
        return 0, response

    # define the server method "ping" function
    def reverse_req_callback(
            method_information : ecal_core.ServiceMethodInformation,
            request : bytes) -> Tuple[int, bytes]:
        response = request[::-1] #reverse the request
        return 0, response

    server = ecal_core.ServiceServer("mirror")

    # define service methods and add them to the callbacks
    echo_method_info = ecal_core.ServiceMethodInformation()
    echo_method_info.method_name = "echo"
    server.set_method_callback(echo_method_info, echo_req_callback)
  
    reverse_method_info = ecal_core.ServiceMethodInformation()
    reverse_method_info.method_name = "reverse"
    server.set_method_callback(reverse_method_info, reverse_req_callback)

    time.sleep(2)
  
    server_id = server.get_service_id()
    registration_server_ids = ecal_core.registration.get_server_ids()
    assert server_id in registration_server_ids

    registration_server_information = ecal_core.registration.get_server_info(server_id)
    assert echo_method_info in registration_server_information
    assert reverse_method_info in registration_server_information


def test_publisher_subscriber_event_callback():
    callback_publisher_id = None
    callback_publisher_event_type   = None
    callback_subscriber_id = None
    callback_subscriber_event_type   = None    

    def on_publisher_event(publisher_id_, event_type_):
        nonlocal callback_publisher_id, callback_publisher_event_type
        callback_publisher_id = publisher_id_
        callback_publisher_event_type = event_type_

    def on_subscriber_event(subscriber_id, event_type_):
        nonlocal callback_subscriber_id, callback_subscriber_event_type
        callback_subscriber_id = subscriber_id
        callback_subscriber_event_type = event_type_    

    publish_even_token = ecal_core.registration.add_publisher_event_callback(on_publisher_event)
    subscribe_even_token = ecal_core.registration.add_subscriber_event_callback(on_subscriber_event)

    pub = ecal_core.Publisher(TOPIC_NAME_PUB, data_type_info=DATATYPE_INFORMATION)
    pub_id = pub.get_topic_id()
    time.sleep(2.0)

    # Do first round of assertions
    assert callback_publisher_id == pub_id
    assert callback_publisher_event_type == ecal_core.registration.RegistrationEventType.NEW_ENTITY
    assert callback_subscriber_id == None
    assert callback_subscriber_event_type == None

    # Reset variables
    callback_publisher_id = None
    callback_publisher_event_type = None

    del pub  # Explicitly delete publisher to trigger cleanup
    time.sleep(2.0)

    assert callback_publisher_id == pub_id
    assert callback_publisher_event_type == ecal_core.registration.RegistrationEventType.DELETED_ENTITY
    assert callback_subscriber_id == None
    assert callback_subscriber_event_type == None

    # Reset variables
    callback_publisher_id = None
    callback_publisher_event_type = None

    sub = ecal_core.Subscriber(TOPIC_NAME_SUB, data_type_info=DATATYPE_INFORMATION)
    sub_id = sub.get_topic_id()
    time.sleep(2.0)

    # Do first round of assertions
    assert callback_publisher_id == None
    assert callback_publisher_event_type == None
    assert callback_subscriber_id == sub_id
    assert callback_subscriber_event_type == ecal_core.registration.RegistrationEventType.NEW_ENTITY

    # Reset variables
    callback_publisher_id = None
    callback_publisher_event_type = None

    del sub  # Explicitly delete publisher to trigger cleanup
    time.sleep(2.0)

    assert callback_publisher_id == None
    assert callback_publisher_event_type == None 
    assert callback_subscriber_id == sub_id
    assert callback_subscriber_event_type == ecal_core.registration.RegistrationEventType.DELETED_ENTITY

    # See that we can remove a callback again
    ecal_core.registration.remove_publisher_event_callback(publish_even_token)
    ecal_core.registration.remove_subscriber_event_callback(subscribe_even_token)