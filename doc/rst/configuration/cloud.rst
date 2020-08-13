.. include:: /include.txt

.. _configuration_cloud:

===================
Cloud Configuration
===================

To switch eCAL to cloud mode, edit your :file:`ecal.ini` and change the following settings:

* |fa-windows| Windows: |ecalini-path-windows|
* |fa-ubuntu| Ubuntu: |ecalini-path-ubuntu|

.. code-block:: ini

   [network]
   network_enabled           = true
   multicast_ttl             = 2

.. important::
   Don't forget to set your multicast routes and make sure your hostname resolution works on all machines!

   * :ref:`UDP Multicast routes <getting_started_cloud>`
   * :ref:`Hostname resolution for Services <getting_started_services_cloud>`