.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_recorder_advanced_overview:

==============================
|ecalrec_APP_ICON_h1| Overview
==============================

In this chapter we will take a deeper look at the eCAL Recorder.
We will discuss the Host / Client architecture and look at all available configuration options and functions in the GUI.

.. seealso::

   If you are new to eCAL Rec, please first check out the :ref:`Getting Started with eCAL Rec <getting_started_recorder>` chapter.
   That chapter will teach you the very basics of eCAL Rec.

   If you don't want to use the GUI, please see the :ref:`applications_recorder_cli` chapter.

.. _applications_recorder_architecture:

Architecture
============

eCAL Rec consists of 2 applications:

#. The **Host Application**.
   This can be either one of:

   - eCAL Rec GUI (:file:`ecal_rec_gui /.exe`)
   - eCAL Rec CLI (:file:`ecal_rec /.exe`)

   The Host Application is your main Recording application.

   In a `Centraliced` recording (All topics are recorded over network by just one recorder), this is the only applicaiton you need.
   It will function standalone and record everything that is configured to be recorded.
   In a `Distributed` recording (Each machine records its own topics), this application controls the connected clients.

   The host application can load :file:`.ecalrec` configuration files.

#. The **Client Application**
   
   :file:`ecal_rec_client /.exe`

   The Client Application is only needed for distributed recordings.
   It is started on all machines, that shall record topics.
   The host application then connects to it and sends the appropriate commands.

   The eCAL Sys Client is not needed on the machine, where the Host Application is running.

   .. note::

      Even though you can use :file:`ecal_rec_client` from command line to start a recording, it is generally advisable to use :file:`ecal_rec /.exe` (the eCAL rec cli host application) for that purpose.
      It features a richer set of parameters, can load config files and has an interactive mode.