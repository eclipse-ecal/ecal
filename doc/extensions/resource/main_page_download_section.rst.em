Downloading and installing eCAL
===============================

At the moment, we support eCAL @(ecal_latest_version.major).@(ecal_latest_version.minor) and eCAL @(ecal_previous_version.major).@(ecal_previous_version.minor) with fixes.
New users should always use the latest eCAL Version (@(ecal_latest_version.major).@(ecal_latest_version.minor)).
Support for eCAL @(ecal_previous_version.major).@(ecal_previous_version.minor) will be dropped when eCAL @(ecal_latest_version.major).@(ecal_latest_version.minor + 1) is released.

- |fa-windows| Windows:
  
  #. Visit the :ref:`Download Archive <download_archive>` and download the latest eCAL Installer

  #. Install eCAL

     .. image:: getting_started/img/setup.png
        :alt: eCAL Setup

- |fa-ubuntu| Ubuntu:

  Install eCAL from our PPA:

  .. code-block:: bash

     sudo add-apt-repository ppa:ecal/ecal-latest
     sudo apt-get update
     sudo apt-get install ecal

  Check out other PPAs (that are fixed to a specific eCAL release) :ref:`here <getting_started_setup_ubuntu_ppa>`.