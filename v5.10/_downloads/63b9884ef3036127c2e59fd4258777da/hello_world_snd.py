import sys
import time

import ecal.core.core as ecal_core
from ecal.core.publisher import StringPublisher

if __name__ == "__main__":
  # initialize eCAL API. The name of our Process will be "Python Hello World Publisher"
  ecal_core.initialize(sys.argv, "Python Hello World Publisher")

  # Create a String Publisher that publishes on the topic "hello_world_python_topic"
  pub = StringPublisher("hello_world_python_topic")
  
  # Create a counter, so something changes in our message
  counter = 0
  
  # Infinite loop (using ecal_core.ok() will enable us to gracefully shutdown
  # the process from another application)
  while ecal_core.ok():
    # Create a message with a counter an publish it to the topic
    current_message = "Hello World {:6d}".format(counter)
    print("Sending: {}".format(current_message))
    pub.send(current_message)
    
    # Sleep 500 ms
    time.sleep(0.5)
    
    counter = counter + 1
  
  # finalize eCAL API
  ecal_core.finalize()