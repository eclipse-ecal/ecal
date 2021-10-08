.. include:: /include.txt

.. _ecal_configuration:

==================
eCAL Configuration
==================

---------------------------
Loading strategy (Priority)
---------------------------

The eCAL configuration file is loaded based on the following priorities for Windows and Linux. In addition, it can be passed explicitly to every eCAL application with the command line option ``--ecal-ini-file``.

Windows:
  1. ``$ECAL_DATA/ecal.ini``
  2. ``$AppData/ecal/ecal.ini``

Linux:
  1. ``$ECAL_DATA/ecal.ini``
  2. ``CMAKE_CONFIGURED_PATH/ecal.ini``    (e.g. /usr/local/etc/ecal/ecal.ini)
