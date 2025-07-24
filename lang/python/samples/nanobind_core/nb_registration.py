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
import time

import ecal.nanobind_core as ecal_core

topic = "demo"
service = "hello"

# Define a simple data type for the topic
dtype = ecal_core.DataTypeInformation(
  name="demo_type",
  encoding="raw",
  descriptor=b""
)

# Callback function to print when new publishers appear
def on_publisher_event(publisher_id, event_type):
  print(f"[Publisher Event] Topic: {publisher_id.topic_name}, Event: {event_type.name}")

# Callback function to print when new subscribers appear
def on_subscriber_event(subscriber_id, event_type):
  print(f"[Subscriber Event] Topic: {subscriber_id.topic_name}, Event: {event_type.name}")

def main():
  ecal_core.initialize('Registration Python Sample', ecal_core.init.ALL)

  # Register callbacks
  pub_token = ecal_core.registration.add_publisher_event_callback(on_publisher_event)
  sub_token = ecal_core.registration.add_subscriber_event_callback(on_subscriber_event)
  
  # Create a publisher
  print(f"Creating publisher on {topic}")
  pub = ecal_core.Publisher(topic, data_type_info=dtype)

  # Create a subscriber
  print(f"Creating subscriber on {topic}")
  sub = ecal_core.Subscriber(topic, data_type_info=dtype)
  
  # Create a server
  print(f"Creating server on {service}")
  server = ecal_core.ServiceServer(service, data_type_info=dtype)
  
  # Create a client
  print(f"Creating client on {service}")
  #client = ecal_core.ServiceClient(service)
  
  time.sleep(2)
  
  # Query registration system for publisher info
  print("\n--- Registration Info ---")
  pub_ids = ecal_core.registration.get_publisher_ids()
  if pub_ids:
    for tid in pub_ids:
      print(f"Publisher ID: {tid.name}")
      info = ecal_core.registration.get_publisher_info(tid)
      if info:
        print(f" - Data type: {info.name}, Encoding: {info.encoding}")
  
  # Query registration system for subscriber info
  sub_ids = ecal_core.registration.get_subscriber_ids()
  if sub_ids:
    for tid in sub_ids:
      print(f"Subscriber ID: {tid.name}")
      info = ecal_core.registration.get_subscriber_info(tid)
      if info:
        print(f" - Data type: {info.name}, Encoding: {info.encoding}")
  
  # Query registration system for server info
  server_ids = ecal_core.registration.get_server_ids()
  if server_ids:
    for sid in server_ids:
      print(f"Server ID: {sid.name}")
      info = ecal_core.registration.get_server_info(tid)
      if info:
        print(f" - {info}")
  
  # Query registration system for server info
  #client_ids = ecal_core.registration.get_client_ids()
  #if client_ids:
  #  for sid in client_ids:
  #    print(f"Client ID: {sid.name}")
  #    info = ecal_core.registration.get_client_info(sid)
  #    if info:
  #      print(f" - {info}")

  # Show topic names
  print("Published topics:", ecal_core.registration.get_published_topic_names())
  print("Subscribed topics:", ecal_core.registration.get_subscribed_topic_names())
  
  # Clean up: remove callbacks
  ecal_core.registration.remove_publisher_event_callback(pub_token)
  ecal_core.registration.remove_subscriber_event_callback(sub_token)
 
  ecal_core.finalize()
  
if __name__ == "__main__":
  main()
