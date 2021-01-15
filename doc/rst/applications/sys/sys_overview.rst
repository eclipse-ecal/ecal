.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_sys_overview:

=================
eCAL Sys Overview
=================

In this chapter we will take a deeper look at eCAl Sys.
We will discuss the Host / Client architecture and look at all available configuration options and functions in the GUI.

.. seealso::

   If you are new to eCAL Sys, please first check out the :ref:`Getting Started with eCAL Sys <getting_started_sys>` chapter.
   That chapter will teach you the very basics of eCAL Sys.

   If you don't want to use the GUI, please see the :ref:`applications_sys_cli` chapter.

Architecture
============

eCAL Sys consists of 2 (or 3) applications:

#. The **Host Application**.
   This can be either one of:

   - eCAL Sys GUI (:file:`ecal_sys_gui /.exe`)
   - eCAL Sys CLI (:file:`ecal_sys /.exe`)

   The Host Application is started only once on the machine that is supposed to control everything else.

#. The **Client Application** (:file:`ecal_sys_client /.exe`). 

   The Client Application is started on all machines that shall be remote-controlled.
   It creates an eCAL Service and gets remote-controlled by the eCAL Sys Host Application.

   The eCAL Sys Client is not needed on the machine, where the Host Application is running.

   .. note::

      The eCAL Sys Client is a remote control application with not authentication.
      If you are annoyed by accepting this security implication at startup, you can start eCAL Sys Client with:

      .. code-block:: console

         ecal_sys_client --auto-accept-security-risk

#. The **Machine Monitoring Application** (:file:`ecal_mma /.exe`). 

   This application is not required for eCAL Sys itself.
   However, eCAL Sys will display information about system resources of all systems that run an ecal_mma:

   .. image:: img/ecal_sys_mma.png
      :alt: eCAL Sys MMA
      :align: center

   .. tip::

      You can use eCAL Sys to start the MMA Application, just by creating an ``ecal_mma`` task.

Configuration
=============

GUI Functions
=============

|ecalicons_STOP|












This section is still under construction. But we are working on it! Just be patient.

.. image:: /img/snail.svg
   :alt: Snail
   :align: center

