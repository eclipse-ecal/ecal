.. include:: /include.txt

.. _getting_started_setup:

===============
Installing eCAL
===============

Before using eCAL, you must install it on your PC.
This section covers how to install eCAL on Microsoft Windows and Ubuntu (both 64 bit only).
You can either use eCAL just on one PC (local communication) or on multiple PCs (cloud communication).
We will cover both scenarios in the next sections.

So, let's get started!

.. seealso::
   If you are looking for a tutorial on how to build eCAL from source (e.g. to get it working on a non-supported OS) please refer to :ref:`development_building_ecal_from_source` .

.. _getting_started_setup_windows:

|fa-windows| Installing eCAL on Windows
=======================================

#. Choose your desired eCAL Version from the :ref:`eCAL Download Archive<download_archive>` and download the Windows installer.
   New users should download the latest Version.
   
#. Install eCAL with default options (Full installation):

   .. image:: img/setup.png
      :alt: eCAL Setup

.. _getting_started_setup_ubuntu:

|fa-ubuntu| Installing eCAL on Ubuntu
=====================================

For Ubuntu, you can either install eCAL from a PPA (Personal Package Archive) or manually from debian files.
For most users, the PPA will provide the easiest way of installing eCAL.
The PPA also supports more Ubuntu Versions and CPU Architectures (x86, x64, arm64, armhf).

**Ubuntu 22.04 Users**:
You may experience issues with the GUI Applications (actually all Qt5 Apps) due to Ubuntus switch to wayland.
But you can :ref:`log in with X11 <faq_ubuntu_22_04_graphical_issues>`, which solves those issues.

.. seealso::

   Advanced users can also :ref:`compile eCAL from source <development_building_ecal_from_source>`.

.. _getting_started_setup_ubuntu_ppa:

|fa-ubuntu| Automatically install eCAL from a PPA
-------------------------------------------------

Installing eCAL from a PPA will cause apt-get to automatically update eCAL for you.
You can choose from different PPAs, depending on your desired upgrade path.

.. include:: _ppa_tabs.rst.txt

|fa-ubuntu| Manually install eCAL from a .deb file
--------------------------------------------------

.. warning:: 

   We usually recommend installing from a PPA (:ref:`see above <getting_started_setup_ubuntu_ppa>`).

#. Choose your desired eCAL Version from the :ref:`eCAL Download Archive<download_archive>`.

#. Install the eCAL Dependencies:

   - Ubuntu 18.04:

     .. code-block:: bash

        sudo apt update
        sudo apt install libc6 libcurl4 libgcc1 libhdf5-100 libprotobuf10 libqt5core5a libqt5gui5 libqt5widgets5 libqt5svg5 libstdc++6 sysstat ifstat libqwt-qt5-6 libyaml-cpp0.5v5

   - Ubuntu 20.04:

     .. code-block:: bash

        sudo apt update
        sudo apt install libc6 libcurl4 libgcc-s1 libhdf5-103 libprotobuf17 libqt5core5a libqt5gui5 libqt5widgets5 libqt5svg5 libstdc++6 sysstat ifstat libqwt-qt5-6 libyaml-cpp0.6
        
   - Ubuntu 22.04:

     .. code-block:: bash

        sudo apt update
        sudo apt install libc6 libcurl4 libgcc-s1 libhdf5-103 libprotobuf23 libqt5core5a libqt5gui5 libqt5widgets5 libqt5svg5 libstdc++6 sysstat ifstat libqwt-qt5-6 libyaml-cpp0.7

#. Install the debian package:

   .. code-block:: bash

      sudo dpkg -i ecal_*.deb
