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

  *All eCAL releases sharing the same minor version number can act as drop-in replacements.*

- **PATCH**: The last number indicates that we have fixed bugs.
  You can upgrade through patch versions without having to expect any problems.
  Usually, you just want the latest release to profit from all bugfixes.

eCAL 5.11 vs. 5.10
==================

Compatibility table when upgrading from eCAL 5.10:

.. include:: 5.11/compatibility_table.txt

eCAL 5.10 vs. 5.9
=================

Compatibility table when upgrading from eCAL 5.9:

.. include:: 5.10/compatibility_table.txt

eCAL 5.9 vs. 5.8
================

Compatibility table when upgrading from eCAL 5.8:

.. include:: 5.9/compatibility_table.txt

eCAL 5.8 vs. 5.7
================

Compatibility table when upgrading from eCAL 5.7:

.. include:: 5.8/compatibility_table.txt
