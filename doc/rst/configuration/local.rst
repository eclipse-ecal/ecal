.. include:: /include.txt

.. _configuration_local:

===================
Local configuration
===================

Using eCAL in local mode can be benefitial

* to avoid network congestion if the data is not needed on other machines
* to make eCAL work in VPN and Firewall scenarios where non-local traffic is blocked

To switch eCAL to local mode, edit your :file:`ecal.yaml` and change the following settings:

* |fa-windows| Windows: |ecalini-path-windows|
* |fa-ubuntu| Ubuntu: |ecalini-path-ubuntu|

.. code-block:: ini

   [network]
   network_enabled           = false
   multicast_ttl             = 0

.. seealso::
   To learn about the differences between local mode and cloud mode, check out :ref:`this table <getting_started_cloud_local_mode_vs_cloud_mode>`.