import sys
import time

import ecal.nanobind_core as ecal_core

def my_event_callback(topic_id, callback_data):
  print(topic_id)
  print(callback_data)

def main():
  # print eCAL version and date
  print("eCAL {} ({})\n".format(ecal_core.get_version_string(), ecal_core.get_version_date_string()))
  
  # initialize eCAL API
  ecal_core.initialize()

  # create publisher
  config = ecal_core.get_publisher_configuration()
  datatype_info = ecal_core.DataTypeInformation()
  datatype_info.encoding = "string"
  datatype_info.descriptor = b"abcd"

  pub = ecal_core.Publisher("Hello", datatype_info)
  msg = "HELLO WORLD FROM PYTHON"
  
  print(pub.get_data_type_information())
  
  # send messages
  i = 0
  while ecal_core.ok():
    i = i + 1
    current_message = "{} {:6d}".format(msg, i)
    byte_data = current_message.encode('utf-8')
    print(byte_data)
    pub.send(byte_data)
    time.sleep(0.5)
  
  # finalize eCAL API
  ecal_core.finalize()

if __name__ == "__main__":
  main()
