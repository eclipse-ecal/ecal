.. include:: /include.txt

.. _getting_started_hello_world_python:

====================
Hello World (Python)
====================

While eCAL is written in C++, it also offers a great python integration.
Again, we are going to explore that integration by implementing a Hello World Application, that sends the string "Hello World" to an eCAL topic.

.. important::

   The eCAL Python integration is called ``python3-ecal5``.
   With the ``5`` at the end.
   The other name has already been taken by another package.
   

Dependencies
============

First, you have to install some more development dependencies:

* |fa-windows| Windows:

  * Python 3 64-bit (https://www.python.org/downloads/)

  * The eCAL python integration.
    Choose the appropiate version from the :ref:`download_archive` and download the :file:`.whl` file.
    Open a command prompt in your donwload directory and install the python whl with:

    .. code-block:: batch

       pip install ecal5-****-win_amd64.whl

* |fa-ubuntu| Ubuntu:

  * Python 3: 

    .. code-block:: bash

       sudo apt install python3 python3-pip

  * The eCAL Python integration.

    * If you have installed eCAL from a :ref:`PPA<getting_started_setup_ubuntu_ppa>`:
      
      .. code-block:: bash

         sudo apt install python3-ecal5

    * If you are not using a PPA, choose the appropiate version from the :ref:`download_archive` and download the :file:`.whl` file.
      Open a terminal in your donwload directory and install the python whl with:

  
      .. code-block:: bash

         sudo pip3 install ecal5-*-linux_x86_64.whl


Hello World Publisher
=====================

Somewhere on your hard drive create an empty directory and create a file :file:`hello_world.py` with the following content:

* |fa-file-alt| :file:`hello_world.py`:

  .. literalinclude:: src/hello_world_python/hello_world_snd.py
     :language: python
     :linenos:
         

  .. note::
     **What is happening here?**
  
     **Line 4-5** imports the eCAL Core and the eCAL String publisher (In this tutorial we only want to send raw strings).
     eCAL supports multiple message formats.
  
     **Line 9** Initialized the eCAL API.
     The name of our eCAL Process will be "Python Hello World Publisher". 
     This name will be visible in the eCAL Monitor, once the Process is running.
  
     **Line 13** creates an eCAL Publisher.
     An eCAL Process can create multiple publishers (and multiple subscribers).
     The topic we are publishing on will be "hello_world_python_topic".
  
     **line 19** Creates an infinite publish-loop.
     eCAL supports a stop-signal; when an eCAL Process is stopped, ``ecal_core.ok()`` will return false.
     
     **Line 23** will publish our message and send it to other eCAL Processes that have subscribed on the topic.
  
     **Line 31** de-initializes eCAL. You should always do that before your application exits.

Now execute your python hello world script and take a look at the eCAL Monitor!
You will see the "Python Hello World Publisher" process and the "hello_world_python_topic".

- |fa-windows| Windows: ``python hello_world_snd.py``
- |fa-ubuntu| Ubuntu: ``python3 hello_world_snd.py``


.. image:: img/python_hello_world_snd_monitor.png
   :alt: eCAL Monitor Hello World

Hello World Subscriber
======================

For the subscriber, create another file :file:`hello_world_rec.py` with the following content:

* |fa-file-alt| :file:`hello_world_rec.py`:

  .. literalinclude:: src/hello_world_python/hello_world_rec.py
     :language: python
     :linenos:
  
  .. note::
     **What is happening here?**

     **Line 5** Imports the eCAL StringSubscriber class

     **Line 8-9** Is the receive callback.
     This method will be called whenever a new message arrives.
     
     **Line 16** creates an eCAL subscriber that listens to the "hello_world_python_topic".

     **Line 19** adds the receive callback from above to the subscriber, so it can be called.

  .. important::
     eCAL Receive callbacks run in the subscriber's receive thread.
     While the callback is running, the subscriber cannot receive new data.
     So, if your callback needs really long to compute, you may have to decouple your computations to not loose messages.

When you now execute ``hello_world_snd.py`` and ``hello_world_rec.py``, the receiver application will receive the messages sent by the sender.

.. image:: img/python_hello_world_snd_rec.png
   :alt: Python Hello World Sender and Receiver

Awesome, you have written your first eCAL publisher and subscriber with python!
In general however you will want to have a better way to structure your data than a raw string.
So let's head on to the next topic where we will check out protobuf!

Files
=====

.. parsed-literal::

   |fa-folder-open|
   ├─ |fa-file-alt| :download:`hello_world_snd.py <src/hello_world_python/hello_world_snd.py>`
   └─ |fa-file-alt| :download:`hello_world_rec.py <src/hello_world_python/hello_world_rec.py>`
