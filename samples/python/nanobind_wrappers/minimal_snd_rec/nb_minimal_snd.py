import sys
import time

import nanobind_core as ecal_core

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date()))
  
  # initialize eCAL API
  ecal_core.initialize()

  # create publisher
  pub = ecal_core.Publisher("Hello")
  msg = "HELLO WORLD FROM PYTHON"
  
  # send messages
  i = 0
  while ecal_core.ok():
    i = i + 1
    current_message = "{} {:6d}".format(msg, i)
    print("Sending: {}".format(current_message))
    pub.send(current_message,1234)
    time.sleep(0.5)
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
