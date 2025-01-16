.. include:: /include.txt

.. _performance:

========================
Performance measurements
========================

The following table shows latencies for a 1:1 publish/subscribe connection for different payload sizes (the processes are running on the same host).
You can measure those latencies on your own computer by running the ecal_sample_latency_snd and ecal_sample_latency_rec sample applications.

#. Start ``ecal_sample_latency_rec`` . This application will receive the published payloads, print out the number of the received messages, the latencies and the data throughputs.

#. Start ``ecal_sample_latency_snd`` . This application will publish the payloads and can be configured via command line parameter:

.. code-block:: console

   ecal_sample_latency_snd [-z] [-s <int>] [-r <int>] [-b <int>] [--]

   Where:
      -z,  --zero_copy
        Switch zero copy mode on.

      -s <int>,  --size <int>
        Messages size in kB.

      -r <int>,  --runs <int>
        Number of messages to send.

      -b <int>,  --mem_buffer <int>
        Number of memory files per connection.


A message size -1 will run the tests over different message sizes in a range from 1kB to 32 MB.

The table shows the results for the Windows 10 and the Ubuntu 22.04 platform with and without zero copy mode (message size = -1 (1kB - 32MB), runs = 5000, message buffer = 1).

.. code-block:: none

   -------------------------------
    Platform
   -------------------------------
   System Manufacturer:     HP
   System Model:            HP ZBook 15 G5
   System Type:             x64-based PC
   Processor(s):            1 Prozessor(s) Installed.
                            [01]: Intel64 Family 6 Model 158 Stepping 10 GenuineIntel ~2592 MHz
   Total Physical Memory:   32.614 MB


.. raw:: html

  <style> .line {text-align:right;} </style>

+---------------+--------------+--------------+-----------------+-----------------+
|| Payload Size || Win10 AMD64 || Win10 AMD64 || Ubuntu22 AMD64 || Ubuntu22 AMD64 |
||              ||         SHM ||      SHM ZC ||           SHM  ||         SHM ZC |
||              ||        (µs) ||        (µs) ||           (µs) ||           (µs) |
+===============+==============+==============+=================+=================+
||         1 kB ||          10 ||           9 ||              8 ||              9 |
+---------------+--------------+--------------+-----------------+-----------------+
||         2 kB ||          10 ||           9 ||              8 ||              9 |
+---------------+--------------+--------------+-----------------+-----------------+
||         4 kB ||          10 ||           9 ||              8 ||              9 |
+---------------+--------------+--------------+-----------------+-----------------+
||         8 kB ||          11 ||           9 ||             10 ||              9 |
+---------------+--------------+--------------+-----------------+-----------------+
||        16 kB ||          13 ||          10 ||             12 ||             10 |
+---------------+--------------+--------------+-----------------+-----------------+
||        32 kB ||          13 ||          10 ||             14 ||             10 |
+---------------+--------------+--------------+-----------------+-----------------+
||        64 kB ||          17 ||          11 ||             16 ||             10 |
+---------------+--------------+--------------+-----------------+-----------------+
||       128 kB ||          23 ||          14 ||             28 ||             12 |
+---------------+--------------+--------------+-----------------+-----------------+
||       256 kB ||          37 ||          18 ||            135 ||             12 |
+---------------+--------------+--------------+-----------------+-----------------+
||       512 kB ||          64 ||          26 ||            158 ||             21 |
+---------------+--------------+--------------+-----------------+-----------------+
||         1 MB ||         112 ||          42 ||            182 ||             40 |
+---------------+--------------+--------------+-----------------+-----------------+
||         2 MB ||         344 ||          76 ||            316 ||             67 |
+---------------+--------------+--------------+-----------------+-----------------+
||         4 MB ||         773 ||         149 ||            692 ||            146 |
+---------------+--------------+--------------+-----------------+-----------------+
||         8 MB ||        1762 ||         491 ||           1542 ||            389 |
+---------------+--------------+--------------+-----------------+-----------------+
||        16 MB ||        3458 ||        1127 ||           2998 ||            926 |
+---------------+--------------+--------------+-----------------+-----------------+
||        32 MB ||        6642 ||        2471 ||           5880 ||           2078 |
+---------------+--------------+--------------+-----------------+-----------------+
