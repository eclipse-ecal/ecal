.. include:: /include.txt

.. _getting_started_setup:

============================
Setup
============================

Before using eCAL, you must install it on your PC.
This section covers how to install eCAL on Microsoft Windows and Ubuntu 18.04 (We only support 64 bit!).
You can either use eCAL just on one PC (local communication) or on multiple PCs (cloud communication).
We will cover both scenarios in the next sections.

So, let's get started!

.. seealso::
   If you are looking for a tutorial on how to build eCAL from source (e.g. to get it working on a non-supported OS) please refer to :ref:`development_building_ecal_from_source` .

|fa-windows| Installing eCAL on Windows
=======================================

For Microsoft Windows we provide binary installers published on our GitHub release page.

#. Head to the GitHub release page and download the latest eCAL installer (ecal-X.X.X-win64.exe):
   
   https://github.com/continental/ecal/releases

#. Install eCAL with default options (Full installation):

   .. image:: img/setup.png
      :alt: eCAL Setup

|fa-ubuntu| Installing eCAL on Ubuntu 18.04
===========================================

For Ubuntu 18.04 (on AMD64) we provide precompiles binaries. 

#. Install the dependencies:

   .. code-block:: bash

      sudo apt update
      sudo apt install libc6 libcurl4 libgcc1 libhdf5-100 libprotobuf10 libqt5core5a libqt5gui5 libqt5widgets5 libstdc++6

#. Head to the GitHub release page and download the latest eCAL installer (ecal-X.X.X-linux.deb):
   
   https://github.com/continental/ecal/releases

#. Install the debian package:

   .. code-block:: bash

      sudo dpkg -i ecal-*-linux.deb