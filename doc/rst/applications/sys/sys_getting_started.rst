.. include:: /include.txt

.. _applications_sys_getting started:

===============
Getting Started
===============

.. note::
   eCAL Sys is available since eCAL 5.8. 

eCAL Sys is an application for starting, stopping and monitoring applications on one or multiple machines.
You can use it to automatize the startup of your system and check whether any of your applications report an error.

Just like the eCAL Recorder, eCAL Sys consists of a client application and a control application with a GUI.
So for starting applications on multiple PCs, you will have to:

#. Start :file:`ecal_sys_gui (.exe)` on one machine
#. Start :file:`ecal_sys_client (.exe)` on any other machine than the one you started the GUI on

GUI Overview
============

Let's start eCAL Sys GUI and take a look at it!


.. important::
   The eCAL Sys Client does not perform any kind auf authentication.
   Any request that is received will be executed, so only use it in networks where you trust all participants.

   If you want to add an extra layer of security, there are existing standards like SSH that should fit your needs.

This section is still under construction. But we are working on it! Just be patient.

.. image:: /img/snail.svg
   :alt: Snail
   :align: center

