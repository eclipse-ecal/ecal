.. include:: /include.txt

.. _configuration_local:

===================
Local configuration
===================

Using eCAL in local mode can be beneficial

* to avoid network congestion if the data is not needed on other machines
* to make eCAL work in VPN and Firewall scenarios where non-local traffic is blocked

To switch eCAL to local mode, edit your :file:`ecal.yaml` and change the following settings:

* |fa-windows| Windows: |ecalini-path-windows|
* |fa-ubuntu| Ubuntu: |ecalini-path-ubuntu|

.. code-block:: yaml

  # Registration layer configuration
  registration:
    [..]
    # true  = all eCAL components communicate over network boundaries
    # false = local host only communication (Default: false)
    network_enabled: false

  [..]

  # Transport layer configuration
  transport_layer:
    udp:
      [..]

      network:
        [..] 
        # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination
        ttl: 0

.. seealso::
   To learn about the differences between local mode and cloud mode, check out :ref:`this table <getting_started_cloud_local_mode_vs_cloud_mode>`.