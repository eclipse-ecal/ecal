.. include:: /include.txt

.. _threading_model:

==================================
Threading model
==================================

eCAL is using different transport layers for intraprocess, interprocess or interhost communication. With a call of 'eCAL::Initialize' different internal, global threads are startet:

- udp registration send / receive thread (2 threads)
- udp logging receive thread (1 thread)
- udp monitoring send thread (1 thread)
- udp message receive thread (1 thread)

For the eCAL user interface entities 'CTimer', 'CSubscriber', 'CServiceServer', 'CServiceClient' additional threads are utilized:

- shm message receive thread (1 thread per handled memory file)
- timer callback (1 thread per instance)
- tcp client/server implementation (2 threads per instance)

For user API callback functions eCAL is protecting the forwarded data (message header, message payload ..) as long as the callback is processed. 
For subscriptions and their callbacks it is recommended to not block the callback but to copy the data for later processing into user defined container.
Blocking callbacks can lead to dropped (missed) messages.
