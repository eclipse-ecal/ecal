.. include:: /include.txt

.. _configuration_local:

===================
Local configuration
===================

Using eCAL in local mode can be beneficial

* to avoid network congestion if the data is not needed on other machines
* to make eCAL work in VPN and Firewall scenarios where non-local traffic is blocked

To switch eCAL to local mode, edit your :file:`ecal.yaml` and change the following setting to ``local``:

* |fa-windows| Windows: |ecalconfig-path-windows|
* |fa-ubuntu| Ubuntu: |ecalconfig-path-ubuntu|

.. literalinclude:: /configuration_files/ecal.yaml
   :language: yaml
   :linenos:
   :lines: 15-19

.. seealso::
   To learn about the differences between local mode and network mode, check out :ref:`this table <getting_started_network_local_mode_vs_network_mode>`.
