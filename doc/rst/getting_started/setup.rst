.. include:: /include.txt

.. _getting_started_setup:

============================
Setup
============================

Before using eCAL, you must install it on your PC.
This section covers how to install eCAL on Microsoft Windows and Ubuntu (both 64 bit only).
You can either use eCAL just on one PC (local communication) or on multiple PCs (cloud communication).
We will cover both scenarios in the next sections.

So, let's get started!

.. seealso::
   If you are looking for a tutorial on how to build eCAL from source (e.g. to get it working on a non-supported OS) please refer to :ref:`development_building_ecal_from_source` .

|fa-windows| Installing eCAL on Windows
=======================================

For Microsoft Windows we provide binary installers published on our GitHub release page.

#. Head to the GitHub release page and download the latest eCAL installer (ecal_X.X.X-win64.exe):
   
   https://github.com/continental/ecal/releases

#. Install eCAL with default options (Full installation):

   .. image:: img/setup.png
      :alt: eCAL Setup

|fa-ubuntu| Installing eCAL on Ubuntu
=====================================

We provide precompiled binaries for Ubuntu 18.04 and Ubuntu 20.04.

#. Install the dependencies:

   - Ubuntu 18.04:

     .. code-block:: bash

        sudo apt update
        sudo apt install libc6 libcurl4 libgcc1 libhdf5-100 libprotobuf10 libqt5core5a libqt5gui5 libqt5widgets5 libstdc++6

   - Ubuntu 20.04:

     .. code-block:: bash

        sudo apt update
        sudo apt install libc6 libcurl4 libgcc-s1 libhdf5-103 libprotobuf17 libqt5core5a libqt5gui5 libqt5widgets5 libstdc++6


#. Head to the GitHub release page and download the latest eCAL installer for your Ubuntu version:
   
   https://github.com/continental/ecal/releases

   +----------------+----------+-------------------------------------+
   | Ubuntu Version | Codename | eCAL setup file                     |
   +================+==========+=====================================+
   | 18.04          | Bionic   | :file:`ecal_X.X.X-bionic_amd64.deb` |
   +----------------+----------+-------------------------------------+
   | 20.04          | Focal    | :file:`ecal_X.X.X-focal_amd64.deb`  |
   +----------------+----------+-------------------------------------+

#. Install the debian package:

   .. code-block:: bash

      sudo dpkg -i ecal_*.deb

.. seealso::

   If you want to use eCAL on any other Linux than Ubuntu 18.04 or 20.04, you can :ref:`build eCAL from source <development_building_ecal_from_source>`.
