import sys
import time

import ecal.core.core as ecal_core
from ecal.core.publisher import ProtoPublisher

# Import the "hello_world_pb2.py" file that we have just generated from the
# proto_messages directory 
import proto_messages.hello_world_pb2 as hello_world_pb2

if __name__ == "__main__":
  # initialize eCAL API. The name of our Process will be
  # "Python Protobuf Publisher"
  ecal_core.initialize(sys.argv, "Python Protobuf Publisher")

  # Create a Protobuf Publisher that publishes on the topic
  # "hello_world_python_protobuf_topic". We also pass it the datatype we are
  # going to send later. By doing this we enable the eCAL Monitor to dynamically
  # show the content of the messages.
  pub = ProtoPublisher("hello_world_python_protobuf_topic"
                      , hello_world_pb2.HelloWorld)
  
  # Create a counter and some messages, so we see something changing in the
  # message, later
  counter = 0
  dummy_messages = ["The story so far:"
                  , "In the beginning the Universe was created."
                  , "This has made a lot of people very angry"
                  , "and been widely regarded as a bad move."]
  
  # Infinite loop (using ecal_core.ok() will enable us to gracefully shutdown
  # the process from another application)
  while ecal_core.ok():
    # Create a message and fill it with some data
    protobuf_message = hello_world_pb2.HelloWorld()
    protobuf_message.name = "Douglas Adams"
    protobuf_message.id   = counter
    protobuf_message.msg  = dummy_messages[counter % 4]

    print("Sending message {}".format(counter))

    # actually send the message to the topic this publisher was created for
    pub.send(protobuf_message)
    
    # Sleep 1s
    time.sleep(1)
    
    counter = counter + 1
  
  # finalize eCAL API
  ecal_core.finalize()