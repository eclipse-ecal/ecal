.. include:: /include.txt

.. _configuration_options:

=========
ecal.yaml
=========

eCAL has many options that can be configured via an .yaml file which is located at:

* |fa-windows| Windows: |ecalconfig-path-windows|
* |fa-ubuntu| Ubuntu: |ecalconfig-path-ubuntu|

Loading strategy (Priority)
===========================

The eCAL configuration file is loaded based on the following priorities, whereever it is found first.
If you want a specific eCAL Node to run with another ``ecal.yaml`` than the others, you can set the ``ECAL_DATA`` variable before starting the process, e.g. from a batch or shell skript.
Also, if you place a ``ecal.yaml`` in the working directory of the process, it will be loaded if ``ECAL_DATA`` is not specified first.

In order to create the eCAL configuration files in the marked (*) directories, you can use the ``ecal_generate_config`` tool, which is part of the eCAL installation.
It will generate the files in the first of the marked directories that is writable.

.. important::
   This loading strategy is valid for eCAL 6.0 and up.

- |fa-windows| Windows:

  1. ``%ECAL_DATA%\ecal.yaml``
  2. ``%cd%\ecal.yaml``
  3. |ecalconfig-path-windows| *
  4. ``%ProgramData%\eCAL\ecal.yaml`` * (fallback)
  5. ``<ecal installation directory>\etc\ecal.yaml`` (fallback)

- |fa-ubuntu| Ubuntu:

  1. ``$ECAL_DATA/ecal.yaml``
  2. ``$PWD/ecal.yaml``
  3. ``$HOME/.ecal/ecal.yaml`` *
  4. |ecalconfig-path-ubuntu| (fallback)
      
      
.. note::

  When compiling eCAL from source:
  
  In previous versions (before eCAL 6.0.0), you could set CMAKE_INSTALL_SYSCONFDIR to change the default configuration path and the applications could still find it on Linux.
  
  This is not the case anymore. 
  
  Instead, when installing your build to a custom path, a directory next to ``lib`` named ``etc`` will be created containing the ecal.yaml file if no CMAKE_INSTALL_SYSCONFDIR is set.
  That file in etc relativ to the ecal_core library will be used if no other is found.
  

ecal.yaml options
=================

Here you can find the current ecal.yaml options with description and default values. You can download the file here or generate it with the ecal_generate_config tool.

.. parsed-literal::

* |fa-file-alt| :download:`ecal.yaml </configuration_files/ecal.yaml>`

  .. literalinclude:: /configuration_files/ecal.yaml
     :language: yaml
     :linenos: