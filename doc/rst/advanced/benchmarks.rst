.. include:: /include.txt

.. _benchmarks:

===========================
eCAL performance benchmarks
===========================

eCAL includes performance benchmarks based on the Google Benchmark framework. To build them, set ``-DECAL_THIRDPARTY_BUILD_BENCHMARK`` and ``-DECAL_BUILD_BENCHMARKS`` to ``ON``. The applications are named ``ecal_benchmark_<type>``.

The performance benchmarks are also executed automatically for each commit on the master branch of the `GitHub repository <https://github.com/eclipse-ecal/ecal>`. These results can be found on `Bencher <https://bencher.dev/console/projects/ecal/plots>`.

Currently, the benchmarks focus mostly on publisher-subscriber performance via shared memory.


---------------------
Current benchmark set
---------------------

+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| Benchmark name                | For message size     | For background    | Calculated frequency (throughput) | Measurement object            |
|                               | of [Bytes]           | thread count of   | and datarate available            |                               |
+===============================+======================+===================+===================================+===============================+
| BM_eCAL_Send                  | 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message     |
|                               | 16,777,216           |                   |                                   |                               |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_and_Receive      | 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message and |
|                               | 16,777,216           |                   |                                   | wait until it has been        |
|                               |                      |                   |                                   | received                      |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Receive_Latency       | 1; 64; 4096; 262,144;|                   | Yes                               | Time between publishing and   |
|                               | 16,777,216           |                   |                                   | receiving a message           |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_Zero_Copy        | 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message with|
|                               | 16,777,216           |                   |                                   | Zero Copy mode active         |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_Handshake        | 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message with|
|                               | 16,777,216           |                   |                                   | Handshake mode active         |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_Double_Buffer    | 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message with|
|                               | 16,777,216           |                   |                                   | Double Buffer mode active     |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_Zero_Copy_Handsha| 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message with|
| ke                            | 16,777,216           |                   |                                   | Zero Copy and Handshake mode  |
|                               |                      |                   |                                   | active                        |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_Zero_Copy_Double_| 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish u message with|
| Buffer                        | 16,777,216           |                   |                                   | Zero Copy and Double Buffer   |
|                               |                      |                   |                                   | mode active                   |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_Double_Buffer_Han| 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message with|
| dshake                        | 16,777,216           |                   |                                   | Double Buffer and Handshake   |
|                               |                      |                   |                                   | mode active                   |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Send_Zero_Copy_Double_| 1; 64; 4096; 262,144;|                   | Yes                               | Time to publish a message with|
| Buffer_Handshake              | 16,777,216           |                   |                                   | Zero Copy, Double Buffer and  |
|                               |                      |                   |                                   | Handshake mode active         |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Multi_Send            | 1; 4096; 16,777,216  | 1; 2; 4; 8; 16; 32| Yes                               | Time to publish a message with|
|                               |                      |                   |                                   | active background publishers. |
|                               |                      |                   |                                   | Frequency and datarate are    |
|                               |                      |                   |                                   | combined values.              |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Ping                  |                      |                   |                                   | Time to receive a response    |
|                               |                      |                   |                                   | from a server as a client     |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Initialize            |                      |                   |                                   | Time to initialize eCAL       |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Initialize_and_Finaliz|                      |                   |                                   | Time to initialize and        |
| e                             |                      |                   |                                   | finalize eCAL                 |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Publisher_Creation    |                      |                   |                                   | Time to create a publisher    |
|                               |                      |                   |                                   | object                        |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Subscriber_Creation   |                      |                   |                                   | Time to create a subscriber   |
|                               |                      |                   |                                   | object                        |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+
| BM_eCAL_Registration_Delay    |                      |                   |                                   | Time until a new object is    |
|                               |                      |                   |                                   | registered via eCAL           |
+-------------------------------+----------------------+-------------------+-----------------------------------+-------------------------------+