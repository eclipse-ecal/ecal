.. include:: /include.txt

.. _getting_started_cloud:

===================
Cloud configuration
===================

.. hint::
   If you don't have two PCs at your hand or if you are just not interested in inter-machine-communication, just skip this topic.
   It is not essential for the next tutorials to use cloud communication.

   Of course, you can use a virtual machine (e.g. with VMWare or VirtualBox) as second PC.
   Just add a host-only adapter to your VM.

Quite often you want data to flow between eCAL nodes on different machines.
eCAL can run in two modes, that differ from each other: **local mode** and **cloud mode**.

.. _getting_started_cloud_local_mode_vs_cloud_mode:

.. list-table:: Local mode vs. cloud mode
   :widths: 50 50
   :header-rows: 1

   * - Local mode
     - Cloud mode
   * - * Uses localhost (127.0.0.1) for registration (-> e.g. telling others about new topics and subscribing to topics)
     - * Uses Multicast (239.0.0.1) for registration
   * - * Uses shared memory to send actual data to other processes
     - * Uses UDP multicast (239.0.0.x) to send data to other hosts
       * Uses shared memory to send data to processes on the same host

By default, eCAL already is configured in cloud mode, so you don't have to change anything.
You however have to configure your operating system, so it knows where to send that multicast traffic to.
This is done by creating a **multicast route**.

.. seealso::
   Please refer to the advanced section to learn about changing between :ref:`local mode <configuration_local>` and :ref:`cloud mode <configuration_cloud>`!

.. _getting_started_cloud_configuration_on_windows:

|fa-windows| Multicast configuration on Windows
===============================================

#. Check the IPv4 address of the ethernet adapter you are using to connect your two PCs.
   You can do that by typing ``ipconfig`` in a command prompt.
#. Open a command prompt with **administrator** privileges
#. Enter the following line and replace xx.xx.xx.xx with your IP address
   
   .. code-block:: bat

      route -p add 239.0.0.0 mask 255.255.255.0 xx.xx.xx.xx

   .. tip::
      
      If you made a mistake, you can delete your route/s with ``route delete 239.0.0.0``.
      Your eCAL communication may not work, if you leave faulty routes in place.

#. Reboot
#. Check the result from a command prompt.
   It should show your route under `IPv4 Route Table / Persistent Routes`.
   
   .. code-block:: bat

      route print

   .. image:: img/win_route_print.png
      :alt: Windows route print

.. tip::
   It is recommended to assign a static IP, so your multicast route will not become outdated at some point.


|fa-ubuntu| Multicast configuration on Ubuntu
=============================================

#. Configure the loopback route (this will become active, if you disconnect from all networks)
   
   .. code-block:: bash

      sudo gedit /etc/network/interfaces

   Add the following lines beneath the :code:`iface lo inet loopback` line:

   .. code-block:: bash

      post-up ifconfig lo multicast
      post-up route add -net 239.0.0.0 netmask 255.255.255.0 dev lo metric 1000
   
   .. note::
      The high metric will cause this loopback route to have lower priority than the route to the external interface that we will create in the next step.

#. Configure a route for the external interface:

   - **Recommended way**: If you have a graphical network manager installed (-> Desktop Ubuntu), you should use it to configure the route.

     - System Settings -> Network -> Your Adapter -> Options -> IPv4 Tab -> Routes

     - Create a route:

       - Address: :code:`239.0.0.0`
       - Netmask: :code:`255.255.255.0`
       - Gateway: :code:`0.0.0.0`
       - Metric: :code:`1`

       .. image:: img/ubuntu_route_gui.png
          :alt: Ubuntu multicast route

   - **Not recommended** (but works fine): If you do not have a graphical network manager (-> Server Ubuntu), again add the post-up lines to :code:`/etc/network/interfaces` (just as for loopback, but with metric 1):

     .. code-block:: bash

        # replace eth0 with your network adapter
        post-up ifconfig eth0 multicast
        post-up route add -net 239.0.0.0 netmask 255.255.255.0 dev eth0 metric 1

#. Restart your PC

#. Check the result from a terminal. It should show routes for local and external communication:

   .. code-block:: bash
      
      route -n

   .. image:: img/ubuntu_route_print.png
      :alt: Ubuntu route -n

eCAL Samples over network
=========================

Now start one :file:`ecal_sample_person_snd` and one :file:`ecal_sample_person_rec`, just as in the :ref:`previous section<getting_started_samples>`.
But on different machines!
If you configured everything correctly, publisher and subscriber should connect almost immediatelly and exchange data.

.. image:: img/cloud_person_combined.png
   :alt: person_snd and person_rec sample over network
