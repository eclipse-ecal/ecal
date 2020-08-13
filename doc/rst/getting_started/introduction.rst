.. include:: /include.txt

.. _getting_started_introduction:

Introduction
===========================

The **enhanced communication abstraction layer** (eCAL) is a middleware that enables scalable, high performance interprocess communication on a single computer node or between different nodes in a computer network.
eCAL is designed for typical cloud computing scenarios where different processes exchange there I/O's using a publish/subscribe pattern.

If you have ever worked with other publish subscribe middlewares like `MQTT <https://en.wikipedia.org/wiki/MQTT>`_ or `ROS <https://www.ros.org/>`_, you should be familiar with the terminology.
But just in case you are new to this topic, let us clarify the basic terminology:

* **eCAL Process / Node**:

  The term eCAL Process (or eCAL node) describes one small entity of your system that offers or consumes data. Typically, this is an executable (or a script) running on a PC.

* **eCAL Topic**:

  eCAL Processes do not sent data directly to the receiver.
  Instead, they offer the data as a *topic*, which is identified by its name.
  The process is now a **publisher** of that topic.
  Another process that is interested in the data can now **subscribe** to the topic and will automatically receive all messages from it.

  Publishers and subscribers do not have to know anything about each other; the dataflow is entirely managed by eCAL.

Let us look at the following example:

.. image:: img/ecal_concept_notebooks.svg
   :alt: eCAL concept

The image shows two PCs.
`Proc1` and `Proc2` are running on one PC, while `Proc3` runs on another PC that is connected via Ethernet.

`Proc1` **publishes** `Topic A`. That topic is **subscribed** by `Proc2` and `Proc3`, so both will receive all messages that are sent to that topic.
`Proc2` also publishes `Topic B`, which is subscribed by `Proc3`, while `Proc3` publishes `Topic C` which is subscribed by `Proc 2`.

What are the advantages of such a public-subscribe system?

* Components of your system can be implemented independent from each other; they only have to agree on the data format
* You can add and remove publishers and subscribers, use different versions or replace the publisher with a replay
* Publishers and subscribers can run on different machines that may even run different operating systems

.. note::
   eCAL works entirely decentral. It does not rely on a central broker, like the *MQTT Broker* or the *roscore* from ROS1

------------------------------------------------------------

Now, if you think you understood the basic concept, advance to the next section!
