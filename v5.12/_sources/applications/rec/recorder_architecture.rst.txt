.. include:: /include.txt
.. include:: /_include_ecalicons.txt

.. _applications_recorder_architecture:

===========================================
|ecalrec_APP_ICON_h1| eCAL Rec Architecture
===========================================

In this chapter we will take a deeper look at the different recording modes and the host / client architecture of eCAL Rec.

.. seealso::

   If you are new to eCAL Rec, please first check out the :ref:`Getting Started with eCAL Rec <getting_started_recorder>` chapter.
   That chapter will teach you the very basics of eCAL Rec.

.. _applications_recorder_centralized_distributed_recording:

Centralized vs. Distributed Recordings
======================================

The eCAL Recorder has two essential modes for creating recordings:

#. **Centralized recording**:
   
   This means that you simply start the eCAL Recorder on one of your many machines.
   It will subscribe to all topics and record the data to your hard drive.
   Data from remote machines will be sent via network.

   This is the most trivial and easy to use mode, as you only need one application on one machine.

#. **Distributed recording**:

   When having many eCAL applications, it is generally advisable to run applications that exchange huge amounts of data on the same machine, as eCAL will then use its shared memory transport mechanism, which is way faster than sending data over the network.
   The recorder can also take advantage of that feature and avoid network congestion while running a recording.
   For this mode you will have to launch the eCAL Recorder on one of your machines and the eCAL Recorder Client on all of your other machines.
   
   Now each Recorder and Recorder Client will only record topics coming from their respectable machine and record them to their local hard drive.
   After you finished your test, you can then let the Recorder Clients push their files to your main PC to get a contiguous recording.

   Of course, mixed configurations are possible (e.g. some machines will only record their own topics, while other machines will also record topics of some external machines).

   .. seealso::

      .. youtube:: h3DUirOMfu8
         :align: center
         :height: 300px


.. _applications_recorder_host_client_architecture:

Host / Client Architecture
==========================

A (distributed) eCAL Rec setup consists of 2 applications:

#. The **Host Application**.
   This can be either one of:

   - eCAL Rec GUI (:file:`ecal_rec_gui /.exe`)
   - eCAL Rec CLI (:file:`ecal_rec /.exe`)

   The Host Application is your main Recording application.

   In a `Centralized` recording (All topics are recorded over network by just one recorder), this is the only application you need.
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
