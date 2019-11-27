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

# import ecal core
import ecal.core.core as ecal_core

def do_run():

  # initialize eCAL API
  ecal_core.initialize(sys.argv, "py_latency_rec")

  # create publisher/subscriber
  pub = ecal_core.publisher('pkg_reply')
  sub = ecal_core.subscriber('pkg_send')

  # prepare timestamp list
  diff_array = []

  # loop over number of runs
  rec_timeout = -1
  msg_num = 0
  while True:
    # receive
    ret, msg, snt_time = sub.receive(rec_timeout)

    if ret > 0:
      diff_array.append(ecal_core.getmicroseconds()[1] - snt_time)
      msg_num += 1
      # reply
      pub.send(msg)
      # reduce timeout
      rec_timeout = 1000
      msg_size = len(msg)
    else:
      break

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

  ecal_core.finalize()

if __name__ == "__main__":

  while ecal_core.ok():
    do_run()
