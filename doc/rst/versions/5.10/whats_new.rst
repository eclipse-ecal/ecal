.. include:: /include.txt

.. _ecal_version_5_10:

=========
eCAL 5.10
=========

eCAL 5.10 was released in May 2022.
The release notably brings eCAL Core improvements (TCP and Shared Memory)

- **Release**: May 2022
- 
- **End of life**: July 2023 (after 14 months)

New features
============

- Added **Dark Mode** in all apps *(Activate it by View/Theme/Dark!)*
  
  .. image:: img/rec_combined.png
     :alt: eCAL Rec dark mode
     :width: 400px

- eCAL Mon can now display and **dock plugins** in it's main window, not only in the reflection Dialog

- Added eCAL Mon plugin for **plotting numeric data** in a 2D graph

  .. image:: img/monitor_signal_plotting.png
     :alt: eCAL Mon Signal Plotting
     :width: 400px

- Added full **UTF-8 Support** for all Apps on Windows

- Added **Shared Memory improvements** (Multi-Buffering & Zero-Copy)

  *(turned off by default to stay compatible! Turn it on to benefit from the improvements!)*

  .. image:: img/zerocopy_improvements.png
     :alt: eCAL Zero Copy
     :width: 400px

- Added **TCP Transport layer** (Improves Network subscriptions of large messages)


Other notable changes
=====================

- The :file:`ecal.ini` now installs a local configuration by default

- Improved eCAL Service API and internals

- Check out the `entire 5.10.0 changelog <https://github.com/eclipse-ecal/ecal/releases/tag/v5.10.0>`_!.

Compatibility table
===================

.. include:: compatibility_table.txt

Where to download
=================

- |fa-windows| Windows: Head to the :ref:`Download Archive <download_archive>` and pick your version from there!
- |fa-ubuntu| Ubuntu: Install eCAL from :ref:`our PPA <getting_started_setup_ubuntu_ppa>` with ``apt-get`` *(or pick a Version from the download archive as well)*

