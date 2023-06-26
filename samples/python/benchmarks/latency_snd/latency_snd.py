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
import argparse
import time

# import ecal core
import ecal.core.core as ecal_core

def parse_args(args):
  """ Parse the arguments.
  """
  parser = argparse.ArgumentParser(description='Simple script to measure eCAL latency in Python')

  parser.add_argument('--runs',
    dest='runs',
    help='number of runs to perform',
    default=1000,
    type=int)

  parser.add_argument('--size',
    dest='size',
    help='size of the raw buffer to be sent',
    default=1024 * 1024,
    type=int)
  if len(sys.argv) == 0:
    parser.print_help()
    sys.exit(1)

  return parser.parse_args(args)

def do_run(msg_num, msg_size):

  # create string publisher/subscriber
  pub = ecal_core.publisher('pkg_send')
  sub = ecal_core.subscriber('pkg_reply')

  # time to connect pub/sub
  time.sleep(3)

  # create message
  msg = b'7' * msg_size

  print("")
  print("-------------------------------")
  print(" LATENCY / THROUGHPUT TEST")
  print("-------------------------------")
  print("Sent buffer size                 : {:.0f} kB".format(msg_size / 1024))
  print("Sent messages                    : {:d}".format(msg_num))

  # loop over number of runs
  for run_idx in range(msg_num):

    # send message
    pub.send(msg)

    # receive reply with timeout
    if not sub.receive(timeout=1000):
      print("Package lost after message {0} -> stop.".format(run_idx))
      break

  sys.stdout.flush()

if __name__ == "__main__":
    
  # initialize eCAL API
  ecal_core.initialize([], "py_latency_snd")

  args = parse_args(sys.argv[1:])
  do_run(msg_num=args.runs, msg_size=args.size)

  # finalize eCAL API
  ecal_core.finalize()
