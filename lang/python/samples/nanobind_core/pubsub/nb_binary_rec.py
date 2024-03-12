import sys
from time import sleep

import ecal.nanobind_core as ecal_core

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date()))
  
  # initialize eCAL API
  ecal_core.initialize()
  
  # create subscriber
  sub = ecal_core.Subscriber("blob")
  
  # receive messages
  while ecal_core.ok():
    msg = sub.receive(0)
    print("Received:  {} ".format(msg))
    sleep(0.1)
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()