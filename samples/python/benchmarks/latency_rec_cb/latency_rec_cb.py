# ========================= eCAL LICENSE =================================
#
# Copyright (C) 2016 - 2019 Continental Corporation
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

import sys
import time

# import ecal core
import ecal.core.core as ecal_core

def do_run():

  # initialize eCAL API
  ecal_core.initialize(sys.argv, "py_latency_rec_cb")

  # create publisher/subscriber
  pub = ecal_core.publisher('pkg_reply')
  sub = ecal_core.subscriber('pkg_send')

  # prepare globals
  global msg_num, msg_size, diff_array
  diff_array = []
  msg_num    = 0
  msg_size   = 0

  # define message callback
  def callback(topic_name, msg, snt_time):
    global msg_num, msg_size, diff_array
    diff_array.append(ecal_core.getmicroseconds()[1] - snt_time)
    msg_num += 1
    msg_size = len(msg)
    # reply
    pub.send(msg)

  # apply message callback to subscriber
  sub.set_callback(callback)

  # idle until no more messages are received
  msg_last = 0
  while ecal_core.ok():
    if msg_num > 0 and msg_last == msg_num: break
    else: msg_last = msg_num
    time.sleep(1)

  sum_time = sum(diff_array)
  avg_time = sum_time / len(diff_array)

  print("")
  print("-------------------------------")
  print(" LATENCY / THROUGHPUT TEST")
  print("-------------------------------")
  print("Received buffer size             : {:.0f} kB".format(msg_size / 1024))
  print("Received messages                : {:d}".format(msg_num))
  print("Message average receive time     : {:.0f} us".format(int(avg_time)))
  print("Throughput                       : {:.2f} kB/s".format(((msg_size * msg_num) / 1024) / (sum_time / 1000000.0)))
  print("Throughput                       : {:.2f} MB/s".format(((msg_size * msg_num) / (1024 * 1024)) / (sum_time / 1000000.0)))
  print("Throughput                       : {:.2f} msg/s".format(msg_num / (sum_time / 1000000.0)))
  sys.stdout.flush()
  time.sleep(5)

  ecal_core.finalize()

if __name__ == "__main__":

  while ecal_core.ok():
    do_run()
