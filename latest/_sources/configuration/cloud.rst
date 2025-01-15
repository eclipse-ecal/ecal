.. include:: /include.txt

.. _configuration_cloud:

===================
Cloud Configuration
===================

To switch eCAL to cloud mode, edit your :file:`ecal.yaml` and change the following settings:

* |fa-windows| Windows: |ecalini-path-windows|
* |fa-ubuntu| Ubuntu: |ecalini-path-ubuntu|

.. code-block:: yaml

  # Registration layer configuration
  registration:
    [..]
    # true  = all eCAL components communicate over network boundaries
    # false = local host only communication (Default: false)
    network_enabled: true

  [..]
  
  # Transport layer configuration
  transport_layer:
    udp:
      [..]

      network:
        [..] 
        # TTL (hop limit) is used to determine the amount of routers being traversed towards the destination
        ttl: 3

.. important::
   Don't forget to set your multicast routes and make sure your hostname resolution works on all machines!

   * :ref:`UDP Multicast routes <getting_started_cloud>`
   * :ref:`Hostname resolution for Services <getting_started_services_cloud>`