import sys
import time
import capnp
import messages_capnp as msgs
import ecal.core.core as ecal_core
from ecal.core.subscriber import  MessageSubscriber, CapnSubscriber

# callback function for the subscriber 
def callback(topic_name, msg, time):
  # print the received message 
  print(msg) 

if __name__ == "__main__":
  # Initialize eCAL
  ecal_core.initialize(sys.argv, "CapnProto Message Subscriber")

  # Create a subscriber that listenes on the "sensor_imu" topic
  sub = CapnSubscriber("sensor_imu", msgs.Imu)

  # Set the callback
  sub.set_callback(callback)
  
  # infinite loop that keeps the script alive 
  while ecal_core.ok():
    time.sleep(0.5)
  
  # finalize eCAL API
  ecal_core.finalize()
