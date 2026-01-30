.. include:: /include.txt

.. _ecal_version_5_12:

=========
eCAL 5.12
=========

eCAL 5.12 was released in August 2023.

- **Release**: August 2023
- **End of life**: September 2024 (planned)

New features
============

- **True Zero Copy!**

  eCAL had Zero Copy in the past.
  But now it's even better!
  With the new **low level API** you can precisely control which part of your Shared Memory message you actually have to change and therefore speed up your system even more!

  Check out the :ref:`Zero Copy documentation<transport_layer_shm_zerocopy>` for explanations and examples on how to use it!

- **Container support through host-groups**

  You want to containerize your eCAL Apps?
  Well, then this is the release for you.
  In the past, you had to choose between enabling support for eCAL services or utilizing the shared memory transport layer, now you can have both simultaneously.

  Check out the :ref:`eCAL in Docker tutorial<ecal_in_docker>` to learn more!

  This also enables you to manage apps in your docker containers with eCAL Sys!

- **Improved eCAL Monitor for Debugging**

  We enriched the eCAL Monitor with even more information, that allow you to debug your system.
  You can now see the size and hash of your publishers' and subscribers' **descriptors**.
  And - if that isn't enough for you - the eCAL Monitor got an entirely new **Raw Data panel** for very deep (but easy!) inspections of the registration layer.

- **Improved support for C#**

  The C# language binding has seen some mayor updates. Most notably, the Client / Server API is now available in C#, so you can use eCAL's RPC features!


- Check out the `entire 5.12.0 changelog <https://github.com/eclipse-ecal/ecal/releases/tag/v5.12.0>`_!.

Compatibility table
===================

.. include:: compatibility_table.txt

Where to download
=================

- |fa-windows| Windows: Head to the :ref:`Download Archive <download_archive>` and pick your version from there!
- |fa-ubuntu| Ubuntu: Install eCAL from :ref:`our PPA <getting_started_setup_ubuntu_ppa>` with ``apt-get`` *(or pick a Version from the download archive as well)*

