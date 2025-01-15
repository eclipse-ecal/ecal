.. include:: /include.txt

.. _threading_model:

==================================
Threading model
==================================

eCAL is using different transport layers for intraprocess, interprocess or interhost communication. With a call of ``eCAL::Initialize`` different internal threads are started, with a call of ``eCAL::Finalize`` they are stopped:

- udp monitoring send thread
- udp registration send thread
- udp registration receive thread
- udp logging receive thread
- udp message (payload) receive thread

For the eCAL user interface entities ``eCAL::CTimer``, ``eCAL::CSubscriber``, ``eCAL::CServiceServer``, ``eCAL::CServiceClient`` additional threads are utilized:

- shared memory synchronization event thread (1 thread per handled memory file)
- timer callback (1 thread per callback)
- tcp client/server implementation (2 threads per instance)

For user API callback functions eCAL is protecting the forwarded data (message header, message payload ..) as long as the callback is processed. 

For subscriptions and their callbacks it is recommended to not block the callback but to copy the data for later processing into user defined container.
Blocking callbacks can lead to dropped (missed) messages.
