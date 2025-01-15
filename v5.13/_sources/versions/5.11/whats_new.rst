.. include:: /include.txt

.. _ecal_version_5_11:

=========
eCAL 5.11
=========

eCAL 5.11 was released in Nobember 2022.
The release contains the eCAL Mon TUI, the eCAL Meas Cutter and the SHM Monitoring Layer.

- **Release**: December 2022
- **End of life**: March 2024 (after 13 Month)

New features
============

- New Apps:

  - **Mon TUI** (``ecal_mon_tui``) - An htop-like eCAL Monitor running in a terminal

    .. image:: img/mon_tui_combined.png
        :alt: eCAL Rec dark mode
        :width: 400px

  - **Measurement Cutter** (``ecal_meas_cutter``) for trimming and cutting eCAL Measurements (:ref:`-> Documentation<applications_meas_cutter>`)

- eCAL Rec can now **record each topic to its own file**

- Added **Shared-Memory-Monitoring-Layer** (experimental, enable via ``ecal.ini``!). This new monitoring layer can locally match publishers and subscribers and distribute monitoring information without the need for UDP Multicast.

- GUI Apps now start in **dark-mode** by default

- Setup for Windows now supports **non-admin installations**
  


- Check out the `entire 5.11.0 changelog <https://github.com/eclipse-ecal/ecal/releases/tag/v5.11.0>`_!.

Compatibility table
===================

.. include:: compatibility_table.txt

Where to download
=================

- |fa-windows| Windows: Head to the :ref:`Download Archive <download_archive>` and pick your version from there!
- |fa-ubuntu| Ubuntu: Install eCAL from :ref:`our PPA <getting_started_setup_ubuntu_ppa>` with ``apt-get`` *(or pick a Version from the download archive as well)*

