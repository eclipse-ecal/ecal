.. include:: /include.txt

==============
eCAL TCP Layer
==============

The eCAL TCP Transport Layer has been added with eCAL 5.10.

How does it work
================

How to use
==========

The eCAL TCP Layer is a **publisher** setting.
When using network communication, the data is sent **either** via UDP **or** via TCP.
That means, that (by default), any subscriber can receive data from UDP and TCP subscribers (even simultaneously if they send on the same topic).

You can activate TCP in the following ways:

#. Modify the :file:`ecal.ini` and change the following:

   .. code-block:: ini

      [publisher]
      use_tcp                   = 2
      use_udp_mc                = 0
   
   This will

   #. Turn on TCP automatically (in the means of automatic switching between Shared Memory and TCP)
   #. Turn off UDP Multicast

   .. tip::
      
      This 

When should you use it?
=======================