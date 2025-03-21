.. include:: /include.txt

.. _configuration_network:

=====================
Network Configuration
=====================

To switch eCAL to network mode, edit your :file:`ecal.yaml` and change the following setting to ``network``:

* |fa-windows| Windows: |ecalconfig-path-windows|
* |fa-ubuntu| Ubuntu: |ecalconfig-path-ubuntu|

* |fa-file-alt| :download:`ecal.yaml </configuration_files/ecal.yaml>`

.. literalinclude:: /configuration_files/ecal.yaml
   :language: yaml
   :linenos:
   :lines: 15-19

.. important::
   Don't forget to set your multicast routes and make sure your hostname resolution works on all machines!

   * :ref:`UDP Multicast routes <getting_started_network>`
   * :ref:`Hostname resolution for Services <getting_started_services_network>`