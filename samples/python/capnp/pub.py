import sys
import time
import capnp
import random
import time
import messages_capnp as msgs
import ecal.core.core as ecal_core
from ecal.core.publisher import StringPublisher, MessagePublisher, CapnPublisher

if __name__ == "__main__":
  ecal_core.initialize(sys.argv, "CapnProto Message Publisher")

  # Create a CapnProto Publisher that publishes on the topic "sensor_imu"
  pub = CapnPublisher("sensor_imu", msgs.Imu)
  
  # Infinite loop (using ecal_core.ok() will enable us to gracefully shutdown
  # the process from another application)
  while ecal_core.ok():
    # Create a capnproto Imu message, populate it with random values and publish it to the topic 
    imu_msg = msgs.Imu.new_message()
    imu_msg.timestamp = time.monotonic_ns()
    imu_msg.accel = [round(random.uniform(2.0, 20.0), 2), round(random.uniform(2.0, 20.0), 2), round(random.uniform(2.0, 20.0), 2)]
    imu_msg.gyro  = [round(random.uniform(0.0, 4.3), 2), round(random.uniform(0.0, 4.3), 2), round(random.uniform(0.0, 4.3), 2)]
    
    print("Sending: {}".format(imu_msg))
    pub.send(imu_msg)
    
    # Sleep 500 ms 
    time.sleep(0.5)
  
  # finalize eCAL API
  ecal_core.finalize()
