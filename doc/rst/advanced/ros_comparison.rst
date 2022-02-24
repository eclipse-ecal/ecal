.. include:: /include.txt

.. _advanced_ros_comparison:

=======================================
How eCAL relates to ROS
=======================================

Every so often, the question arises how eCAL relates to ROS. 
Both systems aim to provide interprocess communication based on Publish/Subscribe and Service/Client patterns.
ROS1 had been available before the eCAL project was started, ROS2 was published before eCAL was released as an open source project.
This section will shed some light on the motivation behind eCAL as well as how eCAL differs from ROS.
It by no means provides a full fledged comparison of the two systems nor does it provide any benchmarks.
However, it can guide the interested reader to decide if indeed eCAL may be a better fit than ROS for their usecase.

---------------------------------------
Motivation for creating eCAL
---------------------------------------
eCAL was developed by Continental out of the need of a communication middleware solution.
At that time, ROS1 was not considered a suitable choice for the following reasons

- **Lack of Windows support**

  ROS1 doesn't support the Windows operating system, which was a hard requirement at the time

- **Data throughput not high enough**

  ROS1 communcation is based on UDP only, for both local and distributed communication.
  This highly limits the communication bandwidth for local communication scenarios compared to shared memory transport.

- **ROS message definitions cannot be evolved without breaking measurements**

  eCAL was developed with rapid prototyping scenarios in mind. In these scenarios, datatypes could evolve frequently, but it had to be ensured that measurements are still readable as datatypes evolve. ROS1 could not provide this, effectively rendering old measurements useless.
  
eCAL was then build with these three features in mind.

----------------------------------------
How eCAL differs from ROS(2) today
----------------------------------------
eCAL and ROS have both evolved over the years. 
With ROS2 came Windows support, and the ROS RMW concept allows for more flexible transports (there is also an eCAL implementation of a ROS RMW layer).
However there are two key factors which sets eCAL apart from ROS today:

- **eCAL is message protocol agnostic**

  ROS's IDL system is tied into ROS. And it's still not backwards compatible regarding message evolution.
  eCAL on the other hand transports binary data. Thus, there is no coupling between the message serialization (and thus also the message definitions), and the actual transport of this data.
  This allows eCAL to be used in conjunction with many serialization libraries such as Protobuf, Flatbuffers, Capnproto or MessagePack.
  Is it necessary that the schema is backwards compatible? Does the user need fast serialization or easy indexing? 
  Depending on the usecase, the user can choose whatever format fits them best, on a per message basis.
  
- **eCAL is a library (+ tools) while ROS is an ecosystem**

  eCAL was build with modularity and ease of integration in mind. When building communication components, they only need to link to the eCAL core library.
  eCAL tools may be installed in addition.
  ROS on the other hand comes with a complete ecosystem, bundling C++ libraries, providing its own build system.
  While this can be seen as an advantage for inexperienced users, it might make ROS harder to integrate into already existing ecosystems.
  
There are also some additional points which set both frameworks apart:  

- **eCAL can create distributed recordings**

  In distributed scenarios with high data throughput it is essential to create distributed recordings where data is recorded on the host where it is created.
  eCAL Recorder can record in a distributed manner, and later merge recordings.
  These recordings can then be played back synchronized.

- **eCAL communication is brokerless**

  eCAL does not need a broker in order for tasks to communicate with each other. All necessary information is exchanged via UDP on a dedicated "monitoring layer".
  
------------------------------------------
To use ROS or eCAL?
------------------------------------------  
ROS is a great project. However, eCAL is too. 
Which project is the best fit for a given user highly depends on the usecase.
We recommend to use eCAL in trusted networks, in scenarios where high bandwidth matters.







