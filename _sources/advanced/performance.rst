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
||         1 kB ||          10 ||           5 ||              8 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||         2 kB ||          10 ||           5 ||              8 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||         4 kB ||          10 ||           5 ||              8 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||         8 kB ||          11 ||           5 ||             10 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||        16 kB ||          13 ||           5 ||             12 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||        32 kB ||          13 ||           5 ||             14 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||        64 kB ||          17 ||           5 ||             16 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||       128 kB ||          23 ||           5 ||             28 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||       256 kB ||          37 ||           5 ||            135 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||       512 kB ||          64 ||           5 ||            158 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||         1 MB ||         112 ||           5 ||            182 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||         2 MB ||         344 ||           5 ||            316 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||         4 MB ||         773 ||           5 ||            692 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||         8 MB ||        1762 ||           5 ||           1542 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||        16 MB ||        3458 ||           5 ||           2998 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
||        32 MB ||        6642 ||           5 ||           5880 ||              3 |
+---------------+--------------+--------------+-----------------+-----------------+
