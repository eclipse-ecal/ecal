.. include:: /include.txt

.. _performance:

========================
Performance measurements
========================

The following table shows the latency in µs between a single publisher / subscriber connection for different payload sizes (two different processes running on the same host).
You can simply measure the latency on your own machine by running the ecal_sample_latency_snd and ecal_sample_latency_rec_cb sample applications.
The first two columns are showing the performance for the eCAL builtin shared memory layer and the last column for the iceoryx shared memory layer (configured by cmake option ECAL_LAYER_ICEORYX).

First start ecal_sample_latency_rec_cb.
This application will receive the published payloads, send them back to the sender and print out the average receive time, the message frequency and the data throughput over all received messages.
The sending application ecal_sample_latency_snd can be configured that way:

.. code-block:: console
   
   ecal_sample_latency_snd  -s <payload_size [kB]> -r <send loops>

The table shows the results for a Windows and a Linux platform (200000 samples 1kB - 512kB / 10000 samples > 512 kB, zero drops).

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

+--------------------+-------------------+----------------------+----------------------+
|| Payload Size (kB) || Win10 AMD64 (µs) || Ubuntu18 AMD64 (µs) || Ubuntu18 AMD64 (µs) |
||                   ||        eCAL SHM  ||           eCAL SHM  ||        Iceoryx SHM  |
+====================+===================+======================+======================+
||                1  ||              10  ||                  4  ||                  6  |
+--------------------+-------------------+----------------------+----------------------+
||                2  ||              10  ||                  4  ||                  6  |
+--------------------+-------------------+----------------------+----------------------+
||                4  ||              10  ||                  5  ||                  6  |
+--------------------+-------------------+----------------------+----------------------+
||                8  ||              11  ||                  5  ||                  6  |
+--------------------+-------------------+----------------------+----------------------+
||               16  ||              12  ||                  6  ||                  6  |
+--------------------+-------------------+----------------------+----------------------+
||               32  ||              13  ||                  7  ||                  8  |
+--------------------+-------------------+----------------------+----------------------+
||               64  ||              16  ||                 10  ||                 10  |
+--------------------+-------------------+----------------------+----------------------+
||              128  ||              21  ||                 15  ||                 13  |
+--------------------+-------------------+----------------------+----------------------+
||              256  ||              32  ||                 33  ||                 19  |
+--------------------+-------------------+----------------------+----------------------+
||              512  ||              56  ||                 50  ||                 28  |
+--------------------+-------------------+----------------------+----------------------+
||             1024  ||             103  ||                154  ||                 82  |
+--------------------+-------------------+----------------------+----------------------+
||             2048  ||             363  ||                392  ||                177  |
+--------------------+-------------------+----------------------+----------------------+
||             4096  ||             867  ||                877  ||                420  |
+--------------------+-------------------+----------------------+----------------------+
||             8192  ||            1814  ||               1119  ||                534  |
+--------------------+-------------------+----------------------+----------------------+
||            16384  ||            3956  ||               2252  ||               1060  |
+--------------------+-------------------+----------------------+----------------------+
