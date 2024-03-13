.. include:: /include.txt

.. _advanced_ros_comparison:

=======================
How eCAL relates to ROS
=======================

Every so often, the question arises how eCAL relates to ROS. 
Both systems aim to provide interprocess communication based on Publish/Subscribe and Service/Client patterns.
ROS1 had been available before the eCAL project was started; ROS2 was published before eCAL was released as an open source project.
This section will shed some light on the motivation behind eCAL as well as how eCAL differs from ROS.
It by no means provides a fully-fledged comparison of the two systems nor does it provide any benchmarks.
However, it can guide the interested reader to decide if indeed eCAL may be a better fit than ROS for their use case.

----------------------------
Motivation for creating eCAL
----------------------------

eCAL was developed by Continental out of the need of a communication middleware solution.
At that time, ROS1 was not considered to be a suitable choice for the following reasons

- **Lack of Windows support**

  ROS1 doesn't support the Windows operating system, which was a hard requirement at the time

- **Data throughput not high enough**

  ROS1 communication is based on TCP only, for both local and distributed communication.
  This limits the communication bandwidth for local communication compared to shared memory transport.

- **Lack of message downward/upward compatibility**

  eCAL was developed with rapid prototyping scenarios in mind.
  In these scenarios, datatypes usually evolve frequently.
  In ROS however changing message descriptions will usually render any legacy publisher / subscriber, as well as any existing rosbag useless.
  Especially not being able to use old recordings later was not acceptable.

eCAL was then build with these three features in mind.

----------------------------------
How eCAL differs from ROS(2) today
----------------------------------

eCAL and ROS have both evolved over the years. 
With ROS2 came Windows support, and the ROS RMW concept allows for more flexible transport concepts.

.. tip::
   
   As eCAL focuses on message transport, while ROS focuses on API and tools, it is possible to use eCAL as middleware layer in ROS via the `rmw_ecal <https://github.com/continental/rmw_ecal>`_!

However, there are two key factors which sets eCAL apart from ROS today:

- **eCAL is message protocol agnostic**

  eCAL transports binary data that the user provides via the API.
  Thus, there is no coupling to the definition and serialization of those messages.
  This allows eCAL to be used in conjunction with many serialization libraries such as Protobuf, Flatbuffers, Capnproto or MessagePack.
  The developers can choose one with their requirements in mind, like backwards compatibility, fast serialization, or easy indexing.
  
  ROS's IDL system on the other hand is directly tied into ROS and cannot be exchanged.
  Even though the rmw (and therefore the serialization) can now be interchanged, tools like rosbag still cannot record the binary data and therefore not provide backwards compatibility.
  
- **eCAL is a library (+ tools) while ROS is an ecosystem**

  eCAL was build with modularity and ease of integration in mind.
  When building communication components, they only need to link to the eCAL core library.
  eCAL tools may be installed in addition.

  ROS on the other hand comes with a complete ecosystem, bundling C++ libraries, providing its own build system.
  While this can be seen as an advantage for inexperienced users, it might make ROS harder to integrate into already existing ecosystems.
  
There are also some additional points which set both frameworks apart:  

- **eCAL can create distributed recordings**

  In distributed scenarios with high data throughput it is essential to create distributed recordings where data is recorded on the host where it is created.
  eCAL Recorder can record in a distributed manner, and later merge recordings.
  These recordings can then be played back synchronized.

- **eCAL communication is brokerless**

  eCAL does not need a broker for tasks to communicate with each other.
  All necessary information is exchanged via UDP on a dedicated "monitoring layer".
  
-------------------
To use ROS or eCAL?
-------------------

Which project is the best fit for a given user highly depends on the use case.
Usually, it is a good advice to continue using a framework you already know, until you hit its limits.

We recommend eCAL for scenarios where high bandwidth matters, as it usually performs much better than most stock RMWs.
