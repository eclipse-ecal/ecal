.. include:: /include.txt

.. _ecal_compatibility:

==================
eCAL Compatibility
==================

This page aims to help you upgrading from one eCAL Version to another.

eCAL is versioned in the common **MAJOR.MINOR.PATCH** format.
Depending on what we changed in eCAL, we increment the appropriate number:

- **MAJOR**: This number is used exclusively for indicating the wire-compatibility.
  By wire-compatibility we mean the data protocol that is transferred over network (UDP Data transfer).
  This number is expected to stay at 5 for a long time, but if it ever increases, eCAL 5 nodes will no longer communicate with eCAL 6 nodes.

  *All eCAL 5 nodes can communicate with each other.*
  
  .. note::
     We don't make guarantees about the wire-compatibility of eCAL Shared Memory communication, even though we try to keep it compatible as long as possible.
     As Shared Memory is a local communication technique, this is only relevant if you would want to use multiple eCAL Versions on one machine.

- **MINOR**: The minor version is the most important for you to take care of.
  We increment the minor version, if:

  - The API or ABI has changed. You may have to re-compile your nodes e.g. when upgrading from eCAL 5.8 to eCAL 5.9
  - We have added (or removed) features, altered eCAL Tools or did anything that disqualifies the eCAL Release from being a drop-in replacement

  *Alle eCAL releases sharing the same minor version number can act as drop-in replacements.*

- **PATCH**: The last number indicates that we have fixed bugs.
  You can upgrade through patch versions without having to expect any problems.
  Usually, you just want the latest release to profit from all bugfixes.

eCAL 5.10 vs. 5.9
================

Compatibility table when upgrading from eCAl 5.8:

.. list-table:: eCAL 5.10 vs. 5.9
   :widths: 20 80

   * - Wire compatibility
     
     - * eCAL UDP: 100% compatible
  
       * eCAL TCP: New in 5.10.

         Prior versions of eCAL will not be able to receive data from TCP publishers.

       * Services: 100% compatible

       * eCAL Shared Memory: Downwards compatible
     
         eCAL 5.10 offers an option to use multi-buffering for increased performance.
         Prior Versions of eCAL cannot receive data from publishern using that feature.

         By default, the feature is turned off, which makes the Shared memory layer 100% compatible.

   * - API / ABI

     - * API is *mostly* downward compatible.

         When using the raw (non-protobuf) Service API, you may incorporate some incompatibilities.
         Please check :ref:`this guide <compatibility_details_5_10_service_api>` on how to upgrade.
         
       * ABI is not compatible

   * - Tools

     - * Rec: 100% compatible

       * Sys: 100% compatible

       * Measurements: 100% compatible


eCAL 5.9 vs. 5.8
================

Compatibility table when upgrading from eCAl 5.8:

.. list-table:: eCAL 5.9 vs. 5.8
   :widths: 20 80

   * - Wire compatibility
     - * eCAL UDP: 100% compatible
       * Services: 100% compatible
       * eCAL Shared Memory: 100% compatible

   * - API / ABI
     - * API is downward compatible (New functions have been added)
       * ABI is not compatible

   * - Tools
     - * Rec: Somewhat compatible. 
  
         Expect some issues when using the new (5.9) ecal_rec_cli with old (5.8) ecal_rec_gui or ecal_rec_client instances.
         It should be possible to mix ecal_rec_gui and ecal_rec_client instances of both versions, though.

       * Sys: 100% compatible

       * Measurements: 100% compatible

eCAL 5.8 vs. 5.7
================

Compatibility table when upgrading from eCAl 5.7:

.. list-table:: eCAL 5.8 vs. 5.7
   :widths: 20 80

   * - Wire compatibility
     - * eCAL UDP: 100% compatible
       * Services: 100% compatible
       * eCAL Shared Memory: 100% compatible

   * - API / ABI
     - * API is downward compatible
       * ABI is not compatible

   * - Tools
     - * Rec: Not compatible. Some features may work, but many won't.
  
       * Sys: New in 5.8

       * Measurements: 100% compatible

   * - Additional notes
     - * On Ubuntu, the official builds changes the lib dirs to the proper multiarch lib dirs.
       * On Ubuntu the location of :file:`ecal.ini` changed to ``/etc/ecal/ecal.ini``
       * On Windows, the location of :file:`ecal.ini` changed to ``C:\ProgramData\eCAL\ecal.ini``
  
